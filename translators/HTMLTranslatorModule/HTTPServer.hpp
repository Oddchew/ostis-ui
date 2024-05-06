#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "sc-agents-common/utils/AgentUtils.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-memory/sc_debug.hpp"
#include "sc-memory/sc_memory.hpp"
#include "sc-memory/sc_type.hpp"
#include <httplib.h>
#include <string>

namespace htmlTranslationModule {
template <typename T>
ScAddr ConvertToScAddr(const T str) {
    ScAddr::HashType value;
    std::istringstream iss(str);
    iss >> value;
    return ScAddr(value);
}

// a function to return the sc-file content and its format
std::pair<std::string, std::string> GetFileContent(ScMemoryContext *ctx, ScAddr scFile);
void FileRequestHandler(ScMemoryContext *ctx, ScAddr fileAddr, const httplib::Request& req, httplib::Response& res) noexcept;

inline void StartServer() {
  // Instantiate the server
  httplib::Server server;
  // TODO: use newer ScMemoryContext constructor
  ScMemoryContext ctx(sc_access_lvl_make_min, "ostis_ui_web_server");
  // Define a handler for the root path
  server.Get("/", [&](const httplib::Request &req, httplib::Response &res) {
    // TODO: use user-specific current model
    ScAddr currentModelPointer =
        ctx.HelperResolveSystemIdtf("ostis_ui_current_ui_model");
    ScAddr currentModel =
        utils::IteratorUtils::getAnyFromSet(&ctx, currentModelPointer);
    ScAddr translationResult =
        utils::AgentUtils::applyActionAndGetResultIfExists(
            &ctx, HTMLTranslatorKeynodes::action_translate_sc_node_to_html,
            {currentModel}, 1000);
    ScAddr translationResultLink =
        utils::IteratorUtils::getAnyFromSet(&ctx, translationResult);
    if (!ctx.IsElement(translationResultLink)) {
      res.set_content("Error: translation result is invalid.", "text/html");
    } else {
      std::string result;
      ctx.GetLinkContent(translationResultLink, result);
      res.set_content(result, "text/html");
    }
  });

    // an endpoint to resolve sc-files using ScAddr
    server.Get("/files/by_sc_addr/:addr", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string addr = req.path_params.at("addr");
      ScAddr::HashType value;
      ScAddr fileAddr = ConvertToScAddr(addr);
      FileRequestHandler(&ctx, fileAddr, req, res);
    });
    // an endpoint to resolve sc-files using system identifier
    server.Get("/files/by_system_idtf/:system_idtf", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string system_idtf = req.path_params.at("system_idtf");
      ScAddr fileAddr;

      try {
        fileAddr = ctx.HelperResolveSystemIdtf(system_idtf);
      }
      catch (utils::ExceptionInvalidParams& e) {
        res.status = 404;
      }

      FileRequestHandler(&ctx, fileAddr, req, res);
    });
    // Start the server
    std::cout << "Starting server on port 8080..." << std::endl;
    server.listen("localhost", 8080);
}
}
