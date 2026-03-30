set(HTML_TRANSLATOR_AGENT_TEST_SOURCES
    "units/HTMLTranslationAgentTest.cpp"
    "units/HTMLTranslationTest.cpp"
    "units/utils.hpp"
    "units/utils.cpp"
)
add_executable(ostis-ui-html-translator-agent-test ${HTML_TRANSLATOR_AGENT_TEST_SOURCES})

target_link_libraries(ostis-ui-html-translator-agent-test
    LINK_PRIVATE GTest::gtest_main
    LINK_PRIVATE ostis-ui-html-translator
    LINK_PRIVATE sc-machine::sc-builder-lib
)

target_include_directories(
    ostis-ui-html-translator-agent-test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
gtest_discover_tests(ostis-ui-html-translator-agent-test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/units)
