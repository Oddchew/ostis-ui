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

#include "html-translator/HTMLTranslator.hpp"

#include "utils.hpp"


using namespace htmlTranslationModule;

namespace htmlTranslatorTest
{
ScsLoader loader;
std::string const TEST_FILES_DIR_PATH = "../test-structures/";
std::string const TEST_KB_DIR_PATH = "../test-structures/kb";

using HTMLTranslatorTest = ScMemoryTest;

void TestHTMLTranslator(ScAgentContext & context, std::string const & scsTestFileName)
{
  context.SubscribeAgent<HTMLTranslatorAgent>();


  loader.loadScsFile(context, TEST_FILES_DIR_PATH + scsTestFileName);
  loadKB(context, loader, TEST_KB_DIR_PATH);

  ScAddr actionNode = context.SearchElementBySystemIdentifier("test_action_node");
  ScAddr rootUiElement = utils::IteratorUtils::getAnyByOutRelation(&context, actionNode, ScKeynodes::rrel_1);

  EXPECT_TRUE(context.IsElement(rootUiElement));
  ScAddr resultLink;
  resultLink = HTMLTranslator::TranslateScToHTML(context, rootUiElement);
  std::string resultLinkContent;
  context.GetLinkContent(resultLink, resultLinkContent);
  EXPECT_NE(resultLinkContent, "");

  // Check if the result is correct
  ScAddr stringTemplateExpectedResult = utils::IteratorUtils::getAnyByOutRelation(&context, actionNode, context.ResolveElementSystemIdentifier("rrel_expected_result"));
  std::string stringTemplateExpectedResultContent;
  context.GetLinkContent(stringTemplateExpectedResult, stringTemplateExpectedResultContent);
  EXPECT_EQ(stringTemplateExpectedResultContent, resultLinkContent);

  context.UnsubscribeAgent<HTMLTranslatorAgent>();
}

TEST_F(HTMLTranslatorTest, TranslateTextOutput)
{
  TestHTMLTranslator(*m_ctx, "test_translate_text_output.scs");
}

TEST_F(HTMLTranslatorTest, TranslateDecompositionButton)
{
  TestHTMLTranslator(*m_ctx, "test_translate_button_decomposition.scs");
}
} // namespace htmlTranslatorTest
