/*
 * This source file is part of an OSTIS project. For the latest info, see
 * http://ostis.net Distributed under the MIT License (See accompanying file
 * COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

/*
 * Назначение:
 *   Изменить порядок (позиции) двух дочерних UI-компонентов внутри
 *   родительского компонента путём обмена их nrel_html_parameter_id в SC-памяти
 *   с последующей принудительной перегенерацией HTML родителя.
 *
 * Архитектурное обоснование:
 *   Позиция дочернего компонента в HTML определяется тем, какой placeholder
 *   {<id>} он занимает в шаблоне родителя (nrel_html_template). ID placeholder'а
 *   хранится в nrel_html_parameter_id дочернего компонента. Чтобы переместить
 *   два компонента относительно друг друга, достаточно поменять их ID местами —
 *   при следующей (принудительной) трансляции шаблон подставит компоненты
 *   в обратном порядке.
 *
 *          Родитель
 *          template: "<div>{slot1}{slot2}</div>"
 *
 *          comp_A ---nrel_html_parameter_id---> "slot1"   ← будет "slot2"
 *          comp_B ---nrel_html_parameter_id---> "slot2"   ← будет "slot1"
 *
 *   После свапа шаблон заполняется: comp_B встанет на место slot1, comp_A — slot2.
 */

#include "UIComponentsReorderAgent.hpp"

#include <sc-memory/sc_action.hpp>
#include <sc-memory/sc_event.hpp>
#include <sc-memory/sc_result.hpp>
#include <sc-memory/sc_structure.hpp>

#include <sc-agents-common/utils/IteratorUtils.hpp>

#include "html-translator/HTMLTranslator.hpp"
#include "keynodes/HTMLTranslatorKeynodes.hpp"
#include "parameter-retriever/ParameterRetriever.hpp"

using namespace utils;

namespace htmlTranslationModule
{

ScResult UIComponentsReorderAgent::DoProgram(ScActionInitiatedEvent const & event, ScAction & action)
{
  // ------------------------------------------------------------------
  // 1. Получаем аргументы
  // ------------------------------------------------------------------
  auto const [parentComponent, firstIdLink, secondIdLink] = action.GetArguments<3>();

  if (!parentComponent.IsValid())
  {
    SC_LOG_ERROR("UIComponentsReorderAgent: parent component argument is invalid.");
    return action.FinishUnsuccessfully();
  }
  if (!firstIdLink.IsValid() || !secondIdLink.IsValid())
  {
    SC_LOG_ERROR("UIComponentsReorderAgent: parameter ID link arguments are invalid.");
    return action.FinishUnsuccessfully();
  }

  // ------------------------------------------------------------------
  // 2. Читаем строки ID из ссылок
  // ------------------------------------------------------------------
  std::string firstId, secondId;
  m_context.GetLinkContent(firstIdLink, firstId);
  m_context.GetLinkContent(secondIdLink, secondId);

  if (firstId.empty())
  {
    SC_LOG_ERROR("UIComponentsReorderAgent: first parameter ID is empty.");
    return action.FinishUnsuccessfully();
  }
  if (secondId.empty())
  {
    SC_LOG_ERROR("UIComponentsReorderAgent: second parameter ID is empty.");
    return action.FinishUnsuccessfully();
  }
  if (firstId == secondId)
  {
    SC_LOG_WARNING("UIComponentsReorderAgent: both parameter IDs are equal — nothing to reorder.");
    // Отдаём текущее HTML-представление родителя без изменений
    ScAddr currentRepr = IteratorUtils::getAnyByOutRelation(
        &m_context, parentComponent, HTMLTranslatorKeynodes::nrel_html_representation);
    if (m_context.IsElement(currentRepr))
    {
      ScStructure result = m_context.GenerateStructure();
      result << currentRepr;
      action.SetResult(result);
    }
    return action.FinishSuccessfully();
  }

  // ------------------------------------------------------------------
  // 3. Получаем карту дочерних компонентов родителя: ID → ScAddr
  // ------------------------------------------------------------------
  StringScAddrMap nestedComponents;
  try
  {
    nestedComponents = ParameterRetriever::GetNestedUIComponents(m_context, parentComponent);
  }
  catch (utils::ScException const &)
  {
    SC_LOG_ERROR("UIComponentsReorderAgent: failed to retrieve nested components of parent.");
    return action.FinishUnsuccessfully();
  }

  // ------------------------------------------------------------------
  // 4. Ищем оба компонента по их ID
  // ------------------------------------------------------------------
  auto it1 = nestedComponents.find(firstId);
  if (it1 == nestedComponents.end())
  {
    SC_LOG_ERROR(
        "UIComponentsReorderAgent: child component with parameter_id='" + firstId + "' not found in parent.");
    return action.FinishUnsuccessfully();
  }

  auto it2 = nestedComponents.find(secondId);
  if (it2 == nestedComponents.end())
  {
    SC_LOG_ERROR(
        "UIComponentsReorderAgent: child component with parameter_id='" + secondId + "' not found in parent.");
    return action.FinishUnsuccessfully();
  }

  ScAddr const comp1 = it1->second;  // компонент, у которого ID == firstId
  ScAddr const comp2 = it2->second;  // компонент, у которого ID == secondId

  // ------------------------------------------------------------------
  // 5. Получаем SC-ссылки nrel_html_parameter_id каждого компонента
  // ------------------------------------------------------------------
  ScAddr const comp1IdScLink =
      IteratorUtils::getAnyByOutRelation(&m_context, comp1, HTMLTranslatorKeynodes::nrel_html_parameter_id);
  ScAddr const comp2IdScLink =
      IteratorUtils::getAnyByOutRelation(&m_context, comp2, HTMLTranslatorKeynodes::nrel_html_parameter_id);

  if (!m_context.IsElement(comp1IdScLink))
  {
    SC_LOG_ERROR("UIComponentsReorderAgent: nrel_html_parameter_id link for first component is invalid.");
    return action.FinishUnsuccessfully();
  }
  if (!m_context.IsElement(comp2IdScLink))
  {
    SC_LOG_ERROR("UIComponentsReorderAgent: nrel_html_parameter_id link for second component is invalid.");
    return action.FinishUnsuccessfully();
  }

  // ------------------------------------------------------------------
  // 6. Свапаем содержимое ID-ссылок в SC-памяти
  //    comp1: firstId  → secondId
  //    comp2: secondId → firstId
  // ------------------------------------------------------------------
  m_context.SetLinkContent(comp1IdScLink, secondId);
  m_context.SetLinkContent(comp2IdScLink, firstId);

  SC_LOG_DEBUG(
      "UIComponentsReorderAgent: swapped parameter IDs '" + firstId + "' <-> '" + secondId + "'.");

  // ------------------------------------------------------------------
  // 7. Принудительно перегенерируем HTML родителя (минуя кэш)
  // ------------------------------------------------------------------
  ScAddr updatedReprLink;
  try
  {
    updatedReprLink = HTMLTranslator::RegenerateHTMLRepresentation(m_context, parentComponent);
  }
  catch (utils::ScException const &)
  {
    // Откат свапа при ошибке трансляции
    m_context.SetLinkContent(comp1IdScLink, firstId);
    m_context.SetLinkContent(comp2IdScLink, secondId);
    SC_LOG_ERROR(
        "UIComponentsReorderAgent: HTML regeneration failed. Swap rolled back.");
    return action.FinishUnsuccessfully();
  }

  if (!m_context.IsElement(updatedReprLink))
  {
    // Откат свапа
    m_context.SetLinkContent(comp1IdScLink, firstId);
    m_context.SetLinkContent(comp2IdScLink, secondId);
    SC_LOG_ERROR("UIComponentsReorderAgent: regenerated HTML link is invalid. Swap rolled back.");
    return action.FinishUnsuccessfully();
  }

  // ------------------------------------------------------------------
  // 8. Формируем результат
  // ------------------------------------------------------------------
  ScStructure result = m_context.GenerateStructure();
  result << updatedReprLink;
  action.SetResult(result);

  SC_LOG_INFO(
      "UIComponentsReorderAgent: components '" + firstId + "' and '" + secondId
      + "' successfully reordered.");

  return action.FinishSuccessfully();
}

ScAddr UIComponentsReorderAgent::GetActionClass() const
{
  return HTMLTranslatorKeynodes::action_reorder_ui_components;
}

}  // namespace htmlTranslationModule
