set(TEMPLATE_AGENT_TEST_SOURCES
    "template_variables_search_test.cpp"
)
add_executable(template_agent-test ${TEMPLATE_AGENT_TEST_SOURCES})
target_include_directories(
    template_agent-test 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/.. ${SC_MEMORY_SRC} ${SC_KPM_SRC})

target_link_libraries(template_agent-test gtest_main ostis-specified-template-agent)

add_test(NAME specified-string-template-agent-test COMMAND template-agent-test)