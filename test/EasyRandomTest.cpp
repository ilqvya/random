#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "EasyRandom.hpp"
#include <limits>

using namespace EasyRandom;

TEST_CASE( "Test random random" ) {
    const auto firstRandomNumber = Random::get( std::numeric_limits<std::intmax_t>::min( ),
                                                std::numeric_limits<std::intmax_t>::max( ) );

    const auto secondRandomNumber = Random::get( std::numeric_limits<std::intmax_t>::min( ),
                                                 std::numeric_limits<std::intmax_t>::max( ) );

    CHECK( firstRandomNumber != secondRandomNumber );
}

TEST_CASE( "Test range" ) {
    for( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random::get( -1, 1 );
        REQUIRE( randomNumber >= -1 );
        REQUIRE( randomNumber <= 1 );
    }

    for( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random::get( 1, -1 );
        REQUIRE( randomNumber >= -1 );
        REQUIRE( randomNumber <= 1 );
    }

    const auto randomNumber = Random::get( 1, 1 );

    REQUIRE( 1 == randomNumber );
}

TEST_CASE( "Test type deduction" ) {
    static_assert( std::is_same<int, decltype( Random::get( -10, 10 ) )>::value, "" );
    static_assert( std::is_same<unsigned, decltype( Random::get( 10u, 10u ) )>::value, "" );
    static_assert( std::is_same<long, decltype( Random::get( 10, 10l ) )>::value, "" );
    static_assert( std::is_same<long long, decltype( Random::get( 10ll , 10l ) ) >::value, "" );
    static_assert( std::is_same<unsigned long long, decltype( Random::get( 10llu, 10lu ) ) >::value, "" );
    static_assert( std::is_same<short, decltype( Random::get<short, short>( 10llu, 10lu ) ) >::value, "" );
    static_assert( std::is_same<unsigned short, 
                   decltype( Random::get<unsigned short, unsigned short>( 10llu, 10lu ) ) >::value, "" );
}