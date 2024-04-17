/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "StringTemplateRenderer.hpp"

#include "keynodes/SpecifiedStringTemplateKeynodes.hpp"

#include "sc-agents-common/keynodes/coreKeynodes.hpp"
#include "sc-agents-common/utils/CommonUtils.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-agents-common/utils/AgentUtils.hpp"
#include "sc-memory/sc_debug.hpp"
#include "sc-memory/sc_type.hpp"
#include "sc-search/search_keynodes.h"

#include <inja.hpp>
#include <nlohmann/json.hpp>

using namespace utils;

namespace specifiedStringTemplateModule
{

std::string StringTemplateRenderer::RenderStringTemplate(ScMemoryContext & context, ScAddr const & stringTemplateLink, ScAddr const & stringTemplateLinkReplacements, ScAddr const & stringFormatAddr)
{
  // Get string template sc-link content
  std::string templateString;
  bool const templateStringLinkExists = context.GetLinkContent(stringTemplateLink, templateString);

  if (!templateStringLinkExists)
  {
    throw utils::ScException(utils::ExceptionItemNotFound("StringTemplateRenderer: string template link has no content.", ""));
  }

  ScAddr const variablesSetAddr = IteratorUtils::getAnyByOutRelation(
        &context, stringTemplateLink, SpecifiedStringTemplateKeynodes::nrel_variable_template);

  // If link has no variables, return it. Nothing to render
  if (!context.IsElement(variablesSetAddr))
  {
    std::string result = inja::render(templateString, nlohmann::json());
    SC_LOG_INFO("StringTemplateRenderer: there is no variables. Rendered string is " << result);
    return result;
  }

  nlohmann::json variableTemplateValues;
  ScAddr variableAddr;
  std::string variableContent;
  ScAddr templateAddr;
  ScAddrVector const variableTemplatesVector = IteratorUtils::getAllWithType(&context, variablesSetAddr, ScType::NodeConst);
  // Iterate over all the specified in sc-link string variables
  for (ScAddr const & variableTemplateNode : variableTemplatesVector)
  {
    variableAddr = IteratorUtils::getAnyByOutRelation(&context, variableTemplateNode, SpecifiedStringTemplateKeynodes::rrel_variable);
    templateAddr = IteratorUtils::getAnyByOutRelation(&context, variableTemplateNode, SpecifiedStringTemplateKeynodes::rrel_template);

    // Get variable and corresponding template to find variable value
    if (!context.IsElement(variableAddr) || !context.IsElement(templateAddr))
    {
      throw utils::ScException(utils::ExceptionItemNotFound("StringTemplateRenderer: string template variables are specified incorrectly.", ""));
    }
    context.GetLinkContent(variableAddr, variableContent);
    SC_LOG_DEBUG("StringTemplateRenderer: found variable " << variableContent);

    ScAddr const keyScElement = IteratorUtils::getAnyByOutRelation(&context, templateAddr, scAgentsCommon::CoreKeynodes::rrel_key_sc_element);
    ScAddr keyScElementValue;

    ScTemplate scTemplate;
    ScTemplateParams params;
    // Fill ScTemplateParams if valid replacements are passed as a parameter (e.g. real user interface component to pass in sc-template)
    if (context.IsElement(stringTemplateLinkReplacements))
    {
      params = GetScTemplateParamsFromTemplateReplacements(context, templateAddr, stringTemplateLinkReplacements);
    }

    // Build template from knowledge base address and search by it
    try
    {
      context.HelperBuildTemplate(scTemplate, templateAddr, params);
      // We need only one result to find
      context.HelperSmartSearchTemplate(scTemplate, [&keyScElement, &keyScElementValue](ScTemplateSearchResultItem const & item) -> ScTemplateSearchRequest
      {
        item.Get(keyScElement, keyScElementValue);
        return ScTemplateSearchRequest::STOP;
      });
    }
    catch (ScException const & exception)
    {
      SC_LOG_ERROR(exception.Message());
    }
    if (!context.IsElement(keyScElementValue))
    {
      throw utils::ScException(utils::ExceptionItemNotFound("StringTemplateRenderer: template is not found.", ""));
    }
    ScType keyScElementValueType = context.GetElementType(keyScElementValue);

    if (keyScElementValueType == ScType::LinkConst) 
    {
      std::string keyScElementValueContent;
      context.GetLinkContent(keyScElementValue, keyScElementValueContent);

    // Fill json to replace variables with their values in the string template
    variableTemplateValues[variableContent] = keyScElementValueContent;
    SC_LOG_DEBUG("StringTemplateRenderer: add mapping " << variableContent << ": " << keyScElementValueContent);
    }
    else if (keyScElementValueType.IsNode())
    {
    ScAddr translateActionClass = IteratorUtils::getAnyByOutRelation(&context, stringFormatAddr, SpecifiedStringTemplateKeynodes::nrel_translation_action);
    if (!context.IsElement(translateActionClass))
    {
      throw utils::ScException(utils::ExceptionItemNotFound("StringTemplateRenderer: can't translate a node to the specified format: action not found.", ""));
    }
    SC_LOG_DEBUG("StringTemplateRenderer: found action class for translation to a format: " << context.HelperGetSystemIdtf(translateActionClass));
    std::stringstream dependentComponentsTranslation;

    // check if it's a set
      if (keyScElementValueType == ScType::NodeConstTuple) {
        // is it an ordered set?
        ScAddr currentElement = IteratorUtils::getAnyByOutRelation(&context, keyScElementValue, scAgentsCommon::CoreKeynodes::rrel_1);
        if (context.IsElement(currentElement))
        {
          std::string currentComponentTranslation;
          // oriented set
          while (context.IsElement(currentElement))
          {
              ScAddr const templateAgentAnswer = AgentUtils::applyActionAndGetResultIfExists(
                    &context, translateActionClass, {currentElement}, 300);
              ScAddr answerLink = IteratorUtils::getAnyFromSet(&context, templateAgentAnswer);
              context.GetLinkContent(answerLink, currentComponentTranslation);
              dependentComponentsTranslation << currentComponentTranslation;
            currentElement = IteratorUtils::getNextFromSet(&context, keyScElementValue, currentElement);
          }
        }
        else
        {
          ScAddrVector components = IteratorUtils::getAllWithType(&context, keyScElementValue, ScType::NodeConst);
          std::string currentComponentTranslation;
          for (ScAddr const & component : components)
          {
            ScAddr const templateAgentAnswer = AgentUtils::applyActionAndGetResultIfExists(
                  &context, translateActionClass, {component}, 300);
            ScAddr answerLink = IteratorUtils::getAnyFromSet(&context, templateAgentAnswer);
            context.GetLinkContent(answerLink, currentComponentTranslation);
            dependentComponentsTranslation << currentComponentTranslation;
          }
        }
        variableTemplateValues[variableContent] = dependentComponentsTranslation.str();
      }
      // not a set
      else {
        std::string currentComponentTranslation;
        ScAddr const templateAgentAnswer = AgentUtils::applyActionAndGetResultIfExists(
                    &context, translateActionClass, {keyScElementValue}, 300);
              ScAddr answerLink = IteratorUtils::getAnyFromSet(&context, templateAgentAnswer);
              context.GetLinkContent(answerLink, currentComponentTranslation);
              dependentComponentsTranslation << currentComponentTranslation;
      }
    }
    else 
    {
      throw utils::ScException(utils::ExceptionItemNotFound("StringTemplateRenderer: template found a key element of an unsupported type.", ""));
   }
  }

  // Replace variables with their values in the string template
  std::string result = inja::render(templateString, variableTemplateValues);
  SC_LOG_INFO("StringTemplateRenderer: rendered string is " << result);
  return result;
}

ScTemplateParams StringTemplateRenderer::GetScTemplateParamsFromTemplateReplacements(ScMemoryContext & context, ScAddr const & templateAddr, ScAddr const & stringTemplateLinkReplacements)
{
  ScAddr const templateReplacementsSet = IteratorUtils::getAnyByOutRelation(&context, templateAddr, SpecifiedStringTemplateKeynodes::nrel_replacements_variables);

  if (!context.IsElement(templateReplacementsSet))
  {
    throw utils::ScException(utils::ExceptionItemNotFound("StringTemplateRenderer: template has no nrel_replacements_variables.", ""));
  }

  ScTemplateParams params;
  ScAddr replacementVariable = IteratorUtils::getAnyByOutRelation(&context, templateReplacementsSet, scAgentsCommon::CoreKeynodes::rrel_1);
  ScAddr replacementValue = IteratorUtils::getAnyByOutRelation(&context, stringTemplateLinkReplacements, scAgentsCommon::CoreKeynodes::rrel_1);
  while (context.IsElement(replacementVariable) && context.IsElement(replacementValue))
  {
    params.Add(replacementVariable, replacementValue);
    SC_LOG_DEBUG("StringTemplateRenderer: add replacement " << context.HelperGetSystemIdtf(replacementVariable) << ": " << context.HelperGetSystemIdtf(replacementValue));
    replacementVariable = IteratorUtils::getNextFromSet(&context, templateReplacementsSet, replacementVariable);
    replacementValue = IteratorUtils::getNextFromSet(&context, stringTemplateLinkReplacements, replacementValue);
  }

  return params;
}

} // namespace specifiedStringTemplateModule
