#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "EasyRandom.hpp"

using namespace EasyRandom;

TEST_CASE( "TEST_TEST", "RANDOM" ) {
    const auto randomNumber = Random::get( );
    REQUIRE( randomNumber >= -10 );
    REQUIRE( randomNumber <= 10 );
}