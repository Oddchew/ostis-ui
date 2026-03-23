/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "HTMLTranslator.hpp"

#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-agents-common/utils/GenerationUtils.hpp"

#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "sc-memory/sc_debug.hpp"
#include "sc-memory/utils/sc_logger.hpp"

using namespace utils;

namespace htmlTranslationModule
{
ScAddr HTMLTranslator::TranslateScToHTML(ScAgentContext & context, ScAddr const & rootUiElement)
{
  if (!context.IsElement(rootUiElement))
  {
    SC_LOG_ERROR("HTMLTranslator: ui element is invalid.");
    throw utils::ScException(utils::ExceptionInvalidParams("HTMLTranslator: ui element is invalid.", ""));
  }

  // Check if the element is translated or have hardcoded html representation
  ScAddr answerHTMLLink =
      IteratorUtils::getAnyByOutRelation(&context, rootUiElement, HTMLTranslatorKeynodes::nrel_html_representation);
  if (context.IsElement(answerHTMLLink))
  {
    return answerHTMLLink;
  }

  ScAddr uiHTMLTemplateLink = GetUIComponentHTMLTemplate(context, rootUiElement);
  if (!context.IsElement(uiHTMLTemplateLink))
  {
    SC_LOG_ERROR("HTMLTranslator: nrel_html_template not found.");
    throw utils::ScException(
        utils::ExceptionItemNotFound("HTMLTranslator: nrel_html_template of the component class not found.", ""));
  }

  answerHTMLLink = GetTemplateAgentAnswerLink(context, rootUiElement, uiHTMLTemplateLink);
  if (!context.IsElement(answerHTMLLink))
  {
    throw utils::ScException(
        utils::ExceptionInvalidState("HTMLTranslator: SpecifiedStringTemplateAgent answer is invalid.", ""));
  }

  return answerHTMLLink;
}

ScAddr HTMLTranslator::GetUIComponentHTMLTemplate(ScAgentContext & context, ScAddr const & uiComponent)
{
  ScAddr HTMLTemplateLink;
  std::string const UIComponentClassAlias = "_ui_component_class";
  std::string const HTMLTemplateLinkAlias = "_html_template_link";

  ScTemplate HTMLTemplateTemplate;
  // component class
  HTMLTemplateTemplate.Triple(ScType::VarNodeClass >> UIComponentClassAlias, ScType::VarPermPosArc, uiComponent);
  // check that a component class is a UI component class
  HTMLTemplateTemplate.Quintuple(
      HTMLTranslatorKeynodes::concept_user_interface_component,
      ScType::VarCommonArc,
      UIComponentClassAlias,
      ScType::VarPermPosArc,
      ScKeynodes::nrel_inclusion);
  // search for an HTML template in this component class
  HTMLTemplateTemplate.Quintuple(
      UIComponentClassAlias,
      ScType::VarCommonArc,
      ScType::VarNodeLink >> HTMLTemplateLinkAlias,
      ScType::VarPermPosArc,
      HTMLTranslatorKeynodes::nrel_html_template);

  // Search only the first template result
  context.SearchByTemplateInterruptibly(
      HTMLTemplateTemplate,
      [&HTMLTemplateLinkAlias, &HTMLTemplateLink](ScTemplateSearchResultItem const & item) -> ScTemplateSearchRequest
      {
        item.Get(HTMLTemplateLinkAlias, HTMLTemplateLink);
        return ScTemplateSearchRequest::STOP;
      });

  return HTMLTemplateLink;
}

ScAddr HTMLTranslator::GetTemplateAgentAnswerLink(
    ScAgentContext & context,
    ScAddr const & uiElement,
    ScAddr const & uiHTMLTemplateLink)
{
  ScAddr const replacementConstantsSet = context.GenerateNode(ScType::ConstNode);
  GenerationUtils::generateRelationBetween(&context, replacementConstantsSet, uiElement, ScKeynodes::rrel_1);
  ScAction action = context.GenerateAction(HTMLTranslatorKeynodes::action_evaluate_specified_string_template);
  action.SetArguments(uiHTMLTemplateLink, replacementConstantsSet, HTMLTranslatorKeynodes::format_html);
  action.InitiateAndWait();
  ScStructure templateAgentAnswer = action.GetResult();
  return IteratorUtils::getAnyFromSet(&context, templateAgentAnswer);
}

}  // namespace htmlTranslationModule
