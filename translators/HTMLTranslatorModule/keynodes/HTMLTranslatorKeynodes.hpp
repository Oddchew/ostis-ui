/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include <sc-memory/sc_keynodes.hpp>

namespace htmlTranslationModule
{
class HTMLTranslatorKeynodes : public ScKeynodes
{
  public:
  static inline ScKeynode const action_translate_sc_node_to_html{"action_translate_sc_node_to_html"};

  static inline ScKeynode const nrel_decomposition{"nrel_decomposition", ScType::ConstNodeNonRole};

  static inline ScKeynode const concept_user_interface_component{"concept_user_interface_component", ScType::ConstNodeClass};

  static inline ScKeynode const nrel_html_template{"nrel_html_template", ScType::ConstNodeNonRole};

  static inline ScKeynode const nrel_html_representation{"nrel_html_representation", ScType::ConstNodeNonRole};

  static inline ScKeynode const nrel_mimetype{"nrel_mimetype", ScType::ConstNodeNonRole};

  static inline ScKeynode const action_evaluate_specified_string_template{"action_evaluate_specified_string_template"};

  static inline ScKeynode const format_html{"format_html", ScType::ConstNodeClass};

  static inline ScKeynode const servable_content{"servable_content", ScType::ConstNodeClass};

  static inline ScKeynode const nrel_format{"nrel_format", ScType::ConstNodeClass};

  static inline ScKeynode const ostis_ui_current_ui_model{"ostis_ui_current_ui_model", ScType::ConstNode};
};

}  // namespace htmlTranslationModule
