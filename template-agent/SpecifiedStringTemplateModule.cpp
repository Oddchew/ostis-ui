/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "agent/SpecifiedStringTemplateAgent.hpp"
#include "keynodes/SpecifiedStringTemplateKeynodes.hpp"

#include "SpecifiedStringTemplateModule.hpp"

using namespace specifiedStringTemplateModule;

SC_IMPLEMENT_MODULE(SpecifiedStringTemplateModule)

sc_result SpecifiedStringTemplateModule::InitializeImpl()
{
  if (!SpecifiedStringTemplateKeynodes::InitGlobal())
    return SC_RESULT_ERROR;

  SC_AGENT_REGISTER(SpecifiedStringTemplateAgent);

  return SC_RESULT_OK;
}

sc_result SpecifiedStringTemplateModule::ShutdownImpl()
{
  SC_AGENT_UNREGISTER(SpecifiedStringTemplateAgent);

  return SC_RESULT_OK;
}
