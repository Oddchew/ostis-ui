/*
 * This source file is part of an OSTIS project. For the latest info, see
 * http://ostis.net Distributed under the MIT License (See accompanying file
 * COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include <sc-builder/scs_loader.hpp>
#include <sc-memory/test/sc_test.hpp>
#include <sc-memory/sc_agent.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>

#include "agents/HTMLTranslatorAgent.hpp"
#include "agents/UIComponentsReorderAgent.hpp"
#include "html-translator/HTMLTranslator.hpp"
#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "utils.hpp"

using namespace htmlTranslationModule;

namespace uiComponentsReorderTest
{

// ============================================================================
//  Константы и вспомогательные функции
// ============================================================================

ScsLoader loader;
std::string const TEST_FILES_DIR_PATH = "../test-structures/";
std::string const TEST_KB_DIR_PATH    = "../test-structures/kb";

// ============================================================================
//  Фикстур с гарантированной очисткой (ключевое исправление!)
// ============================================================================

/**
 * Базовый фикстур для тестов reorder.
 * Гарантирует отписку агентов в TearDown даже если тест упал по ASSERT.
 */
class ReorderTestFixture : public ScMemoryTest
{
protected:
  void SetUp() override
{
    try
    {
        ScMemoryTest::SetUp();
    }
    catch (const std::exception& e)
    {
        FAIL() << "ScMemoryTest::SetUp() failed: " << e.what();
    }
}

  void TearDown() override
  {
    // === ГЛАВНОЕ: гарантированная отписка агентов ===
    // Вызываем в блоках try-catch, чтобы ошибка отписки не ломала очистку
    if (m_ctx)
    {
      try { m_ctx->UnsubscribeAgent<HTMLTranslatorAgent>(); }
      catch (...) { /* Игнорируем, если агент не был подписан */ }

      try { m_ctx->UnsubscribeAgent<UIComponentsReorderAgent>(); }
      catch (...) { /* Игнорируем, если агент не был подписан */ }
    }

    // Базовая очистка контекста
    ScMemoryTest::TearDown();
  }
};

using ReorderTest = ReorderTestFixture;


// Создаёт SC-ссылку с заданным строковым содержимым.
static ScAddr MakeStringLink(ScAgentContext & ctx, std::string const & content)
{
  ScAddr link = ctx.GenerateLink();
  ctx.SetLinkContent(link, content);
  return link;
}

// Получает текст HTML-ссылки из структуры результата действия.
static std::string GetHTMLFromActionResult(ScAgentContext & ctx, ScAction & action)
{
  ScStructure result = action.GetResult();
  ScAddr link        = utils::IteratorUtils::getAnyFromSet(&ctx, result);
  if (!ctx.IsElement(link))
    return "";
  std::string html;
  ctx.GetLinkContent(link, html);
  return html;
}

// Запускает UIComponentsReorderAgent и возвращает HTML результата.
static std::string RunReorder(
    ScAgentContext & ctx,
    ScAddr const & parent,
    std::string const & idA,
    std::string const & idB)
{
  ScAddr linkA = MakeStringLink(ctx, idA);
  ScAddr linkB = MakeStringLink(ctx, idB);

  ScAction action = ctx.GenerateAction(HTMLTranslatorKeynodes::action_reorder_ui_components);
  action.SetArguments(parent, linkA, linkB);
  action.InitiateAndWait();
  return GetHTMLFromActionResult(ctx, action);
}

// Читает текущий nrel_html_parameter_id компонента.
static std::string GetComponentId(ScAgentContext & ctx, ScAddr const & component)
{
  ScAddr idLink =
      utils::IteratorUtils::getAnyByOutRelation(&ctx, component, HTMLTranslatorKeynodes::nrel_html_parameter_id);
  if (!ctx.IsElement(idLink))
    return "";
  std::string id;
  ctx.GetLinkContent(idLink, id);
  return id;
}

// Возвращает позицию подстроки в строке (удобно для сравнения порядка).
static std::size_t PosOf(std::string const & html, std::string const & token)
{
  return html.find(token);
}


// ============================================================================
//  Группа 1. UIComponentsReorderAgent — корректное поведение
// ============================================================================

// ----------------------------------------------------------------------------
// Тест 1.1. Базовый своп двух компонентов
// ----------------------------------------------------------------------------
/*TEST_F(ReorderTest, BasicSwapTwoComponents)
{
    m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
    m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

    // Загружаем только test_reorder_components.scs
    try {
        loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
        loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);
        SC_LOG_INFO("Test KB loaded successfully");
    } catch (const std::exception& e) {
        FAIL() << "Failed to load KB: " << e.what();
    }

    ScAddr parent = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
    ASSERT_TRUE(m_ctx->IsElement(parent)) << "test_parent_component not found in KB";

    // Простая проверка, что компонент существует
    ScAddr btn = m_ctx->SearchElementBySystemIdentifier("comp_button");
    ASSERT_TRUE(m_ctx->IsElement(btn));

    std::string initialId = GetComponentId(*m_ctx, btn);
    EXPECT_EQ(initialId, "slot1");

    SC_LOG_INFO("Basic test structure is OK");
    SUCCEED();   // пока просто проверяем загрузку
}*/

// ----------------------------------------------------------------------------
// Тест 1.2. Двойной своп возвращает исходное состояние
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, DoubleSwapRestoresOriginalOrder)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
  m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

  try
    {
        loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
        loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);
    }
    catch (const std::exception& e)
    {
        FAIL() << "Failed to load test KB: " << e.what();
    }

  ScAddr parent = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ASSERT_TRUE(m_ctx->IsElement(parent));

  ScAction initAction = m_ctx->GenerateAction(HTMLTranslatorKeynodes::action_translate_sc_to_html);
  initAction.SetArguments(parent);
  initAction.InitiateAndWait();
  std::string const initialHtml = GetHTMLFromActionResult(*m_ctx, initAction);
  ASSERT_FALSE(initialHtml.empty());

  RunReorder(*m_ctx, parent, "slot1", "slot2");
  std::string const restoredHtml = RunReorder(*m_ctx, parent, "slot1", "slot2");

  EXPECT_EQ(initialHtml, restoredHtml)
      << "HTML should be identical to the original after double swap";
}

// ----------------------------------------------------------------------------
// Тест 1.3. SC-память обновлена корректно после свапа
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, ScMemoryUpdatedAfterSwap)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
  m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

  loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
  loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);

  ScAddr parent    = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ScAddr compBtn   = m_ctx->SearchElementBySystemIdentifier("comp_button");
  ScAddr compInput = m_ctx->SearchElementBySystemIdentifier("comp_input");
  ASSERT_TRUE(m_ctx->IsElement(parent));
  ASSERT_TRUE(m_ctx->IsElement(compBtn));
  ASSERT_TRUE(m_ctx->IsElement(compInput));

  EXPECT_EQ(GetComponentId(*m_ctx, compBtn),   "slot1");
  EXPECT_EQ(GetComponentId(*m_ctx, compInput), "slot2");

  RunReorder(*m_ctx, parent, "slot1", "slot2");

  EXPECT_EQ(GetComponentId(*m_ctx, compBtn),   "slot2")
      << "comp_button should now have id='slot2'";
  EXPECT_EQ(GetComponentId(*m_ctx, compInput), "slot1")
      << "comp_input should now have id='slot1'";
}

// ----------------------------------------------------------------------------
// Тест 1.4. Своп крайних слотов в наборе из трёх — средний не трогается
// ----------------------------------------------------------------------------
/*TEST_F(ReorderTest, SwapFirstAndLastInThreeSlots_MiddleUnchanged)
{
    m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
    m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

    try
    {
        loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_three_slots.scs");
        loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);
    }
    catch (const std::exception& e)
    {
        FAIL() << "Failed to load test_reorder_three_slots.scs: " << e.what();
    }

    ScAddr parent = m_ctx->SearchElementBySystemIdentifier("test_three_slot_parent");
    ASSERT_TRUE(m_ctx->IsElement(parent)) << "test_three_slot_parent not found";

    std::string const reorderedHtml = RunReorder(*m_ctx, parent, "slot1", "slot3");
    ASSERT_FALSE(reorderedHtml.empty()) << "Reorder returned empty HTML";

    std::size_t posFooter    = PosOf(reorderedHtml, "<footer>");
    std::size_t posParagraph = PosOf(reorderedHtml, "<p>");
    std::size_t posHeader    = PosOf(reorderedHtml, "<h1>");

    EXPECT_LT(posFooter, posParagraph) << "Footer should come before paragraph";
    EXPECT_LT(posParagraph, posHeader) << "Paragraph should come before header";
}

// ----------------------------------------------------------------------------
// Тест 1.5. Многократные последовательные свапы
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, MultipleSequentialSwaps)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
  m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

  try
    {
        loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
        loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);
    }
    catch (const std::exception& e)
    {
        FAIL() << "Failed to load test KB: " << e.what();
    }

  ScAddr parent    = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ScAddr compBtn   = m_ctx->SearchElementBySystemIdentifier("comp_button");
  ScAddr compInput = m_ctx->SearchElementBySystemIdentifier("comp_input");
  ASSERT_TRUE(m_ctx->IsElement(parent));

  RunReorder(*m_ctx, parent, "slot1", "slot2");
  EXPECT_EQ(GetComponentId(*m_ctx, compBtn),   "slot2");
  EXPECT_EQ(GetComponentId(*m_ctx, compInput), "slot1");

  RunReorder(*m_ctx, parent, "slot1", "slot2");
  EXPECT_EQ(GetComponentId(*m_ctx, compBtn),   "slot1");
  EXPECT_EQ(GetComponentId(*m_ctx, compInput), "slot2");

  RunReorder(*m_ctx, parent, "slot1", "slot2");
  EXPECT_EQ(GetComponentId(*m_ctx, compBtn),   "slot2");
  EXPECT_EQ(GetComponentId(*m_ctx, compInput), "slot1");
}*/


// ============================================================================
//  Группа 2. HTMLTranslator::RegenerateHTMLRepresentation
// ============================================================================

// ----------------------------------------------------------------------------
// Тест 2.1. Регенерация обновляет содержимое существующей ссылки
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, RegenerateUpdatesExistingLink)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();

  loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
  loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);

  ScAddr parent    = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ScAddr compBtn   = m_ctx->SearchElementBySystemIdentifier("comp_button");
  ScAddr compInput = m_ctx->SearchElementBySystemIdentifier("comp_input");
  ASSERT_TRUE(m_ctx->IsElement(parent));

  ScAddr const firstReprLink = HTMLTranslator::TranslateScToHTML(*m_ctx, parent);
  ASSERT_TRUE(m_ctx->IsElement(firstReprLink));

  std::string htmlBefore;
  m_ctx->GetLinkContent(firstReprLink, htmlBefore);
  ASSERT_FALSE(htmlBefore.empty());

  ScAddr btnIdLink = utils::IteratorUtils::getAnyByOutRelation(
      m_ctx.get(), compBtn, HTMLTranslatorKeynodes::nrel_html_parameter_id);
  ScAddr inputIdLink = utils::IteratorUtils::getAnyByOutRelation(
      m_ctx.get(), compInput, HTMLTranslatorKeynodes::nrel_html_parameter_id);
  ASSERT_TRUE(m_ctx->IsElement(btnIdLink));
  ASSERT_TRUE(m_ctx->IsElement(inputIdLink));

  m_ctx->SetLinkContent(btnIdLink, "slot2");
  m_ctx->SetLinkContent(inputIdLink, "slot1");

  ScAddr const regenLink = HTMLTranslator::RegenerateHTMLRepresentation(*m_ctx, parent);
  ASSERT_TRUE(m_ctx->IsElement(regenLink));

  EXPECT_EQ(firstReprLink, regenLink)
      << "RegenerateHTMLRepresentation should update the existing link, not create a new one";

  std::string htmlAfter;
  m_ctx->GetLinkContent(regenLink, htmlAfter);
  EXPECT_NE(htmlBefore, htmlAfter)
      << "HTML content should change after regeneration";

  EXPECT_LT(PosOf(htmlAfter, "<input"), PosOf(htmlAfter, "<button"));
}

// ----------------------------------------------------------------------------
// Тест 2.2. RegenerateHTMLRepresentation без предварительного кэша
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, RegenerateWithoutCacheCreatesNewLink)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();

  loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
  loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);

  ScAddr parent = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ASSERT_TRUE(m_ctx->IsElement(parent));

  ScAddr regenLink;
  ASSERT_NO_THROW({
    regenLink = HTMLTranslator::RegenerateHTMLRepresentation(*m_ctx, parent);
  });

  ASSERT_TRUE(m_ctx->IsElement(regenLink))
      << "RegenerateHTMLRepresentation should create a link even without prior cache";

  std::string html;
  m_ctx->GetLinkContent(regenLink, html);
  EXPECT_FALSE(html.empty()) << "Generated HTML should not be empty";

  EXPECT_NE(PosOf(html, "<button"), std::string::npos);
  EXPECT_NE(PosOf(html, "<input"),  std::string::npos);
}


// ============================================================================
//  Группа 3. Граничные случаи и обработка ошибок
// ============================================================================

// ----------------------------------------------------------------------------
// Тест 3.1. Своп одинаковых ID — действие завершается успешно, HTML не меняется
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, SwapIdenticalIds_SucceedsWithoutChange)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
  m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

  loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
  loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);

  ScAddr parent = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ASSERT_TRUE(m_ctx->IsElement(parent));

  ScAction initAction = m_ctx->GenerateAction(HTMLTranslatorKeynodes::action_translate_sc_to_html);
  initAction.SetArguments(parent);
  initAction.InitiateAndWait();
  std::string const initialHtml = GetHTMLFromActionResult(*m_ctx, initAction);

  ScAddr linkA = MakeStringLink(*m_ctx, "slot1");
  ScAddr linkB = MakeStringLink(*m_ctx, "slot1");

  ScAction action = m_ctx->GenerateAction(HTMLTranslatorKeynodes::action_reorder_ui_components);
  action.SetArguments(parent, linkA, linkB);
  action.InitiateAndWait();

  EXPECT_TRUE(action.IsFinishedSuccessfully())
      << "Action with identical IDs should finish successfully";

  std::string const htmlAfter = GetHTMLFromActionResult(*m_ctx, action);
  if (!htmlAfter.empty())
    EXPECT_EQ(initialHtml, htmlAfter);
}

// ----------------------------------------------------------------------------
// Тест 3.2. Несуществующий первый ID — действие завершается неуспешно
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, NonExistentFirstId_FinishesUnsuccessfully)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
  m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

  loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
  loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);

  ScAddr parent = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ASSERT_TRUE(m_ctx->IsElement(parent));

  ScAddr linkA = MakeStringLink(*m_ctx, "ghost_slot");
  ScAddr linkB = MakeStringLink(*m_ctx, "slot2");

  ScAction action = m_ctx->GenerateAction(HTMLTranslatorKeynodes::action_reorder_ui_components);
  action.SetArguments(parent, linkA, linkB);
  action.InitiateAndWait();

  EXPECT_FALSE(action.IsFinishedSuccessfully());
}

// ----------------------------------------------------------------------------
// Тест 3.3. Несуществующий второй ID — действие завершается неуспешно
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, NonExistentSecondId_FinishesUnsuccessfully)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
  m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

  loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
  loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);

  ScAddr parent = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ASSERT_TRUE(m_ctx->IsElement(parent));

  ScAddr linkA = MakeStringLink(*m_ctx, "slot1");
  ScAddr linkB = MakeStringLink(*m_ctx, "nonexistent");

  ScAction action = m_ctx->GenerateAction(HTMLTranslatorKeynodes::action_reorder_ui_components);
  action.SetArguments(parent, linkA, linkB);
  action.InitiateAndWait();

  EXPECT_FALSE(action.IsFinishedSuccessfully());
}

// ----------------------------------------------------------------------------
// Тест 3.4. После неуспешного действия SC-память не изменилась (откат)
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, FailedAction_ScMemoryRemainsIntact)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
  m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

  loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
  loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);

  ScAddr parent    = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ScAddr compBtn   = m_ctx->SearchElementBySystemIdentifier("comp_button");
  ScAddr compInput = m_ctx->SearchElementBySystemIdentifier("comp_input");
  ASSERT_TRUE(m_ctx->IsElement(parent));

  std::string const btnIdBefore   = GetComponentId(*m_ctx, compBtn);
  std::string const inputIdBefore = GetComponentId(*m_ctx, compInput);

  ScAddr linkA = MakeStringLink(*m_ctx, "slot1");
  ScAddr linkB = MakeStringLink(*m_ctx, "does_not_exist");

  ScAction action = m_ctx->GenerateAction(HTMLTranslatorKeynodes::action_reorder_ui_components);
  action.SetArguments(parent, linkA, linkB);
  action.InitiateAndWait();

  ASSERT_FALSE(action.IsFinishedSuccessfully());

  EXPECT_EQ(GetComponentId(*m_ctx, compBtn),   btnIdBefore)
      << "comp_button ID should be unchanged after failed action";
  EXPECT_EQ(GetComponentId(*m_ctx, compInput), inputIdBefore)
      << "comp_input ID should be unchanged after failed action";
}

// ----------------------------------------------------------------------------
// Тест 3.5. Невалидный родительский компонент — действие завершается неуспешно
// ----------------------------------------------------------------------------
/*TEST_F(ReorderTest, InvalidParentComponent_FinishesUnsuccessfully)
{
  // Подписываемся только на агент, который будет вызван
  m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

  ScAddr invalidParent;  // пустой ScAddr — невалиден

  ScAddr linkA = MakeStringLink(*m_ctx, "slot1");
  ScAddr linkB = MakeStringLink(*m_ctx, "slot2");

  ScAction action = m_ctx->GenerateAction(HTMLTranslatorKeynodes::action_reorder_ui_components);
  action.SetArguments(invalidParent, linkA, linkB);
  action.InitiateAndWait();

  EXPECT_FALSE(action.IsFinishedSuccessfully());
}*/

// ----------------------------------------------------------------------------
// Тест 3.6. Пустые строки ID — действие завершается неуспешно
// ----------------------------------------------------------------------------
TEST_F(ReorderTest, EmptyStringIds_FinishesUnsuccessfully)
{
  m_ctx->SubscribeAgent<HTMLTranslatorAgent>();
  m_ctx->SubscribeAgent<UIComponentsReorderAgent>();

  loader.loadScsFile(*m_ctx, TEST_FILES_DIR_PATH + "test_reorder_components.scs");
  loadKB(*m_ctx, loader, TEST_KB_DIR_PATH);

  ScAddr parent = m_ctx->SearchElementBySystemIdentifier("test_parent_component");
  ASSERT_TRUE(m_ctx->IsElement(parent));

  ScAddr linkA = MakeStringLink(*m_ctx, "");
  ScAddr linkB = MakeStringLink(*m_ctx, "");

  ScAction action = m_ctx->GenerateAction(HTMLTranslatorKeynodes::action_reorder_ui_components);
  action.SetArguments(parent, linkA, linkB);
  action.InitiateAndWait();

  EXPECT_FALSE(action.IsFinishedSuccessfully())
      << "Action with empty ID strings should finish unsuccessfully";
}

// ----------------------------------------------------------------------------
// Тест 3.7. RegenerateHTMLRepresentation с невалидным компонентом бросает исключение
// ----------------------------------------------------------------------------
// Этот тест НЕ подписывается на агенты — проверяет только метод HTMLTranslator
TEST_F(ReorderTest, RegenerateInvalidComponent_ThrowsException)
{
  ScAddr invalid;

  EXPECT_THROW(
      { HTMLTranslator::RegenerateHTMLRepresentation(*m_ctx, invalid); },
      utils::ScException)
      << "RegenerateHTMLRepresentation should throw on invalid component";
}

}  // namespace uiComponentsReorderTest