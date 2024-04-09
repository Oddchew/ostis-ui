/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include "sc-memory/sc_addr.hpp"
#include "sc-memory/sc_object.hpp"

#include "SpecifiedStringTemplateKeynodes.generated.hpp"

namespace specifiedStringTemplateModule
{
class SpecifiedStringTemplateKeynodes : public ScObject
{
  SC_CLASS()
  SC_GENERATED_BODY()

public:
  SC_PROPERTY(Keynode("action_evaluate_specified_string_template"), ForceCreate)
  static ScAddr action_evaluate_specified_string_template;

  SC_PROPERTY(Keynode("nrel_variable_template"), ForceCreate(ScType::NodeConstNoRole))
  static ScAddr nrel_variable_template;

  SC_PROPERTY(Keynode("nrel_format"), ForceCreate(ScType::NodeConstNoRole))
  static ScAddr nrel_format;

  SC_PROPERTY(Keynode("rrel_variable"), ForceCreate(ScType::NodeConstRole))
  static ScAddr rrel_variable;

  SC_PROPERTY(Keynode("rrel_value"), ForceCreate(ScType::NodeConstRole))
  static ScAddr rrel_value;

  SC_PROPERTY(Keynode("rrel_template"), ForceCreate(ScType::NodeConstRole))
  static ScAddr rrel_template;

  SC_PROPERTY(Keynode("nrel_replacements_variables"), ForceCreate(ScType::NodeConstNoRole))
  static ScAddr nrel_replacements_variables;
};

}  // namespace specifiedStringTemplateModule
