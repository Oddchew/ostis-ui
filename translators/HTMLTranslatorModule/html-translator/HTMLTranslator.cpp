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

namespace htmlTranslationModule
{
ScAddr HTMLTranslator::TranslateScToHTML(ScAgentContext & context, ScAddr const & uiComponent)
{
  if (!context.IsElement(uiComponent))
  {
    SC_LOG_ERROR("HTMLTranslator: given UI element is invalid.");
    throw utils::ScException(utils::ExceptionInvalidParams("HTMLTranslator: given UI element is invalid.", ""));
  }

  // If element already has html translation - return it
  // TODO: we need mechanism that will allow us to regenerate component (some
  // bool flag?)
  ScAddr answerHTMLLink =
      IteratorUtils::getAnyByOutRelation(&context, uiComponent, HTMLTranslatorKeynodes::nrel_html_representation);

  if (context.IsElement(answerHTMLLink))
  {
    return answerHTMLLink;
  }

  // We are getting 2 templates here: general template for class of (given)
  // component and specific template for (given) ui component
  auto [componentHTMLTemplateLink, classHTMLTemplateLink] = GetUIComponentHTMLTemplate(context, uiComponent);

  if (!context.IsElement(componentHTMLTemplateLink) || !context.IsElement(classHTMLTemplateLink))
  {
    SC_LOG_ERROR("HTMLTranslator: nrel_html_template not found.");
    throw utils::ScException(
        utils::ExceptionItemNotFound(
            "HTMLTranslator: nrel_html_template of the component class or of "
            "element not found.",
            ""));
  }

  // This link should contain fully translated html document
  answerHTMLLink = GetAnswerLink(context, uiComponent, componentHTMLTemplateLink, classHTMLTemplateLink);

  if (!context.IsElement(answerHTMLLink))
  {
    throw utils::ScException(utils::ExceptionInvalidState("HTMLTranslator: Recursive answer element is invalid.", ""));
  }

  return answerHTMLLink;
}

ScAddrPair HTMLTranslator::GetUIComponentHTMLTemplate(ScAgentContext & context, ScAddr const & uiComponent)
{
  ScAddr componentHTMLTemplateLink, classHTMLTemplateLink;
  std::string const classAlias = "_ui_component_class";
  std::string const classHTMLTemplateLinkAlias = "_class_html_template_link";
  std::string const componentHTMLTemplateLinkAlias = "_component_html_template_link";

  ScTemplate classHTMLTemplate, componentHTMLTemplate;

  //
  // class - - - -> component
  //
  // component class:
  classHTMLTemplate.Triple(ScType::VarNodeClass >> classAlias, ScType::VarPermPosArc, uiComponent);

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
  // Search for an HTML template for this class
  classHTMLTemplate.Quintuple(
      classAlias,
      ScType::VarCommonArc,
      ScType::VarNodeLink >> classHTMLTemplateLinkAlias,
      ScType::VarPermPosArc,
      HTMLTranslatorKeynodes::nrel_html_template);

  //
  // component = = = = = => template_link_for_component
  //                ^
  //                |
  //
  //                |
  //
  //                |
  //
  //        nrel_html_template
  //
  // Search for an HTML template for this component
  componentHTMLTemplate.Quintuple(
      uiComponent,
      ScType::VarCommonArc,
      ScType::VarNodeLink >> componentHTMLTemplateLinkAlias,
      ScType::VarPermPosArc,
      HTMLTranslatorKeynodes::nrel_html_template);

  // Search only the first template result (class)
  context.SearchByTemplateInterruptibly(
      classHTMLTemplate,
      [&classHTMLTemplateLinkAlias,
       &classHTMLTemplateLink](ScTemplateSearchResultItem const & item) -> ScTemplateSearchRequest
      {
        item.Get(classHTMLTemplateLinkAlias, classHTMLTemplateLink);
        return ScTemplateSearchRequest::STOP;
      });

  // Search only the first template result (component)
  context.SearchByTemplateInterruptibly(
      componentHTMLTemplate,
      [&componentHTMLTemplateLinkAlias,
       &componentHTMLTemplateLink](ScTemplateSearchResultItem const & item) -> ScTemplateSearchRequest
      {
        item.Get(componentHTMLTemplateLinkAlias, componentHTMLTemplateLink);
        return ScTemplateSearchRequest::STOP;
      });

  return {componentHTMLTemplateLink, classHTMLTemplateLink};
}

ScAddr HTMLTranslator::GetAnswerLink(
    ScAgentContext & context,
    ScAddr const & uiComponent,
    ScAddr const & componentHTMLTemplateLink,
    ScAddr const & classHTMLTemplateLink)
{
  ScAddr linkWithHTMLRepresentation = context.GenerateLink();

  std::string componentTemplateString, classTemplateString;
  context.GetLinkContent(componentHTMLTemplateLink, componentTemplateString);
  context.GetLinkContent(classHTMLTemplateLink, classTemplateString);

  // Of what components does our uiComponent consists of?
  StringScAddrMap nestedComponents =
      ParameterRetriever::GetNestedUIComponents(context, uiComponent, classTemplateString);

  // Replace parameters with actual html code
  for (auto const & [name, parameter] : nestedComponents)
  {
    // launch recursive agent for nested components
    ScAction action = context.GenerateAction(HTMLTranslatorKeynodes::action_translate_sc_to_html);
    action.SetArguments(parameter);
    action.InitiateAndWait();
    ScStructure translationResult = action.GetResult();
    ScAddr translationResultLink = utils::IteratorUtils::getAnyFromSet(&context, translationResult);

    if (!context.IsElement(translationResultLink))
    {
      SC_LOG_ERROR("HTMLTranslator: translation of " + name + " component failed.");
      throw utils::ScException(
          utils::ExceptionItemNotFound("HTMLTranslator: translation of " + name + " component failed.", ""));
    }
    //

    std::string nestedComponentHtml;
    context.GetLinkContent(translationResultLink, nestedComponentHtml);

    // Update template content by reference
    InsertParameterValue(componentTemplateString, name, nestedComponentHtml);
  }

  context.SetLinkContent(linkWithHTMLRepresentation, componentTemplateString);

  // Generating html representation
  ScAddr arcAddr = context.GenerateConnector(ScType::CommonArc, uiComponent, linkWithHTMLRepresentation);
  context.GenerateConnector(ScType::PermPosArc, HTMLTranslatorKeynodes::nrel_html_representation, arcAddr);

  return linkWithHTMLRepresentation;
}

void HTMLTranslator::InsertParameterValue(
    std::string & componentTemplateString,
    std::string const & parameterName,
    std::string const & parameterValue)
{
  std::string foundParameterName = "";
  unsigned int start = 0, end = 0, collectMode = 0;

  for (int i = 0; i < componentTemplateString.size(); i++)
  {
    if (componentTemplateString[i] == '}')
    {
      end = i;
      if (foundParameterName == parameterName)
      {
        componentTemplateString.replace(start, end - start + 1, parameterValue);
        break;
      }

      foundParameterName.clear();
      start = end = 0;
      collectMode--;
    }

    if (collectMode == 1)
    {
      foundParameterName.push_back(componentTemplateString[i]);
    }

    if (componentTemplateString[i] == '{')
    {
      start = i;
      collectMode++;
      if (collectMode > 1)
      {
        SC_LOG_ERROR("HTMLTranslator: given html template is invalid.");
        throw utils::ScException(
            utils::ExceptionInvalidParams(
                "HTMLTranslator: given html template is invalid.", "Multiple nested brackets {...{...}} were given."));
      }
    }
  }
}

}  // namespace htmlTranslationModule
