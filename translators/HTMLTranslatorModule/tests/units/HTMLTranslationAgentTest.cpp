/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include <sc-memory/test/sc_test.hpp>
#include <sc-builder/scs_loader.hpp>

#include <sc-memory/sc_agent.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>

#include "agents/HTMLTranslatorAgent.hpp"
#include "keynodes/HTMLTranslatorKeynodes.hpp"

#include "agent/SpecifiedStringTemplateAgent.hpp"
#include "keynodes/SpecifiedStringTemplateKeynodes.hpp"

#include "utils.hpp"

using namespace specifiedStringTemplateModule;
using namespace htmlTranslationModule;
namespace htmlTranslatorAgentTest
{
ScsLoader loader;
std::string const TEST_FILES_DIR_PATH = "../test-structures/";
std::string const TEST_KB_DIR_PATH = "../test-structures/kb";
size_t const WAIT_TIME = 1000;
using HTMLTranslatorAgentTest = ScMemoryTest;

void TestHTMLTranslatorAgent(ScAgentContext & context, std::string const & scsTestFileName)
{
  context.SubscribeAgent<HTMLTranslatorAgent>();
  context.SubscribeAgent<SpecifiedStringTemplateAgent>();
  
  loader.loadScsFile(context, TEST_FILES_DIR_PATH + scsTestFileName);
  loadKB(context, loader, TEST_KB_DIR_PATH);

  // Call the agent, get and validate result
  ScAddr actionNode = context.SearchElementBySystemIdentifier("test_action_node");
  EXPECT_TRUE(context.IsElement(actionNode));
  ScAddr rootUiElement = utils::IteratorUtils::getAnyByOutRelation(&context, actionNode, ScKeynodes::rrel_1);
  EXPECT_TRUE(context.IsElement(rootUiElement));
  ScAction action = context.GenerateAction(HTMLTranslatorKeynodes::action_translate_sc_node_to_html);
  action.SetArguments(rootUiElement);
  action.InitiateAndWait();
  ScStructure result = action.GetResult();
  EXPECT_TRUE(context.IsElement(result));
  ScAddr resultLink = utils::IteratorUtils::getAnyFromSet(&context, result);
  EXPECT_TRUE(context.IsElement(resultLink));

  std::string resultLinkContent;
  context.GetLinkContent(resultLink, resultLinkContent);
  EXPECT_NE(resultLinkContent, "");

  // Check if the result is correct
  ScAddr stringTemplateExpectedResult = utils::IteratorUtils::getAnyByOutRelation(&context, actionNode, context.ResolveElementSystemIdentifier("rrel_expected_result"));
  std::string stringTemplateExpectedResultContent;
  context.GetLinkContent(stringTemplateExpectedResult, stringTemplateExpectedResultContent);
  EXPECT_EQ(stringTemplateExpectedResultContent, resultLinkContent);

  context.UnsubscribeAgent<HTMLTranslatorAgent>();
  context.UnsubscribeAgent<SpecifiedStringTemplateAgent>();
}

TEST_F(HTMLTranslatorAgentTest, TranslateTextOutput)
{
  TestHTMLTranslatorAgent(*m_ctx, "test_translate_text_output.scs");
}

TEST_F(HTMLTranslatorAgentTest, TranslateButtonDecomposition)
{
  TestHTMLTranslatorAgent(*m_ctx, "test_translate_button_decomposition.scs");
}

TEST_F(HTMLTranslatorAgentTest, TranslateDemoUI)
{
  TestHTMLTranslatorAgent(*m_ctx, "test_translate_demo_ui.scs");
}

} // namespace htmlTranslatorAgentTest
