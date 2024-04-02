#include "sc-agents-common/utils/AgentUtils.hpp"
#include "sc-agents-common/utils/CommonUtils.hpp"
#include "sc-agents-common/utils/GenerationUtils.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"

#include "SpecifiedStringTemplateKeynodes.hpp"
// should always be imported last since it imports a generated file
#include "SpecifiedStringTemplateAgent.hpp"

#include <inja.hpp>

using namespace utils;
namespace HTMLTranslatorModule {
SC_AGENT_IMPLEMENTATION(SpecifiedStringTemplateAgent) {
  ScAddr actionNode = m_memoryCtx.GetEdgeTarget(edgeAddr);

  if (!checkActionClass(actionNode)) {
    return SC_RESULT_OK;
  }
  SC_LOG_DEBUG("SpecifiedStringTemplateAgent started");

  // get template structure
  ScAddr templateNode = IteratorUtils::getAnyByOutRelation(
      &m_memoryCtx, actionNode, scAgentsCommon::CoreKeynodes::rrel_1);

  if (!templateNode.IsValid()) {
    SC_LOG_ERROR("Action doesn't have a template node.");
    SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
    AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }

  // get template output format
  ScAddr outputClassNode = IteratorUtils::getAnyByOutRelation(
      &m_memoryCtx, actionNode, scAgentsCommon::CoreKeynodes::rrel_2);

  if (!outputClassNode.IsValid()) {
    SC_LOG_ERROR("Action doesn't have an output class node.");
    SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
    AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }

  // get template string
  std::string templateString;
  bool templateStringLinkExists =
      m_memoryCtx.GetLinkContent(templateNode, templateString);
  
  if (!templateStringLinkExists) {
    SC_LOG_ERROR("Action doesn't have a link with a text template.");
    SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
    AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }

  ScAddr variablesNode = IteratorUtils::getAnyByOutRelation(
      &m_memoryCtx, templateNode, SpecifiedStringTemplateKeynodes::nrel_template_variables);
  
  if (!variablesNode.IsValid()) {
    SC_LOG_ERROR("Action doesn't have a variables node.");
    SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
    AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }

  // convert variablesNode to the map of variables

  std::string result =
      evaluateTemplate(m_memoryCtx, templateString, variablesNode);
  
  ScAddr resultLink = m_memoryCtx.CreateLink();
  if (!resultLink.IsValid()) {
    SC_LOG_ERROR("Wasn't able to generate a response link.");
    SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
    AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR;
  }
  
  m_memoryCtx.SetLinkContent(resultLink, result);
  m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm, outputClassNode,
                           resultLink);

  const std::vector res = {resultLink};
  AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, res, true);
  SC_LOG_DEBUG("SpecifiedStringTemplateAgent finished");
  return SC_RESULT_OK;
}

bool SpecifiedStringTemplateAgent::checkActionClass(const ScAddr & actionNode) {
  return SpecifiedStringTemplateKeynodes::action_evaluate_specified_string_template == actionNode;
}

std::string SpecifiedStringTemplateAgent::evaluateTemplate(
    const ScMemoryContext & m_memoryCtx,
    const std::string & templateString,
    const ScAddr & variablesNode) {
  return "TODO";
}

} // namespace HTMLTranslatorModule