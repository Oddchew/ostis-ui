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

using namespace utils;

namespace htmlTranslationModule {

StringScAddrMap
ParameterRetriever::GetNestedUIComponents(ScAgentContext &context,
                                          ScAddr const &uiComponent,
                                          std::string const &templateString) {
  StringScAddrMap nestedComponents;

  StringScAddrMap parameterClasses =
      ExtractParameterClasses(context, templateString);

  for (auto const &[name, node] : parameterClasses) {

    std::string const parameterAlias = "_parameter_" + name;
    ScTemplate parameterTemplate;
    ScAddr parameter;

    //
    // parameter_class - - - -> parameter
    //
    parameterTemplate.Triple(node, ScType::VarPermPosArc,
                             ScType::VarNode >> parameterAlias);

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
    parameterTemplate.Quintuple(uiComponent, ScType::VarCommonArc,
                                parameterAlias, ScType::VarPermPosArc,
                                HTMLTranslatorKeynodes::nrel_parameter);

    context.SearchByTemplateInterruptibly(
        parameterTemplate,
        [&parameterAlias, &parameter](
            ScTemplateSearchResultItem const &item) -> ScTemplateSearchRequest {
          item.Get(parameterAlias, parameter);
          return ScTemplateSearchRequest::STOP;
        });

    if (!context.IsElement(parameter)) {
      SC_LOG_ERROR("ParameterRetriever: given parameter is invalid.");
      throw utils::ScException(utils::ExceptionInvalidParams(
          "ParameterRetriever: given parameter is invalid.", ""));
    }

    nestedComponents[name] = parameter;
  }

  return nestedComponents;
}

/*!
 * @brief Extracts classes of parameters found in html template.
 * @param templateString String that contains html template.
 * @return Vector of classes.
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
 * In this case these class nodes are: concept_html_head, concept_html_body
 *
 * concept_html_head, concept_html_body are so called "parameter classes".
 */
StringScAddrMap
ParameterRetriever::ExtractParameterClasses(ScAgentContext &context,
                                            std::string const &templateString) {

  StringScAddrMap extractedNames;
  std::string parameterClassName = "";
  unsigned int collectMode = 0;

  // Assuming there are no nested brackets like: {...{...{}}}
  for (int i = 0; i < templateString.size(); i++) {
    if (templateString[i] == '}') {

      ScAddr parameterClass =
          context.SearchElementBySystemIdentifier(parameterClassName);

      if (!context.IsElement(parameterClass)) {
        SC_LOG_ERROR("ParameterRetriever: given parameter class is invalid.");
        throw utils::ScException(utils::ExceptionInvalidParams(
            "ParameterRetriever: given parameter class is invalid.",
            "Class with the name provided as a parameter in the html template "
            "is invalid."));
      }

      extractedNames[parameterClassName] = parameterClass;
      parameterClassName.clear();
      collectMode--;
    }

    if (collectMode == 1) {
      parameterClassName.push_back(templateString[i]);
    }

    if (templateString[i] == '{') {
      collectMode++;

      if (collectMode > 1) {
        SC_LOG_ERROR("ParameterRetriever: given html template is invalid.");
        throw utils::ScException(utils::ExceptionInvalidParams(
            "ParameterRetriever: given html template is invalid.",
            "Multiple nested brackets {...{...}} were given."));
      }
    }
  }

  return extractedNames;
}
} // namespace htmlTranslationModule