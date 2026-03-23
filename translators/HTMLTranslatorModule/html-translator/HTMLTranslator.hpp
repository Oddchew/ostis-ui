/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include <sc-memory/sc_agent.hpp>
#include <sc-memory/sc_memory.hpp>

namespace htmlTranslationModule
{

class HTMLTranslator
{
public:
  static ScAddr TranslateScToHTML(ScAgentContext & context, ScAddr const & rootUiElement);
  static ScAddr GetUIComponentHTMLTemplate(ScAgentContext & context, ScAddr const & uiComponent);
  static ScAddr GetTemplateAgentAnswerLink(ScAgentContext & context, ScAddr const & uiElement, ScAddr const & uiHTMLTemplateLink);
};

} // namespace htmlTranslationModule
