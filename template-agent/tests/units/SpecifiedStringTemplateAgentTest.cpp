/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "renderer/StringTemplateRenderer.hpp"

#include "sc_test.hpp"
#include "scs_loader.hpp"

#include "keynodes/SpecifiedStringTemplateKeynodes.hpp"
#include "sc-agents-common/keynodes/coreKeynodes.hpp"

using namespace specifiedStringTemplateModule;

namespace templateAgentTest
{
std::string const TEST_FILES_DIR_PATH = TEMPLATE_AGENT_TEST_SRC_PATH "/test-structures/";

using TemplateAgentTest = ScMemoryTest;

TEST_F(TemplateAgentTest, ZeroVariavlesStringTemplate)
{
  ScMemoryContext & context = *m_ctx;
  scAgentsCommon::CoreKeynodes::InitGlobal();
  SpecifiedStringTemplateKeynodes::InitGlobal();
  ScsLoader loader;
  loader.loadScsFile(context, TEST_FILES_DIR_PATH + "zero_variables_string_template.scs");

  ScAddr string_template = context.HelperFindBySystemIdtf("string_template");
  ScAddr replacement_values = context.HelperFindBySystemIdtf("replacement_values");
  std::string result = specifiedStringTemplateModule::StringTemplateRenderer::RenderStringTemplate(context, string_template, replacement_values);

  std::string expectedResult;
  ScAddr string_template_expected_result = context.HelperFindBySystemIdtf("string_template_expected_result");
  context.GetLinkContent(string_template_expected_result, expectedResult);

  EXPECT_EQ(result, expectedResult);
}

TEST_F(TemplateAgentTest, OneVariavleStringTemplate)
{
  ScMemoryContext & context = *m_ctx;
  scAgentsCommon::CoreKeynodes::InitGlobal();
  SpecifiedStringTemplateKeynodes::InitGlobal();
  ScsLoader loader;
  loader.loadScsFile(context, TEST_FILES_DIR_PATH + "one_variable_string_template.scs");

  ScAddr string_template = context.HelperFindBySystemIdtf("string_template");
  ScAddr replacement_values = context.HelperFindBySystemIdtf("replacement_values");
  std::string result = specifiedStringTemplateModule::StringTemplateRenderer::RenderStringTemplate(context, string_template, replacement_values);

  std::string expectedResult;
  ScAddr string_template_expected_result = context.HelperFindBySystemIdtf("string_template_expected_result");
  context.GetLinkContent(string_template_expected_result, expectedResult);

  EXPECT_EQ(result, expectedResult);
}

TEST_F(TemplateAgentTest, TwoVariavlesStringTemplate)
{
  ScMemoryContext & context = *m_ctx;
  scAgentsCommon::CoreKeynodes::InitGlobal();
  SpecifiedStringTemplateKeynodes::InitGlobal();
  ScsLoader loader;
  loader.loadScsFile(context, TEST_FILES_DIR_PATH + "two_variables_string_template.scs");

  ScAddr string_template = context.HelperFindBySystemIdtf("string_template");
  ScAddr replacement_values = context.HelperFindBySystemIdtf("replacement_values");
  std::string result = specifiedStringTemplateModule::StringTemplateRenderer::RenderStringTemplate(context, string_template, replacement_values);

  std::string expectedResult;
  ScAddr string_template_expected_result = context.HelperFindBySystemIdtf("string_template_expected_result");
  context.GetLinkContent(string_template_expected_result, expectedResult);

  EXPECT_EQ(result, expectedResult);
}

TEST_F(TemplateAgentTest, ThreeVariavlesStringTemplate)
{
  ScMemoryContext & context = *m_ctx;
  scAgentsCommon::CoreKeynodes::InitGlobal();
  SpecifiedStringTemplateKeynodes::InitGlobal();
  ScsLoader loader;
  loader.loadScsFile(context, TEST_FILES_DIR_PATH + "three_variables_string_template.scs");

  ScAddr string_template = context.HelperFindBySystemIdtf("string_template");
  ScAddr replacement_values = context.HelperFindBySystemIdtf("replacement_values");
  std::string result = specifiedStringTemplateModule::StringTemplateRenderer::RenderStringTemplate(context, string_template, replacement_values);

  std::string expectedResult;
  ScAddr string_template_expected_result = context.HelperFindBySystemIdtf("string_template_expected_result");
  context.GetLinkContent(string_template_expected_result, expectedResult);

  EXPECT_EQ(result, expectedResult);
}

} // namespace templateAgentTest
