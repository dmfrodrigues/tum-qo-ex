# ---------------------------------------------------------------------------
# QO Executables
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

list(APPEND EXAMPLES_SRC examples/ChiExample.cpp examples/JoinExample.cpp examples/ScanExample.cpp examples/SelectExample.cpp examples/AdminExample.cpp examples/IntegrationTester.cpp)
list(APPEND EXAMPLES_SRC) #append your own targets here

# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------

foreach (examplesourcefile ${EXAMPLES_SRC})
    get_filename_component(examplename_base ${examplesourcefile} NAME)
    string(REPLACE ".cpp" "" examplename ${examplename_base})
    add_executable(${examplename} ${examplesourcefile})
    target_link_libraries(${examplename} qolib)
    set_target_properties(${examplename} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endforeach (examplesourcefile ${EXAMPLES_SRC})
