#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-memory/sc_memory.hpp"
#include <HTTPRequestHandler.hpp>

#include <sc-memory/sc_agent.hpp>

namespace htmlTranslationModule
{

ScAddr HTTPRequestHandler::ConvertToScAddr(std::string const & str)
{
  ScAddr::HashType value;
  std::istringstream iss(str);
  iss >> value;
  return ScAddr(value);
}

std::string HTTPRequestHandler::GetFileMimetype(ScMemoryContext * context, ScAddr const & scFile)
{
  std::string fileContent;

  ScAddr HTTPFormatLink;
  std::string const FileFormatClassAlias = "_html_template_link";
  std::string const FileFormatLinkAlias = "_file_format_link";

  // search for mime-type of the file
  ScTemplate FileFormatHTTPDescription;

  // searching the format first
  FileFormatHTTPDescription.Quintuple(
      scFile,
      ScType::VarCommonArc,
      ScType::VarNodeClass >> FileFormatClassAlias,
      ScType::VarPermPosArc,
      HTMLTranslatorKeynodes::nrel_format);

  // search for mime-type representation of that format
  FileFormatHTTPDescription.Quintuple(
      FileFormatClassAlias,
      ScType::VarCommonArc,
      ScType::VarNodeLink >> FileFormatLinkAlias,
      ScType::VarPermPosArc,
      HTMLTranslatorKeynodes::nrel_mimetype);

  context->SearchByTemplateInterruptibly(
      FileFormatHTTPDescription,
      [&FileFormatLinkAlias, &HTTPFormatLink](ScTemplateSearchResultItem const & item) -> ScTemplateSearchRequest
      {
        item.Get(FileFormatLinkAlias, HTTPFormatLink);
        return ScTemplateSearchRequest::STOP;
      });

  if (!context->IsElement(HTTPFormatLink))
  {
    SC_LOG_ERROR("HTTPRequestHandler: Requested file mime type is not found.");
    SC_THROW_EXCEPTION(utils::ExceptionInvalidType, "HTTPRequestHandler: HTTPFormatLink is not found.");
  }
  else
  {
    // TODO: better variable types depending on the format of the file?
    std::string format;
    context->GetLinkContent(HTTPFormatLink, format);
    return format;
  }
}

void HTTPRequestHandler::FileRetriever(
    ScMemoryContext * context,
    ScAddr & fileAddr,
    httplib::Request const & req,
    httplib::Response & res) noexcept
{
  // file not found
  if (!context->IsElement(fileAddr))
  {
    SC_LOG_ERROR("HTTPRequestHandler: File addr is not valid.");
    res.status = 404;
    return;
  }

  try
  {
    std::string fileContent;
    bool const getContentResult = context->GetLinkContent(fileAddr, fileContent);
    if (!getContentResult)
    {
      SC_LOG_ERROR("HTTPRequestHandler: File addr is not a file.");
      res.status = 500;
      return;
    }
    std::string format = GetFileMimetype(context, fileAddr);
    res.set_content(fileContent, format);
  }
  // Thrown when file mime-type is not found
  catch (utils::ExceptionInvalidType & exception)
  {
    SC_LOG_ERROR("HTTPRequestHandler: " << exception.Message());
    res.status = 406;
    return;
  }
}

void HTTPRequestHandler::RetrieveCurrentUIHandler(httplib::Request const & req, httplib::Response & res)
{
  // TODO: use user-specific current model
  ScAgentContext * context = new ScAgentContext();

  ScAddr const currentModel =
      utils::IteratorUtils::getAnyFromSet(context, HTMLTranslatorKeynodes::concept_current_ostis_ui_model);
  if (!context->IsElement(currentModel))
  {
    res.set_content("Error: current ui model is not found.", "text/html");
    return;
  }

  ScAction action = context->GenerateAction(HTMLTranslatorKeynodes::action_translate_sc_to_html);
  action.SetArguments(currentModel);
  action.InitiateAndWait();

  ScStructure translationResult = action.GetResult();
  ScAddr translationResultLink = utils::IteratorUtils::getAnyFromSet(context, translationResult);
  if (!context->IsElement(translationResultLink))
  {
    res.set_content("Error: translation result is invalid.", "text/html");
  }
  else
  {
    std::string result;
    context->GetLinkContent(translationResultLink, result);
    // TODO: should we retrieve the link format rather than hardcode it?
    // current solution is more performant
    res.set_content(result, "text/html");
  }
}

void HTTPRequestHandler::FileByAddrRequestHandler(httplib::Request const & req, httplib::Response & res) noexcept
{
  // TODO: use user-specific context
  ScMemoryContext * context = new ScMemoryContext();
  ScAddr fileAddr;
  if (!req.path_params.count("addr"))
  {
    res.status = 400;
    return;
  }
  std::string const addr = req.path_params.at("addr");
  fileAddr = ConvertToScAddr(addr);
  FileRetriever(context, fileAddr, req, res);
}

void HTTPRequestHandler::FileBySystemIdtfRequestHandler(httplib::Request const & req, httplib::Response & res) noexcept
{
  ScMemoryContext * context = new ScMemoryContext();
  ScAddr fileAddr;
  if (!req.path_params.count("system_idtf"))
  {
    res.status = 400;
    return;
  }
  std::string const system_idtf = req.path_params.at("system_idtf");
  try
  {
    fileAddr = context->ResolveElementSystemIdentifier(system_idtf);
  }
  catch (utils::ExceptionInvalidParams & e)
  {
    res.status = 404;
  }
  FileRetriever(context, fileAddr, req, res);
}

}  // namespace htmlTranslationModule
