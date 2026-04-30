#include <sc-memory/sc_action.hpp>
#include <sc-memory/sc_result.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "VisualAdaptationAgent.hpp"
#include "parameter-retriever/ParameterRetriever.hpp"
#include <iomanip>
#include <sstream>

namespace htmlTranslationModule
{

ScResult VisualAdaptationAgent::DoProgram(ScActionInitiatedEvent const & event, ScAction & action)
{
    // 1. Получаем компонент и multiplier
    auto const [component, multiplierLink] = action.GetArguments<2>();

    if (!component.IsValid())
    {
        SC_LOG_ERROR("VisualAdaptationAgent: component is invalid.");
        return action.FinishUnsuccessfully();
    }

    double multiplier = 1.0;
    if (multiplierLink.IsValid())
    {
        std::string multiplierStr;
        m_context.GetLinkContent(multiplierLink, multiplierStr);
        try {
            multiplier = std::stod(multiplierStr);
        } catch (...) {
            SC_LOG_WARNING("VisualAdaptationAgent: failed to parse multiplier '" + multiplierStr + "', using 1.0");
        }
    }

    SC_LOG_INFO("VisualAdaptation: Applying multiplier " + std::to_string(multiplier) + " to component.");

    // 2. Получаем все параметры компонента
    StringScAddrMap parameters = ParameterRetriever::GetNestedUIComponents(m_context, component);

    for (auto const & [id, paramAddr] : parameters)
    {
        // Проверяем, является ли параметр визуальным (fz, width, height, margin, padding)
        if (id.find("fz") == 0 || id.find("width") == 0 || id.find("height") == 0 ||
            id.find("margin") == 0 || id.find("padding") == 0)
        {
            // Находим ссылку с текущим значением
            ScAddr valueLink = utils::IteratorUtils::getAnyByOutRelation(&m_context, paramAddr, HTMLTranslatorKeynodes::nrel_html_representation);

            if (m_context.IsElement(valueLink) && m_context.GetElementType(valueLink).IsLink())
            {
                std::string currentValueStr;
                m_context.GetLinkContent(valueLink, currentValueStr);

                // Пробуем извлечь числовое значение и единицы измерения
                std::stringstream ss(currentValueStr);
                double value;
                std::string unit;
                if (ss >> value)
                {
                    ss >> unit; // px, %, em, etc.

                    double newValue = value * multiplier;

                    std::stringstream newSs;
                    newSs << std::fixed << std::setprecision(2) << newValue << unit;
                    std::string newValueStr = newSs.str();

                    // Удаляем лишние нули в конце
                    size_t lastZero = newValueStr.find_last_not_of('0');
                    if (lastZero != std::string::npos && newValueStr[lastZero] == '.') {
                        lastZero--;
                    }
                    if (lastZero != std::string::npos && lastZero < newValueStr.size() - 1) {
                         // Careful here, we need to keep the unit.
                         // Actually, let's just use a simpler formatting.
                    }

                    // Re-format to be cleaner
                    std::ostringstream oss;
                    oss << newValue << unit;
                    newValueStr = oss.str();

                    SC_LOG_INFO("VisualAdaptation: Changing parameter " + id + " from " + currentValueStr + " to " + newValueStr);
                    m_context.SetLinkContent(valueLink, newValueStr);
                }
            }
        }
    }

    return action.FinishSuccessfully();
}

ScAddr VisualAdaptationAgent::GetActionClass() const
{
    return HTMLTranslatorKeynodes::action_apply_visual_adaptation;
}

}  // namespace htmlTranslationModule
