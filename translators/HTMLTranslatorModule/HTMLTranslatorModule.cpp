/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "agents/HTMLTranslatorAgent.hpp"
#include "agents/UIComponentsReorderAgent.hpp"
#include "agents/AdaptationManagerAgent.hpp"
#include "agents/VisualAdaptationAgent.hpp"

#include "HTMLTranslatorModule.hpp"

using namespace htmlTranslationModule;

SC_MODULE_REGISTER(HTMLTranslatorModule)
    ->Agent<HTMLTranslatorAgent>()
    ->Agent<UIComponentsReorderAgent>()
    ->Agent<AdaptationManagerAgent>()
    ->Agent<VisualAdaptationAgent>()
    ;