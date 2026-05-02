#include <sc-memory/sc_action.hpp>
#include <sc-memory/sc_result.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>

#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "VisualAdaptationAgent.hpp"
#include "parameter-retriever/ParameterRetriever.hpp"

#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>        // ← обязательно для std::floor

namespace htmlTranslationModule
{

// ===================================================================
static bool IsScalableProperty(const std::string& id)
{
    static const std::vector<std::string> prefixes = {
        "fz", "font-size", "width", "height", "margin", "padding", 
        "line-height", "border-radius"
    };
    for (const auto& p : prefixes)
        if (id.find(p) == 0) return true;
    return false;
}

// ===================================================================
static std::string ScaleCssValue(const std::string& rawValue, double multiplier)
{
    if (std::abs(multiplier - 1.0) < 0.001) 
        return rawValue;

    std::string v = rawValue;

    // Максимальная очистка
    v.erase(std::remove_if(v.begin(), v.end(), [](unsigned char c){
        return std::isspace(c) || c == '"' || c == '\'' || c == ';';
    }), v.end());

    if (v.size() >= 6 && v.compare(0, 6, "float:") == 0) {
        v = v.substr(6);
    }

    if (v.empty() || v == "auto" || v == "none" || v[0] == '#' || 
        v.find("rgb") == 0 || v.find("hsl") == 0) {
        return rawValue;
    }

    std::stringstream ss(v);
    double num = 0.0;
    std::string unit;

    if (!(ss >> num)) return rawValue;

    ss >> unit;

    double newValue = num * multiplier;

    std::ostringstream oss;
    if (std::floor(newValue) == newValue) {
        oss << static_cast<long long>(newValue);
    } else {
        oss << std::fixed << std::setprecision(2) << newValue;
    }
    oss << unit;

    return oss.str();
}

// ===================================================================
static void InvalidateHTMLCache(ScAgentContext& ctx, ScAddr component)
{
    // Удаляем кэш текущего компонента
    ScAddr cached = utils::IteratorUtils::getAnyByOutRelation(
        &ctx, component, HTMLTranslatorKeynodes::nrel_html_representation);
    if (cached.IsValid())
        ctx.EraseElement(cached);

    // Поднимаемся к родителям
    ScIterator3Ptr it = ctx.CreateIterator3(ScType::Unknown, ScType::ConstPermPosArc, component);
    while (it->Next())
    {
        ScAddr parent = it->Get(0);
        ScAddr arc = it->Get(1);

        if (ctx.CheckConnector(HTMLTranslatorKeynodes::nrel_inclusion, arc, ScType::ConstPermPosArc))
        {
            InvalidateHTMLCache(ctx, parent);
        }
    }
}

// ===================================================================
ScResult VisualAdaptationAgent::DoProgram(ScActionInitiatedEvent const & event, ScAction & action)
{
    auto const [component, multiplierLink] = action.GetArguments<2>();
    if (!component.IsValid()) {
        SC_LOG_ERROR("VisualAdaptationAgent: component is invalid.");
        return action.FinishUnsuccessfully();
    }

    double multiplier = 1.0;
    if (multiplierLink.IsValid()) {
        std::string s;
        m_context.GetLinkContent(multiplierLink, s);
        try { multiplier = std::stod(s); } catch (...) {}
    }

    SC_LOG_INFO("VisualAdaptation: Applying multiplier " + std::to_string(multiplier));

    auto parameters = ParameterRetriever::GetNestedUIComponents(m_context, component);

    for (auto const& [id, paramAddr] : parameters)
    {
        if (!IsScalableProperty(id)) continue;

        ScAddr valueLink = utils::IteratorUtils::getAnyByOutRelation(
            &m_context, paramAddr, HTMLTranslatorKeynodes::nrel_html_representation);

        if (!valueLink.IsValid() || !m_context.GetElementType(valueLink).IsLink())
            continue;

        std::string currentValue;
        m_context.GetLinkContent(valueLink, currentValue);

        std::string newValue = ScaleCssValue(currentValue, multiplier);

        if (newValue != currentValue) {
            m_context.SetLinkContent(valueLink, newValue);
            SC_LOG_INFO("VisualAdaptation: " + id + " : " + currentValue + " → " + newValue);
        }
    }

    InvalidateHTMLCache(m_context, component);
    SC_LOG_INFO("VisualAdaptation: cache invalidated");

    return action.FinishSuccessfully();
}

ScAddr VisualAdaptationAgent::GetActionClass() const
{
    return HTMLTranslatorKeynodes::action_apply_visual_adaptation;
}

} // namespace htmlTranslationModule