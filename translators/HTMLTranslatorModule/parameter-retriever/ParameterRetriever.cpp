/*
 * This source file is part of an OSTIS project. For the latest info, see
 * http://ostis.net Distributed under the MIT License (See accompanying file
 * COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "ParameterRetriever.hpp"
#include "keynodes/HTMLTranslatorKeynodes.hpp"

#include <sc-memory/sc_addr.hpp>
#include <sc-memory/sc_template.hpp>
#include <sc-memory/sc_type.hpp>
#include <string>
#include <vector>

using namespace utils;

namespace htmlTranslationModule
{

StringScAddrMap ParameterRetriever::GetNestedUIComponents(
    ScAgentContext & context,
    ScAddr const & uiComponent,
    std::string const & classTemplateString)
{
  StringScAddrMap nestedComponents;

  ScAddrVector unmappedParameters;

  StringScAddrMap parameterClasses = ExtractParameterClasses(context, classTemplateString);

  for (auto const & [name, node] : parameterClasses)
  {
    std::string const parameterAlias = "_parameter_" + name;
    ScTemplate parameterTemplate;
    ScAddr parameter;

    //
    // parameter_class - - - -> parameter
    //
    parameterTemplate.Triple(node, ScType::VarPermPosArc, ScType::VarNode >> parameterAlias);

    //
    // ui_component = = = = = => parameter
    //                   ^
    //                   |
    //
    //                   |
    //
    //                   |
    //
    //           nrel_parameter
    //
    parameterTemplate.Quintuple(
        uiComponent,
        ScType::VarCommonArc,
        parameterAlias,
        ScType::VarPermPosArc,
        HTMLTranslatorKeynodes::nrel_parameter);

    // Search for all parameters
    context.SearchByTemplate(
        parameterTemplate,
        [&parameterAlias, &parameter, &unmappedParameters](
            ScTemplateSearchResultItem const & item)
        {
          item.Get(parameterAlias, parameter);
          unmappedParameters.push_back(parameter);
        });
  }

  // We check here which link with ID corresponds to which component.
  for (auto const & e : unmappedParameters)
  {
    ScTemplate parameterIDTemplate;
    std::string const parameterIDLinkAlias = "_parameter_id_link";
    ScAddr linkWithID;

    //
    // parameter = = = = = => _parameter_id_link
    //                ^
    //                |
    //
    //                |
    //
    //                |
    //
    //      nrel_html_parameter_id
    //
    parameterIDTemplate.Quintuple(
        e,
        ScType::VarCommonArc,
        ScType::VarNodeLink >> parameterIDLinkAlias,
        ScType::VarPermPosArc,
        HTMLTranslatorKeynodes::nrel_html_parameter_id);

    context.SearchByTemplateInterruptibly(
        parameterIDTemplate,
        [&parameterIDLinkAlias, &linkWithID, &unmappedParameters](
            ScTemplateSearchResultItem const & item) -> ScTemplateSearchRequest
        {
          item.Get(parameterIDLinkAlias, linkWithID);
          return ScTemplateSearchRequest::STOP;
        });

    std::string idLink;
    context.GetLinkContent(linkWithID, idLink);
    nestedComponents[idLink] = e;
  }

  return nestedComponents;
}

/*!
 * @brief Extracts parameters in brackets and returns corresponding ScNodeClasses.
 * @param classTemplateString String that contains class html template.
 * @return Map of Class name and corresponding sc element.
 *
 * We recieve something like:
 *
 * <!DOCTYPE html>
 * <html lang="en">
 *     <head>
 *         {concept_html_head}
 *     </head>
 *     <body>
 *         {concept_html_body}
 *     </body>
 * </html>
 *
 * Then we need to find class nodes which names are given in a brackets as
 * parameters.
 *
 * In this case these class nodes are: concept_html_head, concept_html_body.
 *
 * concept_html_head, concept_html_body are so called "parameter classes".
 */
StringScAddrMap ParameterRetriever::ExtractParameterClasses(
    ScAgentContext & context,
    std::string const & classTemplateString)
{
  StringScAddrMap extractedNamesAndClasses;
  std::vector<std::string> extractedParameters = ExtractBracketsContent(classTemplateString);

  for (auto const & e : extractedParameters)
  {
    ScAddr parameterClass = context.SearchElementBySystemIdentifier(e);

    if (!context.IsElement(parameterClass))
    {
      SC_LOG_ERROR("ParameterRetriever: given parameter class is invalid.");
      throw utils::ScException(
          utils::ExceptionInvalidParams(
              "ParameterRetriever: given parameter class is invalid.",
              "Class with the name provided as a parameter in the html template "
              "is invalid."));
    }

    extractedNamesAndClasses[e] = parameterClass;
  }

  return extractedNamesAndClasses;
}

/*!
 * @brief Extracts strings inside brackets.
 * @param str String with content.
 * @return Vector with nested strings.
 *
 * We recieve something like:
 * "{something} blabla {any}"
 *
 * Extract and return: "something", "any".
 *
 */
std::vector<std::string> ExtractBracketsContent(std::string const & str)
{
  std::vector<std::string> extractedContent;
  std::string tempStr = "";
  unsigned int collectMode = 0;

  // Assuming there are no nested brackets like: {...{...{}}}
  for (int i = 0; i < str.size(); i++)
  {
    if (str[i] == '}')
    {
      extractedContent.push_back(tempStr);
      tempStr.clear();
      collectMode--;
    }

    if (collectMode == 1)
    {
      tempStr.push_back(str[i]);
    }

    if (str[i] == '{')
    {
      collectMode++;

      if (collectMode > 1)
      {
        SC_LOG_ERROR("ParameterRetriever: given html template is invalid.");
        throw utils::ScException(
            utils::ExceptionInvalidParams(
                "ParameterRetriever: given html template is invalid.",
                "Multiple nested brackets {...{...}} were given."));
      }
    }
  }
  return extractedContent;
}
}  // namespace htmlTranslationModule