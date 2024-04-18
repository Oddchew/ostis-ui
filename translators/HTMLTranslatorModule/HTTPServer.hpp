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

    // Start the server
    std::cout << "Starting server on port 8080..." << std::endl;
    server.listen("localhost", 8080);
}

}
