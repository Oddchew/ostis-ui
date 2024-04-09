set(TEMPLATE_AGENT_TEST_SOURCES
    "units/SpecifiedStringTemplateAgentTest.cpp"
    "units/StringTemplateRendererTest.cpp"
)
add_executable(template-agent-test ${TEMPLATE_AGENT_TEST_SOURCES})

target_link_libraries(template-agent-test gtest_main ostis-specified-template-agent sc-memory sc-core sc-agents-common sc-builder-lib)

target_include_directories(
    template-agent-test
    PRIVATE ${SC_MEMORY_SRC}/tests/sc-memory/_test ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/.. ${SC_MEMORY_SRC} ${SC_TOOLS_SRC}/sc-builder/src ${SC_KPM_SRC})


add_definitions(-DTEMPLATE_AGENT_TEST_SRC_PATH="${CMAKE_CURRENT_LIST_DIR}")

add_test(NAME template-agent-test COMMAND template-agent-test)
