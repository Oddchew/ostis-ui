#include "sc-memory/sc_memory.hpp"
#include <httplib.h>

namespace htmlTranslationModule {

std::string GetFileMimetype(ScMemoryContext *ctx, ScAddr scFile);
void RetrieveCurrentUIHandler(ScMemoryContext *ctx ,const httplib::Request& req, httplib::Response& res) noexcept;
void FileBySystemIdtfRequestHandler(ScMemoryContext *ctx, const httplib::Request& req, httplib::Response& res) noexcept;
void FileByAddrRequestHandler(ScMemoryContext *ctx, const httplib::Request& req, httplib::Response& res) noexcept;
void FileRetriever(ScMemoryContext *ctx, const httplib::Request& req, httplib::Response& res) noexcept;

}
