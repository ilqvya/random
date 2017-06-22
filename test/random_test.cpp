#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "random.hpp"
#include <limits>

using Random = effolkronium::random_static;

#define DOT ::

TEST_CASE( "Test random random integer" ) {
    const auto firstRandomNumber = Random DOT get( std::numeric_limits<std::intmax_t>::min( ),
                                                std::numeric_limits<std::intmax_t>::max( ) );

    const auto secondRandomNumber = Random::get( std::numeric_limits<std::intmax_t>::min( ),
                                                 std::numeric_limits<std::intmax_t>::max( ) );

	//MAY FAIL
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

TEST_CASE( "Test type deduction integer") {
    static_assert( std::is_same<int, decltype( Random::get( -10, 10 ) )>::value, "" );
    static_assert( std::is_same<unsigned, decltype( Random::get( 10u, 10u ) )>::value, "" );
	static_assert(std::is_same<long, decltype(Random::get(10l, 10l))>::value, "");
	static_assert(std::is_same<short, decltype(Random::get(short{ 1 }, short{ 2 })) > ::value, "");
}

TEST_CASE( "Test random random real" ) {
    const auto firstRandomNumber = Random::get( std::numeric_limits<long double>::min( ),
                                                std::numeric_limits<long double>::max( ) );

    const auto secondRandomNumber = Random::get( std::numeric_limits<long double>::min( ),
                                                 std::numeric_limits<long double>::max( ) );

	//MAY FAIL
    CHECK( firstRandomNumber != secondRandomNumber );
}

TEST_CASE( "Test range real" ) {
	//float
    for( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random::get( -1.f, 1.f );
        REQUIRE( randomNumber >= -1.f );
        REQUIRE( randomNumber <= 1.f );
    }

	//double
    for( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random::get( 1., -1. );
        REQUIRE( randomNumber >= -1. );
        REQUIRE( randomNumber <= 1. );
    }

	//long double
    for( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random::get( 1.l, -1.l );
        REQUIRE( randomNumber >= -1.l );
        REQUIRE( randomNumber <= 1.l );
    }

	//range with 0 gap
    const auto randomNumber = Random::get( 1.f, 1.f );
    
    REQUIRE( 1.l == randomNumber );
}

TEST_CASE( "Test type deduction real" ) {
    static_assert( std::is_same<double, decltype( Random::get( 10., 10. ) )>::value, "" );
    static_assert( std::is_same<long double, decltype( Random::get( 10.l, 10.l ) )>::value, "" );
    static_assert( std::is_same<float, decltype( Random::get( 10.f, 10.f ) )>::value, "" );
}
