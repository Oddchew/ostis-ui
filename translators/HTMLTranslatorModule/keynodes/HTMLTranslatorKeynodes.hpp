/*
 * This source file is part of an OSTIS project. For the latest info, see
 * http://ostis.net Distributed under the MIT License (See accompanying file
 * COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include <sc-memory/sc_keynodes.hpp>
#include <sc-memory/sc_type.hpp>

namespace htmlTranslationModule
{
class HTMLTranslatorKeynodes : public ScKeynodes
{
public:
  static inline ScKeynode const action_translate_sc_to_html{"action_translate_sc_to_html", ScType::ConstNodeClass};

  static inline ScKeynode const concept_ui_object{"concept_ui_object", ScType::ConstNodeClass};

  static inline ScKeynode const concept_current_ostis_ui_model{
      "concept_current_ostis_ui_model",
      ScType::ConstNodeClass};

  static inline ScKeynode const _root_ui_obj{"_root_ui_obj", ScType::VarNodeClass};

  static inline ScKeynode const nrel_html_template{"nrel_html_template", ScType::ConstNodeNonRole};

  static inline ScKeynode const nrel_html_parameter_id{"nrel_html_parameter_id", ScType::ConstNodeNonRole};

  static inline ScKeynode const nrel_html_representation{"nrel_html_representation", ScType::ConstNodeNonRole};

  static inline ScKeynode const nrel_sc_template{"nrel_sc_template", ScType::ConstNodeNonRole};

  static inline ScKeynode const nrel_inclusion{"nrel_inclusion", ScType::ConstNodeNonRole};

  static inline ScKeynode const rrel_specific_realisation{"rrel_specific_realisation", ScType::ConstNodeRole};
};

}  // namespace htmlTranslationModule
