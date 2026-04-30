/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include <string>

#include "sc-memory/sc_memory.hpp"

namespace specifiedStringTemplateModule
{

class StringTemplateRenderer
{
public:
  static std::string RenderStringTemplate(ScMemoryContext & context, ScAddr const & stringTemplateLink, ScAddr const & stringTemplateLinkReplacements);
  static ScTemplateParams GetScTemplateParamsFromTemplateReplacements(ScMemoryContext & context, ScAddr const & templateAddr, ScAddr const & stringTemplateLinkReplacements);
};

} // namespace specifiedStringTemplateModule
