#pragma once

#include <sc-memory/sc_agent.hpp>

namespace htmlTranslationModule
{
class VisualAdaptationAgent : public ScActionAgent
{
public:
    ScResult DoProgram(ScActionInitiatedEvent const & event, ScAction & action) override;
    ScAddr GetActionClass() const override;
};
}  // namespace htmlTranslationModule
