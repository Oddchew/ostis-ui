set(HTML_TRANSLATOR_SOURCES
	"HTMLTranslatorModule.cpp"
	"agents/HTMLTranslatorAgent.cpp"
	"keynodes/HTMLTranslatorKeynodes.cpp"
)

set(HTML_TRANSLATOR_HEADERS
    "HTMLTranslatorModule.hpp"
    "agents/HTMLTranslatorAgent.hpp"
    "keynodes/HTMLTranslatorKeynodes.hpp"
)

add_library(
        ostis-ui-html-translator
        SHARED ${HTML_TRANSLATOR_SOURCES} ${HTML_TRANSLATOR_HEADERS})

target_include_directories(
        ostis-ui-html-translator
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}
        PUBLIC ${SC_MEMORY_SRC} ${SC_KPM_SRC})

target_link_libraries(ostis-ui-html-translator PUBLIC sc-memory sc-agents-common)

sc_codegen_ex(ostis-ui-html-translator ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/generated)

if (${OSTIS_UI_BUILD_TESTS})
    add_subdirectory(tests)
endif ()
