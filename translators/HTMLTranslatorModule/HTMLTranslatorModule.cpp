#include "HTMLTranslatorModule.hpp"

SC_IMPLEMENT_MODULE(HTMLTranslatorModule)

sc_result HTMLTranslatorModule::InitializeImpl()
{
  ScMemoryContext ctx(sc_access_lvl_make_min, "HTMLTranslatorModule");
  // SC_AGENT_REGISTER(SpecifiedStringTemplateAgent);
  return SC_RESULT_OK;
}

sc_result HTMLTranslatorModule::ShutdownImpl()
{
  // SC_AGENT_UNREGISTER(SpecifiedStringTemplateAgent);
  return SC_RESULT_OK;
}