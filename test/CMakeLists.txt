project(effolkronium_random_test) # Enable C language support for pthread

find_package(Threads REQUIRED)
include(set_up_warnings.cmake)

#[[
    \param IS_FAIL if TRUE then set up that test as
           fail to build test which is static library
    \param REG_EXP used in fail to build tests. Parse cmd output and find matches
           if no matches found then test failed
]]
function(add_basic_test TARGET COMPILE_DEFS TEST_SOURCES IS_FAIL REG_EXP)
    if(IS_FAIL)
        add_library(${TARGET} STATIC EXCLUDE_FROM_ALL ${TEST_SOURCES})
    else()
        add_executable(${TARGET} ${TEST_SOURCES})
    endif()
    
    set_up_warnings(${TARGET})
    target_link_libraries(${TARGET} Threads::Threads)
    target_compile_definitions(${TARGET} PRIVATE ${COMPILE_DEFS})
    target_include_directories(${TARGET} PRIVATE "../include" ${CMAKE_CURRENT_SOURCE_DIR})
    
    if(COVERAGE_ENABLED)
        message(STATUS "COVERAGE ENABLED")
        target_compile_options(${TARGET} PRIVATE -g -O0 --coverage)
    endif()
    
    set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
    )
    
    if(IS_FAIL)
        add_test(NAME "${TARGET}"
                 COMMAND ${CMAKE_COMMAND}
                 --build ${CMAKE_CURRENT_BINARY_DIR}
                 --target ${TARGET})
        set_property(TEST ${TARGET} PROPERTY
                     PASS_REGULAR_EXPRESSION ${REG_EXP})
    else()
        add_test(NAME ${TARGET} COMMAND ${TARGET})
    endif()
endfunction(add_basic_test)

function(add_main_test TARGET COMPILE_DEFS)
    add_basic_test(${TARGET} ${COMPILE_DEFS} "random_test.cpp" OFF "")
endfunction(add_main_test)

add_main_test("random_static_test" "RANDOM_STATIC")
add_main_test("random_thread_local_test" "RANDOM_THREAD_LOCAL")
add_main_test("random_local_test" "RANDOM_LOCAL")

function(add_fail_tests TARGET TEST_SOURCES REG_EXP)
    add_basic_test("${TARGET}_static" "RANDOM_STATIC" ${TEST_SOURCES} ON ${REG_EXP})
    add_basic_test("${TARGET}_thread_local" "RANDOM_THREAD_LOCAL" ${TEST_SOURCES} ON ${REG_EXP})
    add_basic_test("${TARGET}_local" "RANDOM_LOCAL" ${TEST_SOURCES} ON ${REG_EXP})
endfunction(add_fail_tests)

# Add fail to build tests
add_fail_tests("SignedUnsigned" "fail_signed_unsigned_test.cpp" "(no matching)|(fail)|(function)|(overloaded)")