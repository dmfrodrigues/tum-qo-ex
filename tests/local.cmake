# ---------------------------------------------------------------------------
# QO Executables
# ---------------------------------------------------------------------------
find_package(PythonInterp REQUIRED)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

list(APPEND TEST_CC tests/tester.cpp)
list(APPEND TEST_CC tests/task1.cpp)
list(APPEND TEST_CC tests/task2.cpp)
list(APPEND TEST_CC tests/task3.cpp)
list(APPEND TEST_CC tests/task4.cpp)
list(APPEND TEST_CC tests/task5.cpp)

# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------

add_executable(tester ${TEST_CC})
add_dependencies(tester catch_src)
target_link_libraries(tester qolib catch)
set_target_properties(tester PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

add_test(NAME Tests COMMAND tester WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

#add_executable(testgenerator "tests/testgenerator.cpp")
#target_link_libraries(testgenerator qolib catch)
#set_target_properties(testgenerator PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

add_custom_target(check
        COMMAND $<TARGET_FILE:tester>
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        )

add_dependencies(check tester)


# ---------------------------------------------------------------------------
# Linting
# ---------------------------------------------------------------------------

#add_tidy_target(lint_tools "${TESTS_SRC}")
#list(APPEND lint_targets lint_tools)

