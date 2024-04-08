/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "sc_test.hpp"
#include "scs_loader.hpp"

#include "sc-agents-common/keynodes/coreKeynodes.hpp"
#include "sc-agents-common/utils/AgentUtils.hpp"
#include "sc-agents-common/utils/IteratorUtils.hpp"
#include "sc-memory/kpm/sc_agent.hpp"

#include "keynodes/SpecifiedStringTemplateKeynodes.hpp"
#include "agent/SpecifiedStringTemplateAgent.hpp"

using namespace specifiedStringTemplateModule;
using namespace scAgentsCommon;

namespace templateAgentTest
{
std::string const TEST_FILES_DIR_PATH = TEMPLATE_AGENT_TEST_SRC_PATH "/test-structures/";
int const WAIT_TIME = 1000;
using TemplateAgentTest = ScMemoryTest;

void InitializeTest()
{
  CoreKeynodes::InitGlobal();
  SpecifiedStringTemplateKeynodes::InitGlobal();

  ScAgentInit(true);
  SC_AGENT_REGISTER(SpecifiedStringTemplateAgent);
}

void ShutdownTest()
{
  SC_AGENT_UNREGISTER(SpecifiedStringTemplateAgent);
}

TEST_F(TemplateAgentTest, ZeroVariavlesStringTemplate)
{
  InitializeTest();

  ScMemoryContext & context = *m_ctx;
  ScsLoader loader;
  loader.loadScsFile(context, TEST_FILES_DIR_PATH + "zero_variables_string_template.scs");

  // Call the agent, get and validate result
  ScAddr test_action_node = context.HelperFindBySystemIdtf("test_action_node");
  EXPECT_TRUE(context.IsElement(test_action_node));
  ScAddr result = utils::AgentUtils::applyActionAndGetResultIfExists(&context, test_action_node, WAIT_TIME);
  EXPECT_TRUE(context.IsElement(result));
  ScAddr resultLink = utils::IteratorUtils::getAnyFromSet(&context, result);
  EXPECT_TRUE(context.IsElement(resultLink));

  std::string resultLinkContent;
  context.GetLinkContent(resultLink, resultLinkContent);
  EXPECT_NE(resultLinkContent, "");

  // Check if the result is correct
  ScAddr string_template_expected_result = context.HelperFindBySystemIdtf("string_template_expected_result");
  std::string string_template_expected_result_content;
  context.GetLinkContent(string_template_expected_result, string_template_expected_result_content);
  EXPECT_EQ(string_template_expected_result_content, resultLinkContent);

  // Check if nrel_format is generated for the result sc-link
  ScIterator5Ptr formatIterator = context.Iterator5(
        resultLink,
        ScType::EdgeDCommonConst,
        context.HelperFindBySystemIdtf("format_html"),
        ScType::EdgeAccessConstPosPerm,
        SpecifiedStringTemplateKeynodes::nrel_format);
  EXPECT_TRUE(formatIterator->Next());

  ShutdownTest();
}


TEST_F(TemplateAgentTest, OneVariavleStringTemplate)
{
  InitializeTest();

  ScMemoryContext & context = *m_ctx;
  ScsLoader loader;
  loader.loadScsFile(context, TEST_FILES_DIR_PATH + "one_variable_string_template.scs");

  // Call the agent, get and validate result
  ScAddr test_action_node = context.HelperFindBySystemIdtf("test_action_node");
  EXPECT_TRUE(context.IsElement(test_action_node));
  ScAddr result = utils::AgentUtils::applyActionAndGetResultIfExists(&context, test_action_node, WAIT_TIME);
  EXPECT_TRUE(context.IsElement(result));
  ScAddr resultLink = utils::IteratorUtils::getAnyFromSet(&context, result);
  EXPECT_TRUE(context.IsElement(resultLink));

  // Check if the result is correct
  std::string resultLinkContent;
  context.GetLinkContent(resultLink, resultLinkContent);
  EXPECT_NE(resultLinkContent, "");

  ScAddr string_template_expected_result = context.HelperFindBySystemIdtf("string_template_expected_result");
  std::string string_template_expected_result_content;
  context.GetLinkContent(string_template_expected_result, string_template_expected_result_content);

  // Check if nrel_format is generated for the result sc-link
  EXPECT_EQ(string_template_expected_result_content, resultLinkContent);
  ScIterator5Ptr formatIterator = context.Iterator5(
        resultLink,
        ScType::EdgeDCommonConst,
        context.HelperFindBySystemIdtf("format_html"),
        ScType::EdgeAccessConstPosPerm,
        SpecifiedStringTemplateKeynodes::nrel_format);
  EXPECT_TRUE(formatIterator->Next());

  ShutdownTest();
}


} // namespace templateAgentTest
