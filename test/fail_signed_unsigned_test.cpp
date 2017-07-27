#include "catch.hpp"
#include "current_random.hpp"

TEST_CASE( "FAIL_TEST" "Prevent signed unsigned range common arguments" ) {
    // Must fail
    Random DOT get<Random_t::common>( -1, 1u );
}