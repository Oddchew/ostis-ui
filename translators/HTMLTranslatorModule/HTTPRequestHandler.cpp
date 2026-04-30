#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-memory/sc_memory.hpp"
#include <HTTPRequestHandler.hpp>

#include <sc-memory/sc_agent.hpp>

namespace htmlTranslationModule
{

void HTTPRequestHandler::RetrieveCurrentUIHandler(httplib::Request const & req, httplib::Response & res)
{
  // TODO: use user-specific current model
  ScAgentContext * context = new ScAgentContext();

  // TODO: Получить user_id из запроса (cookie, header, session)
  // Пока используем заглушку
  ScAddr userProfile = context->ResolveElementSystemIdentifier("some_user");

  if (!context->IsElement(userProfile))
  {
    SC_LOG_WARNING("HTTPRequestHandler: User profile not found, using default.");
    userProfile = ScAddr::Empty;
  }

  ScAddr const currentModel =
      utils::IteratorUtils::getAnyFromSet(context, HTMLTranslatorKeynodes::concept_current_ostis_ui_model);
  if (!context->IsElement(currentModel))
  {
    res.set_content("Error: current ui model is not found.", "text/html");
    return;
  }

  // ЗАПУСКАЕМ АДАПТАЦИЮ ПЕРЕД ТРАНСЛЯЦИЕЙ
  if (context->IsElement(userProfile))
  {
    ScAction adaptationAction = context->GenerateAction(HTMLTranslatorKeynodes::action_adapt_ui_for_user);
    adaptationAction.SetArguments(userProfile, currentModel);
    adaptationAction.InitiateAndWait();

    if (!adaptationAction.IsFinishedSuccessfully())
    {
      SC_LOG_WARNING("HTTPRequestHandler: Adaptation failed, using original model.");
    }
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

}  // namespace htmlTranslationModule
