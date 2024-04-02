#pragma once

#include "sc-memory/sc_addr.hpp"
#include "sc-memory/sc_object.hpp"

#include "SpecifiedStringTemplateKeynodes.generated.hpp"

namespace HTMLTranslatorModule
{
class SpecifiedStringTemplateKeynodes : public ScObject
{
  SC_CLASS()
  SC_GENERATED_BODY()

public:
  SC_PROPERTY(Keynode("action_evaluate_specified_string_template"), ForceCreate)
  static ScAddr action_evaluate_specified_string_template;

  SC_PROPERTY(Keynode("nrel_template_variables"), ForceCreate)
  static ScAddr nrel_template_variables;
};

}  // namespace HTMLTranslatorModule