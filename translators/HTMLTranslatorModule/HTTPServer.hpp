#include "sc-agents-common/utils/AgentUtils.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-memory/sc_memory.hpp"
#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "sc-memory/utils/sc_log.hpp"
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
      std::string routePath ="/" + fileName;
      server.Get(routePath, [&](const httplib::Request& req, httplib::Response& res) {
        std::string fileContent;
        ctx.GetLinkContent(file, fileContent);
        if (fileContent.empty()) {
          SC_LOG_ERROR("HTTPServer: servable file is empty.");
          throw utils::ScException("Error: servable file is empty.", "");
        }
        else {
          ScAddr HTTPFormatLink;
          // let's find the format for the HTTP response
          std::string const FileFormatClassAlias = "_html_template_link";
          std::string const FileFormatLinkAlias = "_file_format_link";
          ScTemplate FileFormatHTTPDescription;
          FileFormatHTTPDescription.Quintuple(
                file, 
                ScType::EdgeDCommonVar, 
                ScType::NodeVarClass >> FileFormatClassAlias, 
                ScType::EdgeAccessVarPosPerm, 
                HTMLTranslatorKeynodes::nrel_format);
          FileFormatHTTPDescription.Quintuple(
                FileFormatClassAlias, 
                ScType::EdgeDCommonVar, 
                ScType::LinkVar >> FileFormatLinkAlias, 
                ScType::EdgeAccessVarPosPerm, 
                HTMLTranslatorKeynodes::nrel_mime_type);
          ctx.HelperSmartSearchTemplate(
            FileFormatHTTPDescription, 
            [&FileFormatLinkAlias, &HTTPFormatLink](ScTemplateSearchResultItem const & item) -> ScTemplateSearchRequest
            {
              item.Get(FileFormatLinkAlias, HTTPFormatLink);
              return ScTemplateSearchRequest::STOP;
            }
        );

          if (!ctx.IsElement(HTTPFormatLink)) {
            throw utils::ScException("HTTPServer: HTTPFormatLink is invalid.", "");
            SC_LOG_ERROR("HTTPServer: HTTPFormatLink is invalid.");

          }
          else {
            std::string fileFormat;
            ctx.GetLinkContent(HTTPFormatLink, fileFormat);
            res.set_content(fileContent, fileFormat);
          }
        }
      });
    }
    // Start the server
    std::cout << "Starting server on port 8080..." << std::endl;
    server.listen("localhost", 8080);
}

}
