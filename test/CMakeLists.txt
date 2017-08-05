project(effolkronium_random_test) # Enable C language support for pthread

find_package(Threads REQUIRED)

if(COVERAGE_ENABLED)
	message(STATUS "COVERAGE ENABLED")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # debug, no optimisation
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage
endif()

include(set_up_warnings.cmake)

#[[
    \param IS_FAIL if TRUE then set up that test as
           fail to build test which is static library
    \param REG_EXP used in fail to build tests. Parse cmd output and find matches
           if no matches found then test failed
]]
function(add_basic_test TARGET COMPILE_DEFS TEST_SOURCES)
    add_executable(${TARGET} ${TEST_SOURCES})
    
    set_up_warnings(${TARGET})
    target_link_libraries(${TARGET} Threads::Threads)
    target_compile_definitions(${TARGET} PRIVATE ${COMPILE_DEFS})
    target_include_directories(${TARGET} PRIVATE "../include" ${CMAKE_CURRENT_SOURCE_DIR})
    
    set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
    )
    
    add_test(NAME ${TARGET} COMMAND ${TARGET})
endfunction(add_basic_test)

function(add_main_test TARGET COMPILE_DEFS)
    add_basic_test(${TARGET} ${COMPILE_DEFS} "random_test.cpp")
endfunction(add_main_test)

add_main_test("random_static_test" "RANDOM_STATIC")
add_main_test("random_thread_local_test" "RANDOM_THREAD_LOCAL")
add_main_test("random_local_test" "RANDOM_LOCAL")