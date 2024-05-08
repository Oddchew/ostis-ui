#include "sc-memory/sc_memory.hpp"
#include "sc-memory/sc_type.hpp"
#include <httplib.h>
#include <string>

namespace htmlTranslationModule {



class ServerWrapper {

public:
  ServerWrapper() : server_thread(), server(), mtx(), running_(false) {}

  ~ServerWrapper();

  void StartServer();

  void StopServer();

  ScAddr ConvertToScAddr(const std::string str);

  void JoinServerThread();

private:
    std::thread server_thread;
    httplib::Server server;
    std::mutex mtx;
    ScMemoryContext ctx;
    bool running_ = false;
 };
}
