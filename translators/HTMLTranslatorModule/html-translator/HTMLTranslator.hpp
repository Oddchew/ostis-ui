/*
 * This source file is part of an OSTIS project. For the latest info, see
 * http://ostis.net Distributed under the MIT License (See accompanying file
 * COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_memory.hpp>
#include <string>

namespace htmlTranslationModule {

class HTMLTranslator {
public:
  static ScAddr TranslateScToHTML(ScAgentContext &context,
                                  ScAddr const &rootUiElement);
  static ScAddr GetUIComponentHTMLTemplate(ScAgentContext &context,
                                           ScAddr const &uiComponent);
  static ScAddr GetAnswerLink(ScAgentContext &context, ScAddr const &uiElement,
                              ScAddr const &uiHTMLTemplateLink);
  static void InsertParameterValue(std::string &templateString,
                                   const std::string &parameterName,
                                   const std::string &parameterValue);
};

} // namespace htmlTranslationModule
