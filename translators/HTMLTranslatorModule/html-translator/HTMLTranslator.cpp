/*
 * This source file is part of an OSTIS project. For the latest info, see
 * http://ostis.net Distributed under the MIT License (See accompanying file
 * COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "HTMLTranslator.hpp"
#include "parameter-retriever/ParameterRetriever.hpp"

#include <sc-agents-common/utils/IteratorUtils.hpp>

#include <keynodes/HTMLTranslatorKeynodes.hpp>
#include <sc-memory/sc_addr.hpp>
#include <sc-memory/sc_debug.hpp>
#include <sc-memory/sc_type.hpp>
#include <sc-memory/utils/sc_logger.hpp>
#include <string>

using namespace utils;

namespace htmlTranslationModule {
ScAddr HTMLTranslator::TranslateScToHTML(ScAgentContext &context,
                                         ScAddr const &uiComponent) {

  if (!context.IsElement(uiComponent)) {
    SC_LOG_ERROR("HTMLTranslator: given UI element is invalid.");
    throw utils::ScException(utils::ExceptionInvalidParams(
        "HTMLTranslator: given UI element is invalid.", ""));
  }

  // Check if the element is already translated
  ScAddr answerHTMLLink = IteratorUtils::getAnyByOutRelation(
      &context, uiComponent, HTMLTranslatorKeynodes::nrel_html_representation);

  if (context.IsElement(answerHTMLLink)) {
    return answerHTMLLink;
  }

  ScAddr uiHTMLTemplateLink = GetUIComponentHTMLTemplate(context, uiComponent);

  if (!context.IsElement(uiHTMLTemplateLink)) {
    SC_LOG_ERROR("HTMLTranslator: nrel_html_template not found.");
    throw utils::ScException(utils::ExceptionItemNotFound(
        "HTMLTranslator: nrel_html_template of the component class not found.",
        ""));
  }

  answerHTMLLink = GetAnswerLink(context, uiComponent, uiHTMLTemplateLink);

  if (!context.IsElement(answerHTMLLink)) {
    throw utils::ScException(utils::ExceptionInvalidState(
        "HTMLTranslator: Recursive answer is invalid.", ""));
  }

  return answerHTMLLink;
}

ScAddr HTMLTranslator::GetUIComponentHTMLTemplate(ScAgentContext &context,
                                                  ScAddr const &uiComponent) {
  ScAddr uiHTMLTemplateLink;
  std::string const UIComponentClassAlias = "_ui_component_class";
  std::string const HTMLTemplateLinkAlias = "_html_template_link";

  ScTemplate HTMLTemplateTemplate;

  //
  // class - - - -> component
  //
  // component class:
  HTMLTemplateTemplate.Triple(ScType::VarNodeClass >> UIComponentClassAlias,
                              ScType::VarPermPosArc, uiComponent);

  //
  // class = = = = = => template_link_for_class
  //             ^
  //             |
  //
  //             |
  //
  //             |
  //
  //      nrel_html_template
  //
  // Search for an HTML template for this component class
  HTMLTemplateTemplate.Quintuple(UIComponentClassAlias, ScType::VarCommonArc,
                                 ScType::VarNodeLink >> HTMLTemplateLinkAlias,
                                 ScType::VarPermPosArc,
                                 HTMLTranslatorKeynodes::nrel_html_template);

  // Search only the first template result
  context.SearchByTemplateInterruptibly(
      HTMLTemplateTemplate,
      [&HTMLTemplateLinkAlias, &uiHTMLTemplateLink](
          ScTemplateSearchResultItem const &item) -> ScTemplateSearchRequest {
        item.Get(HTMLTemplateLinkAlias, uiHTMLTemplateLink);
        return ScTemplateSearchRequest::STOP;
      });

  return uiHTMLTemplateLink;
}

ScAddr HTMLTranslator::GetAnswerLink(ScAgentContext &context,
                                     ScAddr const &uiComponent,
                                     ScAddr const &uiHTMLTemplateLink) {

  ScAddr linkWithHTMLRepresentation = context.GenerateLink();

  std::string templateString;
  context.GetLinkContent(uiHTMLTemplateLink, templateString);

  StringScAddrMap nestedComponents = ParameterRetriever::GetNestedUIComponents(
      context, uiComponent, templateString);

  for (auto const &[name, parameter] : nestedComponents) {

    // launch recursive agent for nested components
    ScAction action = context.GenerateAction(
        HTMLTranslatorKeynodes::action_translate_sc_to_html);
    action.SetArguments(parameter);
    action.InitiateAndWait();
    ScStructure translationResult = action.GetResult();
    ScAddr translationResultLink =
        utils::IteratorUtils::getAnyFromSet(&context, translationResult);

    if (!context.IsElement(translationResultLink)) {
      SC_LOG_ERROR("HTMLTranslator: translation of " + name +
                   " component failed.");
      throw utils::ScException(utils::ExceptionItemNotFound(
          "HTMLTranslator: translation of " + name + " component failed.", ""));
    }

    std::string nestedComponentHtml;
    context.GetLinkContent(translationResultLink, nestedComponentHtml);

    // Update template content by reference
    InsertParameterValue(templateString, name, nestedComponentHtml);
  }

  context.SetLinkContent(linkWithHTMLRepresentation, templateString);

  // Generating html representation
  ScAddr arcAddr = context.GenerateConnector(ScType::CommonArc, uiComponent,
                                             linkWithHTMLRepresentation);
  context.GenerateConnector(ScType::PermPosArc,
                            HTMLTranslatorKeynodes::nrel_html_representation,
                            arcAddr);

  return linkWithHTMLRepresentation;
}

void HTMLTranslator::InsertParameterValue(std::string &templateString,
                                          const std::string &parameterName,
                                          const std::string &parameterValue) {

  std::string foundParameterName = "";
  unsigned int start = 0, end = 0, collectMode = 0;

  // We know that ParameterRetriever ensures that there are no nested brackets
  for (int i = 0; i < templateString.size(); i++) {
    if (templateString[i] == '}') {

      end = i;
      if (foundParameterName == parameterName) {
        templateString.replace(start, end - start + 1, parameterValue);
        break;
      }

      foundParameterName.clear();
      start = end = 0;
      collectMode--;
    }

    if (collectMode == 1) {
      foundParameterName.push_back(templateString[i]);
    }

    if (templateString[i] == '{') {
      start = i;
      collectMode++;
    }
  }
}

} // namespace htmlTranslationModule
