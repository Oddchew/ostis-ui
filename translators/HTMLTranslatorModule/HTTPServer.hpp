#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "sc-agents-common/utils/AgentUtils.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-memory/sc_debug.hpp"
#include "sc-memory/sc_memory.hpp"
#include "sc-memory/sc_type.hpp"
#include <httplib.h>
#include <memory>
#include <string>

namespace htmlTranslationModule {

ScAddr ConvertToScAddr(const std::string str);

std::pair<std::string, std::string> GetFileContent(ScMemoryContext *ctx, ScAddr scFile);

void FileRequestHandler(ScMemoryContext *ctx, ScAddr fileAddr, const httplib::Request& req, httplib::Response& res) noexcept;

class ServerWrapper {
public:
  ServerWrapper() : server_thread_(), server_(), mtx_(), running_(false) {}

  ~ServerWrapper() {
    StopServer();
  }

  void StartServer() {
    // Instantiate the server
    std::lock_guard<std::mutex> lock(mtx_);
    running_ = true;
    if (ctx_ == nullptr || !ctx_->IsValid()) {
      ctx_ = std::make_shared<ScMemoryContext>(sc_access_lvl_make_min, "ostis_ui_web_server");
    }
    server_thread_ = std::thread([this]() {
    // TODO: use newer ScMemoryContext constructor
    // Define a handler for the root path
    server_.Get("/", [&](const httplib::Request &req, httplib::Response &res) {
      // TODO: use user-specific current model
      ScAddr currentModelPointer =
          ctx_->HelperResolveSystemIdtf("ostis_ui_current_ui_model");
      ScAddr currentModel =
          utils::IteratorUtils::getAnyFromSet(ctx_.get(), currentModelPointer);
      ScAddr translationResult =
          utils::AgentUtils::applyActionAndGetResultIfExists(
              ctx_.get(), HTMLTranslatorKeynodes::action_translate_sc_node_to_html,
              {currentModel}, 1000);
      ScAddr translationResultLink =
          utils::IteratorUtils::getAnyFromSet(ctx_.get(), translationResult);
      if (!ctx_->IsElement(translationResultLink)) {
        res.set_content("Error: translation result is invalid.", "text/html");
      } else {
        std::string result;
        ctx_->GetLinkContent(translationResultLink, result);
        // TODO: should we retrieve the link format rather than hardcode it?
        // current solution is more performant
        res.set_content(result, "text/html");
      }
    });

      // an endpoint to resolve sc-files using ScAddr
      server_.Get("/files/by_sc_addr/:addr", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string addr = req.path_params.at("addr");
        ScAddr::HashType value;
        ScAddr fileAddr = ConvertToScAddr(addr);
        FileRequestHandler(ctx_.get(), fileAddr, req, res);
      });
      // an endpoint to resolve sc-files using system identifier
      server_.Get("/files/by_system_idtf/:system_idtf", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string system_idtf = req.path_params.at("system_idtf");
        ScAddr fileAddr;

        try {
          fileAddr = ctx_->HelperResolveSystemIdtf(system_idtf);
        }
        catch (utils::ExceptionInvalidParams& e) {
          res.status = 404;
        }

        FileRequestHandler(ctx_.get(), fileAddr, req, res);
      });

      // Start the server
      std::cout << "Starting server on port 8080..." << std::endl;
      server_.listen("localhost", 8080);
  });
  }

  void StopServer() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (running_) {
        // Stop the server
        server_.stop();
        ctx_->Destroy();
        running_ = false;
    }
}

void JoinServerThread() {
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
}

private:
    std::thread server_thread_;
    httplib::Server server_;
    std::mutex mtx_;
    std::shared_ptr<ScMemoryContext> ctx_;
    bool running_ = false;


 };
}
