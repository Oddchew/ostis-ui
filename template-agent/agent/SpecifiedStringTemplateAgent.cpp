/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "sc-agents-common/utils/AgentUtils.hpp"
#include "sc-agents-common/utils/GenerationUtils.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"

#include "keynodes/SpecifiedStringTemplateKeynodes.hpp"
#include "renderer/StringTemplateRenderer.hpp"

#include "SpecifiedStringTemplateAgent.hpp"

using namespace utils;

namespace specifiedStringTemplateModule
{
SC_AGENT_IMPLEMENTATION(SpecifiedStringTemplateAgent)
{
  ScAddr const actionNode = m_memoryCtx.GetEdgeTarget(edgeAddr);

  if (!CheckActionClass(m_memoryCtx, actionNode))
  {
    return SC_RESULT_OK;
  }
  SC_LOG_DEBUG("SpecifiedStringTemplateAgent started");

  // Get string template sc-link
  ScAddr const stringTemplateLink = IteratorUtils::getAnyByOutRelation(
      &m_memoryCtx, actionNode, scAgentsCommon::CoreKeynodes::rrel_1);

  if (!m_memoryCtx.IsElement(stringTemplateLink))
  {
    SC_LOG_ERROR("Action doesn't have a string template link.");
    SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
    AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }

  // Get template link replacement constants (e.g. user interface component address)
  ScAddr const stringTemplateLinkReplacements = IteratorUtils::getAnyByOutRelation(
        &m_memoryCtx, actionNode, scAgentsCommon::CoreKeynodes::rrel_2);

  // Get output sc-link format
  ScAddr const generatedLinkFormatClass = IteratorUtils::getAnyByOutRelation(
      &m_memoryCtx, actionNode, scAgentsCommon::CoreKeynodes::rrel_3);
  if (!m_memoryCtx.IsElement(generatedLinkFormatClass))
  {
    SC_LOG_ERROR("Action doesn't have a format.");
    SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
    AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }

  std::string result;
  try
  {
    result = StringTemplateRenderer::RenderStringTemplate(m_memoryCtx, stringTemplateLink, stringTemplateLinkReplacements, generatedLinkFormatClass);
  }
  catch (ScException const & exception)
  {
    SC_LOG_ERROR(exception.Description());
    AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
    return SC_RESULT_ERROR;
  }

  // Create result sc-link and set result text as a link content
  ScAddr const resultLink = m_memoryCtx.CreateLink();
  m_memoryCtx.SetLinkContent(resultLink, result);
  // Generate nrel_fromat to link if format is passed to the action parameter
  if (generatedLinkFormatClass.IsValid())
  {
    GenerationUtils::generateRelationBetween(&m_memoryCtx, resultLink, generatedLinkFormatClass, SpecifiedStringTemplateKeynodes::nrel_format);
  }

  // Create agent answer and finish agent work
  ScAddrVector const agentResult = {resultLink};
  AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, agentResult, true);
  SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
  return SC_RESULT_OK;
}

bool SpecifiedStringTemplateAgent::CheckActionClass(ScMemoryContext & context, ScAddr const & actionNode)
{
  return context.HelperCheckEdge(SpecifiedStringTemplateKeynodes::action_evaluate_specified_string_template, actionNode, ScType::EdgeAccessConstPosPerm);
}

} // namespace specifiedStringTemplateModule
