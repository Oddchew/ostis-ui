#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "sc-agents-common/utils/AgentUtils.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-memory/sc_memory.hpp"
#include <HTTPRequestHandler.hpp>

namespace htmlTranslationModule {

ScAddr ConvertToScAddr(const std::string str) {
    ScAddr::HashType value;
    std::istringstream iss(str);
    iss >> value;
    return ScAddr(value);
}

std::string GetFileMimetype(ScMemoryContext *ctx, ScAddr scFile) {
  std::string fileContent;

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
    return format;
  }
}

void FileRetriever(ScMemoryContext *ctx, ScAddr& fileAddr, const httplib::Request& req, httplib::Response& res) noexcept {
  // file not found
  if (!ctx->IsElement(fileAddr)) {
    res.status = 404;
    return;
  }
  if (ctx->GetElementType(fileAddr) == ScType::LinkConst) {
    try {
      std::string fileContent;
      const bool getContentResult = ctx->GetLinkContent(fileAddr, fileContent);
      if (!getContentResult) {
        res.status = 500;
        return;
      }
      std::string format = GetFileMimetype(ctx, fileAddr);
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

void RetrieveCurrentUIHandler(ScMemoryContext *ctx, const httplib::Request& req, httplib::Response& res) noexcept {
      // TODO: use user-specific current model
      ScAddr currentModelPointer = ctx->HelperResolveSystemIdtf("ostis_ui_current_ui_model");
      ScAddr currentModel = utils::IteratorUtils::getAnyFromSet(ctx, currentModelPointer);
      ScAddr translationResult =
          utils::AgentUtils::applyActionAndGetResultIfExists(
              ctx, HTMLTranslatorKeynodes::action_translate_sc_node_to_html,
              {currentModel}, 1000);
      ScAddr translationResultLink = utils::IteratorUtils::getAnyFromSet(ctx, translationResult);
      if (!ctx->IsElement(translationResultLink)) {
        res.set_content("Error: translation result is invalid.", "text/html");
      } 
      else {
        std::string result;
        ctx->GetLinkContent(translationResultLink, result);
        // TODO: should we retrieve the link format rather than hardcode it?
        // current solution is more performant
        res.set_content(result, "text/html");
      }
}



void FileByAddrRequestHandler(ScMemoryContext *ctx, const httplib::Request& req, httplib::Response& res) noexcept {
  ScAddr fileAddr;
  if (!req.has_param("addr")) {
    res.status = 400;
    return;
  }
  const std::string addr = req.path_params.at("addr");
  fileAddr = ConvertToScAddr(addr);
  FileRetriever(ctx, req, res);
}

void FileBySystemIdtfRequestHandler(ScMemoryContext *ctx, const httplib::Request& req, httplib::Response& res) noexcept {
  ScAddr fileAddr;
  if (!req.has_param("system_idtf")) {
    res.status = 400;
    return;
  }
  const std::string system_idtf = req.path_params.at("system_idtf");
  try {
      fileAddr = ctx->HelperResolveSystemIdtf(system_idtf);
    } 
  catch (utils::ExceptionInvalidParams &e) {
      res.status = 404;
  }
  FileRetriever(ctx, req, res);
}

}
