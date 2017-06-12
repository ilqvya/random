#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "EasyRandom.hpp"
#include <limits>

using namespace EasyRandom;

TEST_CASE( "Test random random integer" ) {
    const auto firstRandomNumber = Random::get( std::numeric_limits<std::intmax_t>::min( ),
                                                std::numeric_limits<std::intmax_t>::max( ) );

    const auto secondRandomNumber = Random::get( std::numeric_limits<std::intmax_t>::min( ),
                                                 std::numeric_limits<std::intmax_t>::max( ) );

    CHECK( firstRandomNumber != secondRandomNumber );
}

TEST_CASE( "Test range integer" ) {
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

TEST_CASE( "Test type deduction integer" ) {
    static_assert( std::is_same<int, decltype( Random::get( -10, 10 ) )>::value, "" );
    static_assert( std::is_same<unsigned, decltype( Random::get( 10u, 10u ) )>::value, "" );
    static_assert( std::is_same<long, decltype( Random::get( 10, 10l ) )>::value, "" );
    static_assert( std::is_same<long long, decltype( Random::get( 10ll , 10l ) ) >::value, "" );
    static_assert( std::is_same<unsigned long long, decltype( Random::get( 10llu, 10lu ) ) >::value, "" );
    static_assert( std::is_same<short, decltype( Random::get<short, short>( 10llu, 10lu ) ) >::value, "" );
    static_assert( std::is_same<unsigned short, 
                   decltype( Random::get<unsigned short, unsigned short>( 10llu, 10lu ) ) >::value, "" );
}

TEST_CASE( "Test random random real" ) {
    const auto firstRandomNumber = Random::get( std::numeric_limits<long double>::min( ),
                                                std::numeric_limits<long double>::max( ) );

    const auto secondRandomNumber = Random::get( std::numeric_limits<long double>::min( ),
                                                 std::numeric_limits<long double>::max( ) );

    CHECK( firstRandomNumber != secondRandomNumber );
}

TEST_CASE( "Test range real" ) {
    for( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random::get( -1.f, 1.f );
        REQUIRE( randomNumber >= -1.f );
        REQUIRE( randomNumber <= 1.f );
    }

    for( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random::get( 1., -1. );
        REQUIRE( randomNumber >= -1. );
        REQUIRE( randomNumber <= 1. );
    }

    for( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random::get( 1.l, -1.l );
        REQUIRE( randomNumber >= -1.l );
        REQUIRE( randomNumber <= 1.l );
    }

    const auto randomNumber = Random::get( 1.f, 1.l );
    
    REQUIRE( 1.l == randomNumber );
}

TEST_CASE( "Test type deduction real" ) {
    static_assert( std::is_same<double, decltype( Random::get( 10., 10. ) )>::value, "" );
    static_assert( std::is_same<long double, decltype( Random::get( 10.l, 10.l ) )>::value, "" );
    static_assert( std::is_same<float, decltype( Random::get( 10.f, 10.f ) )>::value, "" );
    static_assert( std::is_same<double, decltype( Random::get( 10.f, 10. ) ) >::value, "" );
    static_assert( std::is_same<long double, decltype( Random::get( 10.l, 10.f ) ) >::value, "" );
    static_assert( std::is_same<short, decltype( Random::get<short, short>( 10llu, 10lu ) ) >::value, "" );
}
