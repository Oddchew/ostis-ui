#include "sc-memory/sc_debug.hpp"
#include "sc-memory/sc_memory.hpp"
#include <httplib.h>
#include <string>
#include "keynodes/HTMLTranslatorKeynodes.hpp"

namespace htmlTranslationModule {

std::pair<std::string, std::string> GetFileContent(ScMemoryContext *ctx, ScAddr scFile) {
  std::string fileContent;
  ctx->GetLinkContent(scFile, fileContent);

  ScAddr HTTPFormatLink;
  std::string const FileFormatClassAlias = "_html_template_link";
  std::string const FileFormatLinkAlias = "_file_format_link";
  // search for mime-type of the file
  ScTemplate FileFormatHTTPDescription;
  // searching the format first
  FileFormatHTTPDescription.Quintuple(
      scFile, ScType::EdgeDCommonVar,
      ScType::NodeVarClass >> FileFormatClassAlias,
      ScType::EdgeAccessVarPosPerm, HTMLTranslatorKeynodes::nrel_format);
  // search for mime-type representation of that format
  FileFormatHTTPDescription.Quintuple(
      FileFormatClassAlias, ScType::EdgeDCommonVar,
      ScType::LinkVar >> FileFormatLinkAlias, ScType::EdgeAccessVarPosPerm,
      HTMLTranslatorKeynodes::nrel_mimetype);

  ctx->HelperSmartSearchTemplate(
      FileFormatHTTPDescription,
      [&FileFormatLinkAlias, &HTTPFormatLink](
          ScTemplateSearchResultItem const &item) -> ScTemplateSearchRequest {
        item.Get(FileFormatLinkAlias, HTTPFormatLink);
        return ScTemplateSearchRequest::STOP;
      });

  if (!ctx->IsElement(HTTPFormatLink)) {
    SC_LOG_ERROR("HTTPServer: Requested file mime type is not found.");
    SC_THROW_EXCEPTION(utils::ExceptionInvalidType,
        "HTTPServer: HTTPFormatLink is not found.");
  } else {
    // TODO: better variable types depending on the format of the file?
    std::string format;
    ctx->GetLinkContent(HTTPFormatLink, format);
    return std::make_pair(fileContent, format);
  }
}

void FileRequestHandler(ScMemoryContext *ctx, ScAddr fileAddr, const httplib::Request& req, httplib::Response& res) noexcept {
  // file not found
  if (!ctx->IsElement(fileAddr)) {
    res.status = 404;
    return;
  }

  if (ctx->GetElementType(fileAddr) == ScType::LinkConst) {
    try {
      auto [fileContent, format] = GetFileContent(ctx, fileAddr);
      res.set_content(fileContent, format);
    }
    // Thrown when file mime-type is not found
    catch (utils::ExceptionInvalidType& e) {
      res.status = 406;
      return;
    }
  }
  else {
    // unsupported media type since we can only return links, not nodes or edges
    res.status = 415;
    return;
  }
}
}
