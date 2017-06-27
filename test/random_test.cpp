#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "random.hpp"
#include <limits>

using Random = effolkronium::random_static;

#define DOT ::

TEST_CASE( "Range overflow for random integer numbers" ) {
    bool isRangeOverflow = false;

    // From lower to greater
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( )
         && !isRangeOverflow;
         ++i ) {
        const auto randomNumber = Random DOT get( -1, 1 );
        isRangeOverflow = randomNumber < -1 || randomNumber > 1;
    }
    REQUIRE( !isRangeOverflow );

    // From greater to lower
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get( 1, -1 );
        isRangeOverflow = randomNumber < -1 || randomNumber > 1;
    }
    REQUIRE( !isRangeOverflow );

    // Range with 0 gap
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get( 0, 0 );
        isRangeOverflow = randomNumber != 0;
    }
    REQUIRE( !isRangeOverflow );
}

TEST_CASE( "Type deduction for random integer numbers" ) {
    // short
    static_assert( std::is_same<short,
                   decltype( Random DOT get(
                       short{ 0 },
                       short{ 0 } ) ) > ::value, "" );
    // unsigned short
    static_assert( std::is_same<unsigned short,
                   decltype( Random DOT get(
                       static_cast<unsigned short>( 0u ),
                       static_cast<unsigned short>( 0u ) ) ) > ::value, "" );
    // int
    static_assert( std::is_same<int,
                   decltype( Random DOT get( 0, 0 ) )>::value, "" );
    // unsigned int
    static_assert( std::is_same<unsigned int,
                   decltype( Random DOT get( 0u, 0u ) )>::value, "" );
    // long
    static_assert( std::is_same<long,
                   decltype( Random DOT get( 0l, 0l ) )>::value, "" );
    // unsigned long
    static_assert( std::is_same<unsigned long,
                   decltype( Random DOT get( 0lu, 0lu ) )>::value, "" );
    // long long
    static_assert( std::is_same<long long,
                   decltype( Random DOT get( 0ll, 0ll ) )>::value, "" );
    // unsigned long long
    static_assert( std::is_same<unsigned long long,
                   decltype( Random DOT get( 0ull, 0ull ) )>::value, "" );
}

TEST_CASE( "Random integer numbres is truly random" ) {
    bool isDifferentNumber{ false };

    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( )
         && !isDifferentNumber;
         ++i ) {

        const auto firstRandomNumber = Random DOT get(
            std::numeric_limits<long long>::min( ),
            std::numeric_limits<long long>::max( ) );

        const auto secondRandomNumber = Random DOT get(
            std::numeric_limits<long long>::min( ),
            std::numeric_limits<long long>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    }

    // May fail but very rarely
    CHECK( isDifferentNumber );
}

TEST_CASE( "Range overflow for random real numbers" ) {
    bool isRangeOverflow = false;

    // From lower to greater
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( )
         && !isRangeOverflow;
         ++i ) {
        const auto randomNumber = Random DOT get( -1., 1. );
        isRangeOverflow = randomNumber < -1. || randomNumber > 1.;
    }
    REQUIRE( !isRangeOverflow );

    // From greater to lower
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get( 1., -1. );
        isRangeOverflow = randomNumber < -1. || randomNumber > 1.;
    }
    REQUIRE( !isRangeOverflow );

    // Range with 0 gap
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get( 0., 0. );
        isRangeOverflow = randomNumber != 0.;
    }
    REQUIRE( !isRangeOverflow );
}

TEST_CASE( "Type deduction for random real numbers" ) {
    // float
    static_assert( std::is_same<float,
                   decltype( Random DOT get(
                       0.f, 0.f ) ) > ::value, "" );
    // double
    static_assert( std::is_same<double,
                   decltype( Random DOT get(
                       0., 0. ) ) > ::value, "" );
    // long double
    static_assert( std::is_same<long double,
                   decltype( Random DOT get( 0.l, 0.l ) )>::value, "" );
}

TEST_CASE( "Random real numbres is truly random" ) {
    bool isDifferentNumber{ false };

    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( )
         && !isDifferentNumber;
         ++i ) {

        const auto firstRandomNumber = Random DOT get(
            std::numeric_limits<long double>::min( ),
            std::numeric_limits<long double>::max( ) );

        const auto secondRandomNumber = Random DOT get(
            std::numeric_limits<long double>::min( ),
            std::numeric_limits<long double>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    }

    // May fail but very rarely
    CHECK( isDifferentNumber );
}

TEST_CASE( "Range overflow for random byte numbers" ) {
    bool isRangeOverflow = false;

    // From lower to greater
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( )
         && !isRangeOverflow;
         ++i ) {
        const auto randomNumber = Random DOT get( static_cast<signed char>( -1 ),
                                                  static_cast<signed char>( 1 ) );
        isRangeOverflow = randomNumber < -1 || randomNumber > 1;
    }
    REQUIRE( !isRangeOverflow );

    // From greater to lower
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get( static_cast<signed char>( 1 ),
                                                  static_cast<signed char>( -1 ) );
        isRangeOverflow = randomNumber < -1. || randomNumber > 1.;
    }
    REQUIRE( !isRangeOverflow );

    // Range with 0 gap
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get( static_cast<signed char>( 0 ),
                                                  static_cast<signed char>( 0 ) );
        isRangeOverflow = randomNumber != 0.;
    }
    REQUIRE( !isRangeOverflow );
}

TEST_CASE( "Type deduction for random byte numbers" ) {
    // signed char
    static_assert( std::is_same<signed char,
                   decltype( Random DOT get(
                       static_cast<signed char>( 0 ),
                       static_cast<signed char>( 0 ) ) ) > ::value, "" );
    // unsigned char
    static_assert( std::is_same<unsigned char,
                   decltype( Random DOT get(
                       static_cast<unsigned char>( 0 ),
                       static_cast<unsigned char>( 0 ) ) ) > ::value, "" );
    // std::int8_t
    static_assert( std::is_same<std::int8_t,
                   decltype( Random DOT get(
                       std::int8_t{ 0 },
                       std::int8_t{ 0 } ) ) > ::value, "" );
    // std::uint8_t
    static_assert( std::is_same<std::uint8_t,
                   decltype( Random DOT get(
                       std::uint8_t{ 0 },
                       std::uint8_t{ 0 } ) ) > ::value, "" );
}

TEST_CASE( "Random byte numbres is truly random" ) {
    bool isDifferentNumber{ false };

    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( )
         && !isDifferentNumber;
         ++i ) {

        const auto firstRandomNumber = Random DOT get(
            std::numeric_limits<signed char>::min( ),
            std::numeric_limits<signed char>::max( ) );

        const auto secondRandomNumber = Random DOT get(
            std::numeric_limits<signed char>::min( ),
            std::numeric_limits<signed char>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    }

    // May fail but very rarely
    CHECK( isDifferentNumber );
}

TEST_CASE( "Range overflow for random common_type numbers" ) {
    bool isRangeOverflow = false;

    // From lower to greater
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( )
         && !isRangeOverflow;
         ++i ) {
        const auto randomNumber = Random DOT get<Random::common>( 
                                                 static_cast<signed char>( -1 ),
                                                 short{ 1 } );

        isRangeOverflow = randomNumber < -1 || randomNumber > 1;
    }
    REQUIRE( !isRangeOverflow );

    // From greater to lower
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get<Random::common>( 
                                           1ull, 0u );

        isRangeOverflow = 1 < randomNumber;
    }
    REQUIRE( !isRangeOverflow );

    // Range with 0 gap
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get<Random::common>(
                                                static_cast<signed char>( 0 ),
                                                0ll );
        isRangeOverflow = randomNumber != 0;
    }
    REQUIRE( !isRangeOverflow );
}

TEST_CASE( "Type deduction for random common_type numbers" ) {
    // signed char
    static_assert( std::is_same<signed char,
                   decltype( Random DOT get<Random::common>(
                       static_cast<signed char>( 0 ),
                       static_cast<signed char>( 0 ) ) ) > ::value, "" );
    // unsigned long
    static_assert( std::is_same<unsigned long,
                   decltype( Random DOT get<Random::common>(
                       static_cast<unsigned char>( 0u ), 0ul ) ) > ::value, "" );
    // double
    static_assert( std::is_same<double,
                   decltype( Random DOT get<Random::common>(
                       0.,
                       std::int8_t{ 0 } ) ) > ::value, "" );
    // long double
    static_assert( std::is_same<long double,
                   decltype( Random DOT get<Random::common>(
                       0.f, 0.l ) ) > ::value, "" );
}

TEST_CASE( "Random common type numbres is truly random" ) {
    bool isDifferentNumber{ false };

    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( )
         && !isDifferentNumber;
         ++i ) {

        const auto firstRandomNumber = Random DOT get<Random::common>(
            std::numeric_limits<float>::min( ),
            std::numeric_limits<signed char>::max( ) );

        const auto secondRandomNumber = Random DOT get<Random::common>(
            std::numeric_limits<signed char>::min( ),
            std::numeric_limits<float>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    }

    // May fail but very rarely
    CHECK( isDifferentNumber );
}

TEST_CASE( "Random bool values" ) {
    SECTION( "probability = 1" ) {
        REQUIRE( Random DOT get<bool>( 1 ) );
    }

    SECTION( "probability = 0" ) {
        REQUIRE( !Random DOT get<bool>( 0 ) );
    }

    SECTION( "generate true and false" ) {
        while( Random DOT get<bool>( ) == Random DOT get<bool>( ) );
        REQUIRE( true == true );
    }

    SECTION( "out of range argument" ) {
        // Random DOT get<bool>( 1.1 );
        // Random DOT get<bool>( -1.1 );
        // assert occurred!
    }
}

TEST_CASE( "Random value from initilizer list by value" ) {
    bool is1{ false }, is2{ false }, is3{ false };
    while( !is1 && !is2 && !is3 ) {
        switch( Random DOT get( { 1, 2, 3 } ) ) {
            case 1: is1 = true; break;
            case 2: is2 = true; break;
            case 3: is3 = true; break;
        }
    }
    REQUIRE( true == true );
    //Random DOT get<int>( {  } ); assertion occurred
}

TEST_CASE( "Noexcept deduction in get from initilizer list by value" ) {
    class NoexceptCopy {
    public:
        NoexceptCopy( ) = default;
        NoexceptCopy( const NoexceptCopy& ) noexcept( true ) { };
    };

    class NotNoexceptCopy {
    public:
        NotNoexceptCopy( ) = default;
        NotNoexceptCopy( const NotNoexceptCopy& ) noexcept( false ) { };
    };

    class NoDefaultConstructorNoexcept {
    public:
        NoDefaultConstructorNoexcept( ) = delete;
        NoDefaultConstructorNoexcept( int ) noexcept { }
    };

    class NoDefaultConstructorNotNoexcept {
    public:
        NoDefaultConstructorNotNoexcept( ) = delete;
        NoDefaultConstructorNotNoexcept( int ) noexcept( false ) { }
    };

    static_assert( noexcept( 
        Random DOT get( { NoexceptCopy{ } } ) ), " " );

    static_assert( !noexcept( 
        Random DOT get( { NotNoexceptCopy{ } } ) ), " " );

    static_assert( noexcept(
        Random DOT get( { NoDefaultConstructorNoexcept{ 1 } } ) ), " " );

    static_assert( !noexcept(
        Random DOT get( { NoDefaultConstructorNotNoexcept{ 1 } } ) ), " " );
}

TEST_CASE( "Move constructor usage in get from initilizer list by value" ) { 
    class NoexceptMoveNoexceptCopy {
    public:
        NoexceptMoveNoexceptCopy( ) = default;
        NoexceptMoveNoexceptCopy(
            const NoexceptMoveNoexceptCopy& ) noexcept  { ++copied_num; };
        NoexceptMoveNoexceptCopy(
            NoexceptMoveNoexceptCopy&& ) noexcept { ++moved_num; };

        std::size_t moved_num{ 0u };
        std::size_t copied_num{ 0u };
    };

    auto val = Random DOT get( { NoexceptMoveNoexceptCopy{ } } );

    // Bad world! std::initilizer_list don't support move semantic }:
    REQUIRE( 1 == val.copied_num );
    REQUIRE( 0 == val.moved_num );
}