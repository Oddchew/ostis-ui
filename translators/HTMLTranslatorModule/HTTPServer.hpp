#include "sc-agents-common/utils/AgentUtils.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-memory/sc_memory.hpp"
#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include <httplib.h>

namespace htmlTranslationModule {

void StartServer() {
    // Instantiate the server
    httplib::Server server;
    ScMemoryContext ctx(sc_access_lvl_make_min, "ostis_ui_web_server");
    // Define a handler for the root path
    server.Get("/", [&](const httplib::Request& req, httplib::Response& res) {
        ScAddr currentModelPointer =ctx.HelperResolveSystemIdtf("ostis_ui_current_ui_model");
        ScAddr currentModel = utils::IteratorUtils::getAnyFromSet(&ctx, currentModelPointer);
        ScAddr translationResult = utils::AgentUtils::applyActionAndGetResultIfExists(&ctx, HTMLTranslatorKeynodes::action_translate_sc_node_to_html, {currentModel}, 1000);
        ScAddr translationResultLink = utils::IteratorUtils::getAnyFromSet(&ctx, translationResult);
        if (!ctx.IsElement(translationResultLink)) {
            res.set_content("Error: translation result is invalid.", "text/html");
        }
        else {
            std::string result;
            ctx.GetLinkContent(translationResultLink, result);
            res.set_content(result, "text/html");
        }
    });
    ScAddr servableFiles = HTMLTranslatorKeynodes::servable_content;
    ScAddrVector servableFilesSet = utils::IteratorUtils::getAllWithType(&ctx, servableFiles, ScType::LinkConst);
    for (ScAddr file : servableFilesSet) {
      std::string fileName = ctx.HelperGetSystemIdtf(file);
      if(fileName.empty()) {
        throw utils::ScException("OSTIS-UI HTTPServer: servable file doesn't have an nrel_system_idtf", "");
      }
      SC_LOG_INFO("servable file detected" << fileName);
      server.Get("/" + fileName, [&](const httplib::Request& req, httplib::Response& res) {
        std::string fileContent;
        ctx.GetLinkContent(file, fileContent);
        if (fileContent.empty()) {
          throw utils::ScException("Error: servable file is empty.", "");
        }
        else {
          // let's find the format for the HTTP response

          res.set_content(fileContent, "text/plain");
        }
      });
    }
    // Start the server
    std::cout << "Starting server on port 8080..." << std::endl;
    server.listen("localhost", 8080);
}

}
