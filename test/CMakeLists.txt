project(effolkronium_random_test) # Enable C language support for pthread

find_package(Threads REQUIRED)
include(set_up_warnings.cmake)

function(add_main_test TARGET COMPILE_DEFS)
    add_executable(${TARGET} random_test.cpp)
    set_up_warnings(${TARGET})
    
    target_link_libraries(${TARGET} Threads::Threads)
    target_compile_definitions(${TARGET} PRIVATE ${COMPILE_DEFS})
    target_include_directories(${TARGET} PRIVATE "../include")
    
    if(COVERAGE_ENABLED)
        message(STATUS "COVERAGE ENABLED")
        target_compile_options(${TARGET} PRIVATE -g -O0 --coverage)
    endif()
    
    set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
    )
    
    add_test(NAME ${TARGET} COMMAND ${TARGET})
endfunction(add_main_test)

add_main_test("random_static_test" "RANDOM_STATIC")
add_main_test("random_thread_local_test" "RANDOM_THREAD_LOCAL")
add_main_test("random_local_test" "RANDOM_LOCAL")