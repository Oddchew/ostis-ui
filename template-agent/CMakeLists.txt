option("BUILD_BENCHMARK" OFF)
option("INJA_BUILD_TESTS" OFF)
FetchContent_Declare(
  inja
  GIT_REPOSITORY https://github.com/pantor/inja.git
  GIT_TAG v3.4.0 # You can specify a commit, tag, or branch here
)
FetchContent_MakeAvailable(inja)
find_package(nlohmann_json 3.2.0 REQUIRED)

set(SPECIFIED_TEMPLATE_AGENT_SOURCES
"SpecifiedStringTemplateAgent.cpp"
"SpecifiedStringTemplateKeynodes.cpp"
)

set(SPECIFIED_TEMPLATE_AGENT_HEADERS
"SpecifiedStringTemplateAgent.hpp"
"SpecifiedStringTemplateKeynodes.hpp")

add_library(
    ostis-specified-template-agent 
    SHARED ${SPECIFIED_TEMPLATE_AGENT_SOURCES} ${SPECIFIED_TEMPLATE_AGENT_HEADERS})

target_include_directories(
    ostis-specified-template-agent
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}
    PUBLIC ${SC_MEMORY_SRC} ${SC_KPM_SRC} ${inja_SOURCE_DIR}/single_include/inja)

target_link_libraries(ostis-specified-template-agent PUBLIC sc-memory sc-agents-common nlohmann_json::nlohmann_json)

sc_codegen_ex(ostis-specified-template-agent ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/generated)

if(${OSTIS_UI_BUILD_TESTS})
    add_subdirectory(tests)
endif()