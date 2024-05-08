#include <httplib.h>
#include <HTTPServer.hpp>
#include <string>
#include <HTTPRequestHandler.hpp>


namespace htmlTranslationModule {


void ServerWrapper::StartServer() {
  // Instantiate the server
  std::lock_guard<std::mutex> lock(mtx);
  running_ = true;
  server_thread = std::thread([this]() {
    // TODO: use newer ScMemoryContext constructor
    // Define a handler for the root path
    server.Get(
        "/", [&](const httplib::Request &req, httplib::Response &res) {
          RetrieveCurrentUIHandler(&ctx, req, res);
        });

    // an endpoint to resolve sc-files using ScAddr
    server.Get("/files/by_sc_addr/:addr",
               [&](const httplib::Request &req, httplib::Response &res) {
                 FileByAddrRequestHandler(&ctx, req, res);
               });
    // an endpoint to resolve sc-files using system identifier
    server.Get("/files/by_system_idtf/:system_idtf",
               [&](const httplib::Request &req, httplib::Response &res) {
                 FileBySystemIdtfRequestHandler(&ctx, req, res);
               });

    // Start the server
    std::cout << "Starting server on port 8080..." << std::endl;
    server.listen("localhost", 8080);
  });
}
void ServerWrapper::StopServer() {
  std::lock_guard<std::mutex> lock(mtx);
  if (running_) {
    // Stop the server
    server.stop();
    running_ = false;
  }
}
void ServerWrapper::JoinServerThread() {
  if (server_thread.joinable()) {
    server_thread.join();
  }
}
ServerWrapper::~ServerWrapper() { StopServer(); }
} // namespace htmlTranslationModule
