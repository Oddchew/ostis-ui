#pragma once

#include <map>

#include "sc-memory/kpm/sc_agent.hpp"
/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "sc-agents-common/keynodes/coreKeynodes.hpp"
#include "generated/SpecifiedStringTemplateAgent.generated.hpp"
#include "sc-memory/sc_memory.hpp"

namespace specifiedStringTemplateModule
{

class SpecifiedStringTemplateAgent : public ScAgent
{
  SC_CLASS(Agent, Event(scAgentsCommon::CoreKeynodes::question_initiated, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()

private:
  static bool CheckActionClass(ScMemoryContext & context, ScAddr const & actionNode);

//   std::map<VariableType, std::vector<std::string>> getTemplateVariables(std::string const & text);

//   std::string findResultText(
//       ScAddr const & templateNode,
//       ScAddr const & parametersNode,
//       std::map<VariableType, std::vector<std::string>> const & variables,
//       std::string const & text);

//   std::vector<ScTemplateParams> findParametersList(const ScAddr & templateNode, const ScAddr & parametersNode);

//   std::vector<std::string> getTemplateLinksVariables(std::string const & text);

//   std::vector<std::string> getTemplateSetElementsVariables(std::string const & text);

//   std::string processScTemplate(
//       ScAddr const & templateNode,
//       ScTemplateParams const & parameters,
//       std::map<VariableType, std::vector<std::string>> const & variables,
//       std::string const & text);

//   void replaceLinksVariables(
//         ScTemplateSearchResultItem const & phraseSemanticResult,
//         std::vector<std::string> const & variables,
//         std::string & text);

//   void replaceSetElementsVariables(
//         ScTemplateSearchResultItem const & phraseSemanticResult,
//         std::vector<std::string> const & variables,
//         std::string & text);

//   void generateSemanticEquivalent(const ScAddr & replyMessageNode, const ScAddr & structure);

//   void updateSemanticAnswer(const ScTemplateSearchResultItem & phraseSemanticResult);

//   void updateSemanticAnswer(const ScAddr & phraseAddr);

//   void addToRemoveNodes(const ScAddr & structNode, const ScAddr & conceptNode, ScAddrVector & vector);

//   ScAddrVector getIncidentElements(const ScAddr & node, const ScAddr & structNode);
};
}  // namespace specifiedStringTemplateModule