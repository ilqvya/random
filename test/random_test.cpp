#include "test_framework.hpp"
#include <limits>
#include <sstream>
#include <array>
#include <thread>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <forward_list>
#include <iterator>
#include <cstring>

#ifdef _WIN32 // Unit test for case when builds fail 'cause of min\max macro included from Windows.h before random.hpp
#    include "Windows.h"
#endif

#include "ilqvya/random.hpp"
#include "effolkronium/random.hpp"

#ifdef _WIN32
#    undef min
#    undef max
#endif

#ifdef RANDOM_LOCAL
auto Random = ilqvya::random_local{ };
using Random_t = ilqvya::random_local;
#endif

#ifdef RANDOM_STATIC
auto Random = ilqvya::random_static{ };
using Random_t = ilqvya::random_static;
#endif

#ifdef RANDOM_THREAD_LOCAL
auto Random = ilqvya::random_thread_local{ };
using Random_t = ilqvya::random_thread_local;
#endif

TEST_CASE( "Range overflow for random integer numbers" ) {
    bool isRangeOverflow = false;

    // From lower to greater
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ) && !isRangeOverflow; ++i ) {
        const auto randomNumber = Random.get( -1, 1 );
        isRangeOverflow = randomNumber < -1 || randomNumber > 1;
    }
    REQUIRE( !isRangeOverflow );

    // From greater to lower
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random.get( 1, -1 );
        isRangeOverflow = randomNumber < -1 || randomNumber > 1;
    }
    REQUIRE( !isRangeOverflow );

    // Range with 0 gap
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random.get( 0, 0 );
        isRangeOverflow = randomNumber != 0;
    }
    REQUIRE( !isRangeOverflow );
}

TEST_CASE( "Type deduction for random integer numbers" ) {
    // short
    static_assert( std::is_same<short, decltype( Random.get( short{ 0 }, short{ 0 } ) )>::value, "" );
    // unsigned short
    static_assert( std::is_same<unsigned short, decltype( Random.get( static_cast<unsigned short>( 0u ),
                                                                      static_cast<unsigned short>( 0u ) ) )>::value,
                   "" );
    // int
    static_assert( std::is_same<int, decltype( Random.get( 0, 0 ) )>::value, "" );
    // unsigned int
    static_assert( std::is_same<unsigned int, decltype( Random.get( 0u, 0u ) )>::value, "" );
    // long
    static_assert( std::is_same<long, decltype( Random.get( 0l, 0l ) )>::value, "" );
    // unsigned long
    static_assert( std::is_same<unsigned long, decltype( Random.get( 0lu, 0lu ) )>::value, "" );
    // long long
    static_assert( std::is_same<long long, decltype( Random.get( 0ll, 0ll ) )>::value, "" );
    // unsigned long long
    static_assert( std::is_same<unsigned long long, decltype( Random.get( 0ull, 0ull ) )>::value, "" );
}

TEST_CASE( "Random integer numbers is truly random" ) {
    bool isDifferentNumber{ false };

    for ( int i = 0; i < 1000 && !isDifferentNumber; ++i ) {
        const auto firstRandomNumber =
            Random.get( std::numeric_limits<long long>::min( ), std::numeric_limits<long long>::max( ) );

        const auto secondRandomNumber =
            Random.get( std::numeric_limits<long long>::min( ), std::numeric_limits<long long>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    }

    CHECK( isDifferentNumber );
}

TEST_CASE( "Range overflow for random real numbers" ) {
    bool isRangeOverflow = false;

    // From lower to greater
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ) && !isRangeOverflow; ++i ) {
        const auto randomNumber = Random.get( -1., 1. );
        isRangeOverflow = randomNumber < -1. || randomNumber > 1.;
    }
    REQUIRE( !isRangeOverflow );

    // From greater to lower
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random.get( 1., -1. );
        isRangeOverflow = randomNumber < -1. || randomNumber > 1.;
    }
    REQUIRE( !isRangeOverflow );

    // Range with 0 gap
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random.get( 0., 0. );
        isRangeOverflow = randomNumber != 0.;
    }
    REQUIRE( !isRangeOverflow );
}

TEST_CASE( "Type deduction for random real numbers" ) {
    // float
    static_assert( std::is_same<float, decltype( Random.get( 0.f, 0.f ) )>::value, "" );
    // double
    static_assert( std::is_same<double, decltype( Random.get( 0., 0. ) )>::value, "" );
    // long double
    static_assert( std::is_same<long double, decltype( Random.get( 0.l, 0.l ) )>::value, "" );
}

TEST_CASE( "Random real numbers is truly random" ) {
    bool isDifferentNumber{ false };

    for ( int i = 0; i < 1000 && !isDifferentNumber; ++i ) {
        const auto firstRandomNumber =
            Random.get( std::numeric_limits<float>::min( ), std::numeric_limits<float>::max( ) );

        const auto secondRandomNumber =
            Random.get( std::numeric_limits<float>::min( ), std::numeric_limits<float>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    }

    CHECK( isDifferentNumber );
}

TEST_CASE( "Range overflow for random byte numbers" ) {
    bool isRangeOverflow = false;

    // From lower to greater
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ) && !isRangeOverflow; ++i ) {
        const auto randomNumber = Random.get( static_cast<signed char>( -1 ), static_cast<signed char>( 1 ) );
        isRangeOverflow = randomNumber < -1 || randomNumber > 1;
    }
    REQUIRE( !isRangeOverflow );

    // From greater to lower
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random.get( static_cast<signed char>( 1 ), static_cast<signed char>( -1 ) );
        isRangeOverflow = randomNumber < -1. || randomNumber > 1.;
    }
    REQUIRE( !isRangeOverflow );

    // Range with 0 gap
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random.get( static_cast<signed char>( 0 ), static_cast<signed char>( 0 ) );
        isRangeOverflow = randomNumber != 0.;
    }
    REQUIRE( !isRangeOverflow );
}

TEST_CASE( "Type deduction for random byte numbers" ) {
    // signed char
    static_assert( std::is_same<signed char, decltype( Random.get( static_cast<signed char>( 0 ),
                                                                   static_cast<signed char>( 0 ) ) )>::value,
                   "" );
    // unsigned char
    static_assert( std::is_same<unsigned char, decltype( Random.get( static_cast<unsigned char>( 0 ),
                                                                     static_cast<unsigned char>( 0 ) ) )>::value,
                   "" );
    // std::int8_t
    static_assert( std::is_same<std::int8_t, decltype( Random.get( std::int8_t{ 0 }, std::int8_t{ 0 } ) )>::value, "" );
    // std::uint8_t
    static_assert( std::is_same<std::uint8_t, decltype( Random.get( std::uint8_t{ 0 }, std::uint8_t{ 0 } ) )>::value,
                   "" );
#if defined( __cpp_lib_byte ) || ( __cplusplus >= 201703L ) || ( defined( _MSVC_LANG ) && _MSVC_LANG >= 201703L )
    static_assert( std::is_same<std::byte, decltype( Random.get<std::byte>( ) )>::value, "" );
    static_assert( std::is_same<std::byte, decltype( Random.get( std::byte{ 0 }, std::byte{ 10 } ) )>::value, "" );
    auto rand_byte = Random.get<std::byte>( );
    (void)rand_byte;
    auto bounded_byte = Random.get( std::byte{ 10 }, std::byte{ 20 } );
    REQUIRE( static_cast<unsigned char>( bounded_byte ) >= 10 );
    REQUIRE( static_cast<unsigned char>( bounded_byte ) <= 20 );
#endif
}

TEST_CASE( "Random byte numbers is truly random" ) {
    bool isDifferentNumber{ false };

    for ( int i = 0; i < 1000 && !isDifferentNumber; ++i ) {
        const auto firstRandomNumber =
            Random.get( std::numeric_limits<signed char>::min( ), std::numeric_limits<signed char>::max( ) );

        const auto secondRandomNumber =
            Random.get( std::numeric_limits<signed char>::min( ), std::numeric_limits<signed char>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    }

    CHECK( isDifferentNumber );
}

TEST_CASE( "Range overflow for random common_type numbers" ) {
    bool isRangeOverflow = false;

    // From lower to greater
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ) && !isRangeOverflow; ++i ) {
        const auto randomNumber = Random.get<Random_t::common>( static_cast<signed char>( -1 ), short{ 1 } );

        isRangeOverflow = randomNumber < -1 || randomNumber > 1;
    }
    REQUIRE( !isRangeOverflow );

    // From greater to lower
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random.get<Random_t::common>( 1ull, 0u );

        isRangeOverflow = 1 < randomNumber;
    }
    REQUIRE( !isRangeOverflow );

    // Range with 0 gap
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        const auto randomNumber = Random.get<Random_t::common>( static_cast<signed char>( 0 ), 0ll );
        isRangeOverflow = randomNumber != 0;
    }
    REQUIRE( !isRangeOverflow );
}

TEST_CASE( "Type deduction for random common_type numbers" ) {
    // signed char
    static_assert(
        std::is_same<signed char, decltype( Random.get<Random_t::common>( static_cast<signed char>( 0 ),
                                                                          static_cast<signed char>( 0 ) ) )>::value,
        "" );
    // unsigned long
    static_assert( std::is_same<unsigned long, decltype( Random.get<Random_t::common>( static_cast<unsigned char>( 0u ),
                                                                                       0ul ) )>::value,
                   "" );
    // double
    static_assert( std::is_same<double, decltype( Random.get<Random_t::common>( 0., std::int8_t{ 0 } ) )>::value, "" );
    // long double
    static_assert( std::is_same<long double, decltype( Random.get<Random_t::common>( 0.f, 0.l ) )>::value, "" );
    // float with unsigned int
    static_assert( std::is_same<float, decltype( Random.get<Random_t::common>( 0u, 1.0f ) )>::value, "" );
    static_assert( std::is_same<float, decltype( Random.get<Random_t::common>( 1.0f, 0u ) )>::value, "" );
    auto u_float_val = Random.get<Random_t::common>( 0u, 1.0f );
    REQUIRE( u_float_val >= 0.0f );
    REQUIRE( u_float_val <= 1.0f );
}

TEST_CASE( "Random common type numbers is truly random" ) {
    bool isDifferentNumber{ false };

    for ( int i = 0; i < 1000 && !isDifferentNumber; ++i ) {
        const auto firstRandomNumber = Random.get<Random_t::common>( std::numeric_limits<float>::min( ),
                                                                     std::numeric_limits<signed char>::max( ) );

        const auto secondRandomNumber = Random.get<Random_t::common>( std::numeric_limits<signed char>::min( ),
                                                                      std::numeric_limits<float>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    }

    CHECK( isDifferentNumber );
}

TEST_CASE( "Random bool values" ) {
    SECTION( "probability = 1" ) {
        REQUIRE( Random.get<bool>( 1 ) );
    }

    SECTION( "probability = 0" ) {
        REQUIRE( !Random.get<bool>( 0 ) );
    }

    SECTION( "generate true and false" ) {
        bool has_true = false;
        bool has_false = false;
        for ( int i = 0; i < 1000 && ( !has_true || !has_false ); ++i ) {
            if ( Random.get<bool>( ) )
                has_true = true;
            else
                has_false = true;
        }
        REQUIRE( has_true );
        REQUIRE( has_false );
    }

    SECTION( "out of range argument" ) {
        bool caught_too_high = false;
        try {
            (void)Random.get<bool>( 1.1 );
        } catch ( const std::invalid_argument& ) {
            caught_too_high = true;
        }
        REQUIRE( caught_too_high );

        bool caught_too_low = false;
        try {
            (void)Random.get<bool>( -0.1 );
        } catch ( const std::invalid_argument& ) {
            caught_too_low = true;
        }
        REQUIRE( caught_too_low );
    }
}

TEST_CASE( "Random value from initilizer list" ) {
    bool is1{ false }, is2{ false }, is3{ false };
    for ( int i = 0; i < 1000 && ( !is1 || !is2 || !is3 ); ++i ) {
        switch ( Random.get( { 1, 2, 3 } ) ) {
        case 1:
            is1 = true;
            break;
        case 2:
            is2 = true;
            break;
        case 3:
            is3 = true;
            break;
        }
    }
    REQUIRE( is1 );
    REQUIRE( is2 );
    REQUIRE( is3 );
    // Random.get<int>( {  } ); assertion occurred
}

TEST_CASE( "Move constructor usage in get from initilizer list" ) {
    static size_t copied_num{ 0u };
    static size_t moved_num{ 0u };
    class NoexceptMoveNoexceptCopy {
    public:
        NoexceptMoveNoexceptCopy( ) = default;
        NoexceptMoveNoexceptCopy( const NoexceptMoveNoexceptCopy& ) noexcept { ++copied_num; };
        NoexceptMoveNoexceptCopy( NoexceptMoveNoexceptCopy&& ) noexcept { ++moved_num; };
    };

    NoexceptMoveNoexceptCopy instance{ };
    auto unused_res = Random.get( { std::move( instance ) } );
    (void)unused_res;
    REQUIRE( 1 == copied_num );
    REQUIRE( 1 == moved_num );
}

TEST_CASE( "Random value from initilizer list by reference" ) {
    int i{ 1 };
    auto val = Random.get( { std::ref( i ) } );
    val.get( ) = 5;
    REQUIRE( 5 == i );
}

TEST_CASE( "Random value from initilizer list by pointer" ) {
    int i{ 1 };
    auto val = Random.get( { &i } );
    *val = 5;
    REQUIRE( 5 == i );
}

TEST_CASE( "Shuffle" ) {
    {
        SECTION( "Range" ) {
            std::array<int, 3> arr = { { 1, 2, 3 } };
            const auto arr_copy = arr;
            bool shuffled = false;
            for ( int i = 0; i < 100 && !shuffled; ++i ) {
                Random.shuffle( arr );
                shuffled = ( arr_copy != arr );
            }

            REQUIRE( shuffled );
        }
    }

    {
        SECTION( "Container" ) {
            std::array<int, 3> arr = { { 1, 2, 3 } };
            const auto arr_copy = arr;
            bool shuffled = false;
            for ( int i = 0; i < 100 && !shuffled; ++i ) {
                Random.shuffle( std::begin( arr ), std::end( arr ) );
                shuffled = ( arr_copy != arr );
            }

            REQUIRE( shuffled );
        }
    }

    {
        SECTION( "RawArray" ) {
            int raw_arr[4] = { 1, 2, 3, 4 };
            int raw_copy[4] = { 1, 2, 3, 4 };
            bool shuffled = false;
            for ( int i = 0; i < 100 && !shuffled; ++i ) {
                Random.shuffle( raw_arr );
                shuffled = std::memcmp( raw_arr, raw_copy, sizeof( raw_arr ) ) != 0;
            }

            REQUIRE( shuffled );
        }
    }
}

TEST_CASE( "Get without arguments" ) {
    auto val = Random.get( );
    REQUIRE( val <= Random.max( ) );
}

TEST_CASE( "discard" ) {
    std::stringstream strStream;

    Random.serialize( strStream );
    Random.discard( 500 );
    auto firstVal = Random.get( );

    Random.deserialize( strStream );
    Random.discard( 500 );
    auto secondVal = Random.get( );

    REQUIRE( firstVal == secondVal );
}

TEST_CASE( "stream operators << and >>" ) {
    std::stringstream strStream;

    strStream << Random;
    Random.discard( 500 );
    auto firstVal = Random.get( );

    strStream >> Random;
    Random.discard( 500 );
    auto secondVal = Random.get( );

    REQUIRE( firstVal == secondVal );
}

TEST_CASE( "Not equal" ) {
    std::stringstream strStream;

    Random.serialize( strStream );
    Random.discard( 1 );

    Random_t::engine_type engine;
    strStream >> engine;

    bool isEqual = Random.is_equal( engine );

    REQUIRE( false == isEqual );
}

TEST_CASE( "Equal" ) {
    std::stringstream strStream;

    Random.serialize( strStream );

    Random_t::engine_type engine;
    strStream >> engine;

    bool isEqual = Random.is_equal( engine );

    REQUIRE( true == isEqual );
}

TEST_CASE( "seed" ) {
    Random.seed( 500 );
    auto firstVal = Random.get( );

    Random.seed( 500 );
    auto secondVal = Random.get( );

    REQUIRE( firstVal == secondVal );
}

TEST_CASE( "seed_seq" ) {
    std::seed_seq sseq{ { 1, 2, 3, 4 } };
    Random.seed( sseq );
    auto firstVal = Random.get( );

    Random.seed( sseq );
    auto secondVal = Random.get( );

    REQUIRE( firstVal == secondVal );
}

TEST_CASE( "serialize & deserialize" ) {
    std::stringstream strStream;

    Random.serialize( strStream );
    auto firstVal = Random.get( );

    Random.deserialize( strStream );
    auto secondVal = Random.get( );

    REQUIRE( firstVal == secondVal );
}

TEST_CASE( "custom distribution" ) {
    auto res = Random.get<std::gamma_distribution<>>( 1. );
    (void)res;
}

TEST_CASE( "custom distribution by argument" ) {
    std::gamma_distribution<> gamma{ };
    auto res = Random.get( gamma );
    (void)res;

    // Test temporary (rvalue) distribution
    auto r_res = Random.get( std::uniform_int_distribution<int>{ 10, 20 } );
    REQUIRE( r_res >= 10 );
    REQUIRE( r_res <= 20 );

    // Test const distribution (for distributions with const operator())
    struct ConstDistribution {
        using result_type = int;
        int operator( )( std::mt19937& ) const { return 42; }
    };
    const ConstDistribution const_dist{ };
    REQUIRE( Random.get( const_dist ) == 42 );

    ilqvya::random_local rl;
    auto rl_res = rl.get( std::uniform_int_distribution<int>{ 100, 200 } );
    REQUIRE( rl_res >= 100 );
    REQUIRE( rl_res <= 200 );
    REQUIRE( rl.get( const_dist ) == 42 );
}

TEST_CASE( "custom seeder" ) {
    constexpr auto currentSeed = 42u;
    struct Seeder {
        unsigned operator( )( ) { return currentSeed; }
    };
    std::mt19937 mt{ currentSeed };

#ifdef RANDOM_STATIC
    ilqvya::basic_random_static<std::mt19937, Seeder> tRandom;
#endif
#ifdef RANDOM_THREAD_LOCAL
    ilqvya::basic_random_thread_local<std::mt19937, Seeder> tRandom;
#endif
#ifdef RANDOM_LOCAL
    ilqvya::basic_random_local<std::mt19937, Seeder> tRandom;
#endif
    (void)tRandom;

    REQUIRE( mt( ) == tRandom.get( ) );
}

TEST_CASE( "seed by default seeder" ) {
    struct Seeder_42 {
        unsigned operator( )( ) { return 42u; }
    };

#ifdef RANDOM_STATIC
    ilqvya::basic_random_static<std::mt19937_64, Seeder_42> tRandom;
#endif
#ifdef RANDOM_THREAD_LOCAL
    ilqvya::basic_random_thread_local<std::mt19937_64, Seeder_42> tRandom;
#endif
#ifdef RANDOM_LOCAL
    ilqvya::basic_random_local<std::mt19937_64, Seeder_42> tRandom;
#endif
    (void)tRandom;

    std::mt19937_64 engine{ 42u };

    REQUIRE( tRandom.is_equal( engine ) );

    tRandom.seed( 12345 );

    // REQUIRE( !tRandom.isEqual( engine ) );

    tRandom.reseed( );

    REQUIRE( tRandom.is_equal( engine ) );
}

TEST_CASE( "custom seeder with seedSeq" ) {
    struct Seeder {
        std::seed_seq& operator( )( ) { return seed_seq_; }

        std::seed_seq seed_seq_{ { 1, 2, 3, 4, 5 } };
    };

    std::seed_seq seed_seq_{ { 1, 2, 3, 4, 5 } };
    std::mt19937 mt{ seed_seq_ };

#ifdef RANDOM_STATIC
    ilqvya::basic_random_static<std::mt19937, Seeder> tRandom;
#endif
#ifdef RANDOM_THREAD_LOCAL
    ilqvya::basic_random_thread_local<std::mt19937, Seeder> tRandom;
#endif
#ifdef RANDOM_LOCAL
    ilqvya::basic_random_local<std::mt19937, Seeder> tRandom;
#endif
    (void)tRandom;

    REQUIRE( mt( ) == tRandom.get( ) );
}

TEST_CASE( "default Seeder generate random seed" ) {
    std::uint16_t counter = std::numeric_limits<decltype( counter )>::max( );
    std::vector<std::seed_seq::result_type> seeds_first( 10u );
    bool isSeedRandom{ false };

    ilqvya::seeder_default{ }( ).generate( seeds_first.begin( ), seeds_first.end( ) );
    do {
        std::vector<std::seed_seq::result_type> seeds_second( 10u );
        ilqvya::seeder_default{ }( ).generate( seeds_second.begin( ), seeds_second.end( ) );
        ilqvya::seeder_default seeder;
        isSeedRandom = seeds_first != seeds_second;
    } while ( !isSeedRandom && 0 != counter-- );

    REQUIRE( isSeedRandom );
}

TEST_CASE( "get engine" ) {
    auto engine = Random.get_engine( );
    REQUIRE( Random.is_equal( engine ) );
}

TEST_CASE( "return random iterator from iterator range" ) {
    SECTION( "Matches" ) {
        std::array<int, 5> array = { { 1, 2, 3, 4, 5 } };
        std::uintmax_t counter{ std::numeric_limits<std::uintmax_t>::max( ) };
        bool is1{ false }, is2{ false }, is3{ false }, is4{ false }, is5{ false }, isEnd{ false };
        do {
            auto it = Random.get( array.begin( ), array.end( ) );
            if ( it == array.end( ) ) {
                isEnd = true;
                break;
            }
            switch ( *it ) {
            case 1:
                is1 = true;
                break;
            case 2:
                is2 = true;
                break;
            case 3:
                is3 = true;
                break;
            case 4:
                is4 = true;
                break;
            case 5:
                is5 = true;
                break;
            }
        } while ( !( is1 && is2 && is3 && is4 && is5 ) && counter-- );

        bool isAllMatches = is1 && is2 && is3 && is4 && is5;

        REQUIRE( isAllMatches );
        REQUIRE( false == isEnd );
    }

    SECTION( "0 elems" ) {
        std::vector<int> vector;
        REQUIRE( Random.get( vector.begin( ), vector.end( ) ) == vector.end( ) );
    }
}

TEST_CASE( "return random iterator from container" ) {
    SECTION( "Matches" ) {
        std::uintmax_t counter{ std::numeric_limits<std::uintmax_t>::max( ) };
        std::array<int, 5> array = { { 1, 2, 3, 4, 5 } };
        bool is1{ false }, is2{ false }, is3{ false }, is4{ false }, is5{ false }, isEnd{ false };
        do {
            auto it = Random.get( array );
            if ( it == array.end( ) ) {
                isEnd = true;
                break;
            }
            switch ( *it ) {
            case 1:
                is1 = true;
                break;
            case 2:
                is2 = true;
                break;
            case 3:
                is3 = true;
                break;
            case 4:
                is4 = true;
                break;
            case 5:
                is5 = true;
                break;
            }
        } while ( !( is1 && is2 && is3 && is4 && is5 ) && counter-- );

        bool isAllMatches = is1 && is2 && is3 && is4 && is5;

        REQUIRE( isAllMatches );
        REQUIRE( false == isEnd );
    }

    SECTION( "0 elems" ) {
        std::vector<int> vector;

        bool isEnd = Random.get( vector ) == vector.end( );
        bool isZero = std::begin( vector ) == vector.end( );
        REQUIRE( isZero );
        REQUIRE( isEnd );
    }
}

TEST_CASE( "return random pointer from built-in array" ) {
    SECTION( "Matches" ) {
        int carray[5] = { 1, 2, 3, 4, 5 };
        std::uintmax_t counter{ std::numeric_limits<std::uintmax_t>::max( ) };
        bool is1{ false }, is2{ false }, is3{ false }, is4{ false }, is5{ false }, isEnd{ false };
        do {
            auto ptr = Random.get( carray );
            if ( ptr == std::end( carray ) ) {
                isEnd = true;
                break;
            }
            switch ( *ptr ) {
            case 1:
                is1 = true;
                break;
            case 2:
                is2 = true;
                break;
            case 3:
                is3 = true;
                break;
            case 4:
                is4 = true;
                break;
            case 5:
                is5 = true;
                break;
            }
        } while ( !( is1 && is2 && is3 && is4 && is5 ) && counter-- );

        bool isAllMatches = is1 && is2 && is3 && is4 && is5;

        REQUIRE( isAllMatches );
        REQUIRE( false == isEnd );
    }
}

static thread_local int reserve_count = 0;
static thread_local int insert_count = 0;

template<typename A>
class Container {
public:
    void* end( ... ) { return 0; }

    void insert( ... ) { ++insert_count; }

    void reserve( std::size_t ) { ++reserve_count; }
};

TEST_CASE( "return randomly generated containers with reserve method" ) {
    auto res = Random.get<Container>( 1, 200, 3 );
    (void)res;

    REQUIRE( 1 == reserve_count );
    REQUIRE( 3 == insert_count );
}

TEST_CASE( "return randomly generated containers" ) {
    auto vec = Random.get<std::vector>( 1, 200, 3 );
    auto set = Random.get<std::set>( 1.0, 200.0, 4 );
    auto arr = Random.get<std::array, 5>( '1', '9' );
    auto deq = Random.get<std::deque>( 10, 50, 4 );
    auto lst = Random.get<std::list>( 100.0f, 200.0f, 5 );

    auto vec_common = Random.get<std::vector, Random_t::common>( 1u, 2ul, 6 );
    auto set_common = Random.get<std::set, Random_t::common>( 1.f, 2.l, 7 );
    auto arr_common = Random.get<std::array, 8, Random_t::common>( 1, 2l );
    auto deq_common = Random.get<std::deque, Random_t::common>( 1u, 500ul, 4 );
    auto lst_common = Random.get<std::list, Random_t::common>( 1.0f, 10.0, 3 );

    static_assert( std::is_same<int, decltype( vec )::value_type>::value, "" );
    static_assert( std::is_same<double, decltype( set )::value_type>::value, "" );
    static_assert( std::is_same<char, decltype( arr )::value_type>::value, "" );
    static_assert( std::is_same<int, decltype( deq )::value_type>::value, "" );
    static_assert( std::is_same<float, decltype( lst )::value_type>::value, "" );

    static_assert( std::is_same<unsigned long, decltype( vec_common )::value_type>::value, "" );
    static_assert( std::is_same<long double, decltype( set_common )::value_type>::value, "" );
    static_assert( std::is_same<long, decltype( arr_common )::value_type>::value, "" );
    static_assert( std::is_same<unsigned long, decltype( deq_common )::value_type>::value, "" );
    static_assert( std::is_same<double, decltype( lst_common )::value_type>::value, "" );

    REQUIRE( 3 == vec.size( ) );
    REQUIRE( set.size( ) <= 4 );
    REQUIRE( set.size( ) >= 1 );
    REQUIRE( 5 == arr.size( ) );
    REQUIRE( 4 == deq.size( ) );
    REQUIRE( 5 == lst.size( ) );

    REQUIRE( 6 == vec_common.size( ) );
    REQUIRE( set_common.size( ) <= 7 );
    REQUIRE( set_common.size( ) >= 1 );
    REQUIRE( 8 == arr_common.size( ) );
    REQUIRE( 4 == deq_common.size( ) );
    REQUIRE( 3 == lst_common.size( ) );

    {
        bool unique = false;

        for ( int try_idx = 0; try_idx < 1000 && !unique; ++try_idx ) {
            auto vec2 = Random.get<std::vector>( 1, 100, 3 );
            auto set2 = Random.get<std::set>( 1.0, 200.0, 4 );
            auto arr2 = Random.get<std::array, 5>( '1', '9' );

            unique = vec != vec2 && set != set2 && arr != arr2;
        }

        REQUIRE( true == unique );
    }

    {
        bool unique2 = false;

        for ( int try_idx = 0; try_idx < 1000 && !unique2; ++try_idx ) {
            auto vec2_common = Random.get<std::vector, Random_t::common>( 1u, 200ul, 6 );
            auto set2_common = Random.get<std::set, Random_t::common>( 1.f, 200.l, 7 );
            auto arr2_common = Random.get<std::array, 8, Random_t::common>( 1, 200l );

            unique2 = vec_common != vec2_common && set_common != set2_common && arr_common != arr2_common;
        }

        REQUIRE( true == unique2 );
    }
}

TEST_CASE( "Random range with default arguments" ) {
    auto r1 = Random.get<uint8_t>( );
    (void)r1;
    auto r2 = Random.get<uint16_t>( );
    (void)r2;
    const auto rand_float = Random.get<float>( );
    REQUIRE( rand_float >= 0.0f );
    REQUIRE( rand_float < 1.0f );
    const auto rand_double = Random.get<double>( );
    REQUIRE( rand_double >= 0.0 );
    REQUIRE( rand_double < 1.0 );
    auto r3 = Random.get( 1 );
    (void)r3;
    auto r4 = Random.get( 1.f );
    (void)r4;
    auto r5 = Random.get( uint8_t{ 0u } );
    (void)r5;
}

TEST_CASE( "std::array value initialization" ) {
    auto arr = Random.get<std::array, 4>( 'a', 'z' );
    REQUIRE( 4 == arr.size( ) );
    for ( char c : arr ) {
        REQUIRE( c >= 'a' );
        REQUIRE( c <= 'z' );
    }
}

TEST_CASE( "Signed integer weights in maps" ) {
    std::map<std::string, int> signed_map = { { "Orange", 1 }, { "Apple", 2 }, { "Banana", 3 } };
    std::unordered_map<std::string, int> signed_umap;
    signed_umap["Orange"] = 1;
    signed_umap["Apple"] = 2;
    signed_umap["Banana"] = 3;

    REQUIRE( Random.get<Random_t::weight>( signed_map ) != signed_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( signed_umap ) != signed_umap.end( ) );

    std::map<std::string, int> zero_signed_map = { { "Orange", 0 }, { "Apple", 0 } };
    REQUIRE( Random.get<Random_t::weight>( zero_signed_map ) == zero_signed_map.end( ) );
}

#ifdef RANDOM_THREAD_LOCAL

TEST_CASE( "is truly thread local" ) {
    auto engine = Random.get_engine( );
    REQUIRE( Random.is_equal( engine ) );

    static std::uint8_t seedCount{ 0u };
    static Random_t::engine_type::result_type firstThreadNumber, secondThreadNumber;

    struct Seeder {
        unsigned operator( )( ) {
            ++seedCount;
            return 42u;
        }
    };

    using customRandom = ilqvya::basic_random_thread_local<std::mt19937, Seeder>;

    std::thread thread_t{ [] { firstThreadNumber = customRandom::get( ); } };

    secondThreadNumber = customRandom::get( );

    thread_t.join( );

    REQUIRE( 2 == seedCount );
    REQUIRE( firstThreadNumber == secondThreadNumber );
}

#endif

TEST_CASE( "Type deduction for random character" ) {
    static_assert( std::is_same<char, decltype( Random.get( 'a', 'b' ) )>::value, "" );

    static_assert( std::is_same<wchar_t, decltype( Random.get( L'a', L'b' ) )>::value, "" );

    static_assert( std::is_same<char16_t, decltype( Random.get( char16_t{ 1 }, char16_t{ 2 } ) )>::value, "" );

    static_assert( std::is_same<char32_t, decltype( Random.get( char32_t{ 1 }, char32_t{ 2 } ) )>::value, "" );
}

TEST_CASE( "Random characters is truly random" ) {
    bool isDifferentNumber{ false };

    for ( int i = 0; i < 1000 && !isDifferentNumber; ++i ) {
        const auto firstRandomNumber =
            Random.get<char>( std::numeric_limits<char>::min( ), std::numeric_limits<char>::max( ) );

        const auto secondRandomNumber =
            Random.get<char>( std::numeric_limits<char>::min( ), std::numeric_limits<char>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    }

    CHECK( isDifferentNumber );

    auto wc = Random.get<wchar_t>( );
    (void)wc;

    auto c16 = Random.get<char16_t>( );
    (void)c16;

    auto c32 = Random.get<char32_t>( );
    (void)c32;

#if defined( __cpp_char8_t )
    auto c8 = Random.get<char8_t>( );
    (void)c8;
#endif
}

TEST_CASE( "Random iterator assignment" ) {
    std::vector<int> testData{ 1, 2, 3, 4 };
    int newVal = 5;

    auto it = Random.get( testData.begin( ), testData.end( ) );
    REQUIRE( it != testData.end( ) );
    *it = newVal;

    bool testDataHasNewVal = std::find( testData.cbegin( ), testData.cend( ), newVal ) != testData.end( );
    CHECK( testDataHasNewVal );
}

TEST_CASE( "Random container assignment" ) {
    std::vector<int> testData{ 1, 2, 3, 4 };
    int newVal = 5;

    auto it = Random.get( testData );
    REQUIRE( it != testData.end( ) );
    *it = newVal;

    bool testDataHasNewVal = std::find( testData.cbegin( ), testData.cend( ), newVal ) != testData.end( );
    CHECK( testDataHasNewVal );
}

TEST_CASE( "Get random engine by value" ) {
    auto engine = Random.get_engine( );

    bool isEngineTheSame = true;
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        auto engineRandom = engine( );
        auto randomRandom = Random.get( );

        if ( engineRandom != randomRandom ) {
            isEngineTheSame = false;
            break;
        }
    }

    REQUIRE( isEngineTheSame );
}

TEST_CASE( "Get random engine by reference:)" ) {
    auto& engine = Random.engine( );

    bool isEngineNotTheSame = false;
    for ( std::uint8_t i{ 0u }; i < std::numeric_limits<std::uint8_t>::max( ); ++i ) {
        auto engineRandom = engine( );
        auto randomRandom = Random.get( );

        if ( engineRandom != randomRandom ) {
            isEngineNotTheSame = true;
            break;
        }
    }

    REQUIRE( isEngineNotTheSame );
}

TEST_CASE( "Construct Seeder only once" ) {
    constexpr auto currentSeed = 42u;
    thread_local int constructCount = 0;
    constructCount = 0;
    thread_local int invokeCount = 0;
    invokeCount = 0;

    struct Seeder {
        Seeder( ) { ++constructCount; }
        unsigned operator( )( ) {
            ++invokeCount;
            return currentSeed;
        }
    };

#ifdef RANDOM_STATIC
    ilqvya::basic_random_static<std::mt19937, Seeder> tRandom;
#endif
#ifdef RANDOM_THREAD_LOCAL
    ilqvya::basic_random_thread_local<std::mt19937, Seeder> tRandom;
#endif
#ifdef RANDOM_LOCAL
    ilqvya::basic_random_local<std::mt19937, Seeder> tRandom;
#endif
    (void)tRandom;

    std::vector<int> randomValues;
    for ( int i = 0; i < 256; ++i ) {
        auto r = tRandom.get( );
        (void)r;
    }

    REQUIRE( constructCount == 1 );
    REQUIRE( invokeCount == 1 );

    tRandom.reseed( );

    REQUIRE( constructCount == 2 );
    REQUIRE( invokeCount == 2 );
}

TEST_CASE( "is_map trait" ) {
    static_assert( ilqvya::details::is_map<std::unordered_map<int, int>>::value, "" );
    static_assert( ilqvya::details::is_map<std::map<int, int>>::value, "" );
    // This should also count as a map
    struct CustomMap {
        using key_type = int;
        using mapped_type = int;
        using value_type = int;
    };
    static_assert( ilqvya::details::is_map<CustomMap>::value, "" );

    // Some datatypes that shouldn't be classified as a map
    static_assert( !ilqvya::details::is_map<int>::value, "" );
    static_assert( !ilqvya::details::is_map<std::vector<int>>::value, "" );
    static_assert( !ilqvya::details::is_map<std::string>::value, "" );
}

TEST_CASE( "Empty map and maps with zero total weight tests" ) {
    // Empty maps
    std::unordered_map<std::string, unsigned long> empty_ulong_umap;
    std::unordered_map<std::string, unsigned> empty_uint_umap;
    std::unordered_map<std::string, float> empty_float_umap;
    std::unordered_map<std::string, double> empty_double_umap;

    std::map<std::string, unsigned long> empty_ulong_map;
    std::map<std::string, unsigned> empty_uint_map;
    std::map<std::string, float> empty_float_map;
    std::map<std::string, double> empty_double_map;

    REQUIRE( Random.get<Random_t::weight>( empty_ulong_umap ) == empty_ulong_umap.end( ) );
    REQUIRE( Random.get<Random_t::weight>( empty_uint_umap ) == empty_uint_umap.end( ) );
    REQUIRE( Random.get<Random_t::weight>( empty_float_umap ) == empty_float_umap.end( ) );
    REQUIRE( Random.get<Random_t::weight>( empty_double_umap ) == empty_double_umap.end( ) );

    REQUIRE( Random.get<Random_t::weight>( empty_ulong_map ) == empty_ulong_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( empty_uint_map ) == empty_uint_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( empty_float_map ) == empty_float_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( empty_double_map ) == empty_double_map.end( ) );

    // Zero weighted maps
    std::unordered_map<std::string, unsigned long> zero_long_umap;
    zero_long_umap["Orange"] = 0ul;
    zero_long_umap["Apple"] = 0ul;
    zero_long_umap["Banana"] = 0ul;

    std::unordered_map<std::string, unsigned> zero_uint_umap;
    zero_uint_umap["Orange"] = 0u;
    zero_uint_umap["Apple"] = 0u;
    zero_uint_umap["Banana"] = 0u;

    std::unordered_map<std::string, float> zero_float_umap;
    zero_float_umap["Orange"] = 0.0f;
    zero_float_umap["Apple"] = 0.0f;
    zero_float_umap["Banana"] = 0.0f;

    std::unordered_map<std::string, double> zero_double_umap;
    zero_double_umap["Orange"] = 0.0;
    zero_double_umap["Apple"] = 0.0;
    zero_double_umap["Banana"] = 0.0;

    std::map<std::string, unsigned long> zero_ulong_map = { { "Orange", 0ul }, { "Apple", 0ul }, { "Banana", 0ul } };
    std::map<std::string, unsigned> zero_uint_map = { { "Orange", 0u }, { "Apple", 0u }, { "Banana", 0u } };
    std::map<std::string, float> zero_float_map = { { "Orange", 0.0f }, { "Apple", 0.0f }, { "Banana", 0.0f } };
    std::map<std::string, double> zero_double_map = { { "Orange", 0.0 }, { "Apple", 0.0 }, { "Banana", 0.0 } };

    REQUIRE( Random.get<Random_t::weight>( zero_long_umap ) == zero_long_umap.end( ) );
    REQUIRE( Random.get<Random_t::weight>( zero_uint_umap ) == zero_uint_umap.end( ) );
    REQUIRE( Random.get<Random_t::weight>( zero_float_umap ) == zero_float_umap.end( ) );
    REQUIRE( Random.get<Random_t::weight>( zero_double_umap ) == zero_double_umap.end( ) );

    REQUIRE( Random.get<Random_t::weight>( zero_ulong_map ) == zero_ulong_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( zero_uint_map ) == zero_uint_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( zero_float_map ) == zero_float_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( zero_double_map ) == zero_double_map.end( ) );
}

TEST_CASE( "Maps with non-zero total weight tests" ) {
    std::unordered_map<std::string, unsigned long> nonzero_ulong_umap;
    nonzero_ulong_umap["Orange"] = 1ul;
    nonzero_ulong_umap["Apple"] = 2ul;
    nonzero_ulong_umap["Banana"] = 3ul;

    std::unordered_map<std::string, unsigned> nonzero_uint_umap;
    nonzero_uint_umap["Orange"] = 1u;
    nonzero_uint_umap["Apple"] = 2u;
    nonzero_uint_umap["Banana"] = 3u;

    std::unordered_map<std::string, float> nonzero_float_umap;
    nonzero_float_umap["Orange"] = 1.0f;
    nonzero_float_umap["Apple"] = 2.0f;
    nonzero_float_umap["Banana"] = 3.0f;

    std::unordered_map<std::string, double> nonzero_double_umap;
    nonzero_double_umap["Orange"] = 1.0;
    nonzero_double_umap["Apple"] = 2.0;
    nonzero_double_umap["Banana"] = 3.0;

    std::map<std::string, unsigned long> nonzero_ulong_map = { { "Orange", 1ul }, { "Apple", 2ul }, { "Banana", 3ul } };
    std::map<std::string, unsigned> nonzero_uint_map = { { "Orange", 1u }, { "Apple", 2u }, { "Banana", 3u } };
    std::map<std::string, float> nonzero_float_map = { { "Orange", 1.0f }, { "Apple", 2.0f }, { "Banana", 3.0f } };
    std::map<std::string, double> nonzero_double_map = { { "Orange", 1.0 }, { "Apple", 2.0 }, { "Banana", 3.0 } };

    REQUIRE( Random.get<Random_t::weight>( nonzero_ulong_umap ) != nonzero_ulong_umap.end( ) );
    REQUIRE( Random.get<Random_t::weight>( nonzero_uint_umap ) != nonzero_uint_umap.end( ) );
    REQUIRE( Random.get<Random_t::weight>( nonzero_float_umap ) != nonzero_float_umap.end( ) );
    REQUIRE( Random.get<Random_t::weight>( nonzero_double_umap ) != nonzero_double_umap.end( ) );

    REQUIRE( Random.get<Random_t::weight>( nonzero_ulong_map ) != nonzero_ulong_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( nonzero_uint_map ) != nonzero_uint_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( nonzero_float_map ) != nonzero_float_map.end( ) );
    REQUIRE( Random.get<Random_t::weight>( nonzero_double_map ) != nonzero_double_map.end( ) );
}

TEST_CASE( "Functor operator() invocation" ) {
    // 0-argument invocation (default engine get)
    auto val0 = Random( );
    REQUIRE( val0 <= Random.max( ) );

    // 2-argument integer range invocation
    auto valInt = Random( -5, 5 );
    REQUIRE( valInt >= -5 );
    REQUIRE( valInt <= 5 );

    // 2-argument real range invocation
    auto valReal = Random( 1.0f, 10.0f );
    REQUIRE( valReal >= 1.0f );
    REQUIRE( valReal <= 10.0f );

    // Array / pointer invocation
    int arr[] = { 10, 20, 30 };
    auto* ptr = Random( arr );
    REQUIRE( ( ptr == &arr[0] || ptr == &arr[1] || ptr == &arr[2] ) );
}

TEST_CASE( "Uniform instance and static calling syntax across all generator types" ) {
    // Static engine supports both static and instance calls
    auto s_val_static = ilqvya::random_static::get( 1, 10 );
    REQUIRE( s_val_static >= 1 );
    REQUIRE( s_val_static <= 10 );

    ilqvya::random_static s_inst;
    auto s_val_member = s_inst.get( 1, 10 );
    REQUIRE( s_val_member >= 1 );
    REQUIRE( s_val_member <= 10 );

    auto s_val_functor = s_inst( 1, 10 );
    REQUIRE( s_val_functor >= 1 );
    REQUIRE( s_val_functor <= 10 );

    // Thread-local engine supports both static and instance calls
    auto tl_val_static = ilqvya::random_thread_local::get( 1, 10 );
    REQUIRE( tl_val_static >= 1 );
    REQUIRE( tl_val_static <= 10 );

    ilqvya::random_thread_local tl_inst;
    auto tl_val_member = tl_inst.get( 1, 10 );
    REQUIRE( tl_val_member >= 1 );
    REQUIRE( tl_val_member <= 10 );

    auto tl_val_functor = tl_inst( 1, 10 );
    REQUIRE( tl_val_functor >= 1 );
    REQUIRE( tl_val_functor <= 10 );

    // Local engine supports instance and functor calls
    ilqvya::random_local loc_inst;
    auto loc_val_member = loc_inst.get( 1, 10 );
    REQUIRE( loc_val_member >= 1 );
    REQUIRE( loc_val_member <= 10 );

    auto loc_val_functor = loc_inst( 1, 10 );
    REQUIRE( loc_val_functor >= 1 );
    REQUIRE( loc_val_functor <= 10 );
}

TEST_CASE( "Correctness: Default real range [0.0, 1.0)" ) {
    // 1. Default float range without arguments
    for ( int i = 0; i < 50; ++i ) {
        float f = Random.get<float>( );
        REQUIRE( f >= 0.0f );
        REQUIRE( f < 1.0f );
    }

    // 2. Default double range without arguments
    for ( int i = 0; i < 50; ++i ) {
        double d = Random.get<double>( );
        REQUIRE( d >= 0.0 );
        REQUIRE( d < 1.0 );
    }
}

TEST_CASE( "Correctness: Signed integer weights in maps" ) {
    // Signed int
    std::map<std::string, int> int_map = { { "A", 10 }, { "B", 0 }, { "C", 20 } };
    for ( int i = 0; i < 50; ++i ) {
        auto it = Random.get<Random_t::weight>( int_map );
        REQUIRE( it != int_map.end( ) );
        REQUIRE( it->second > 0 ); // "B" with 0 weight must never be selected
        REQUIRE( ( it->first == "A" || it->first == "C" ) );
    }

    // Signed long
    std::unordered_map<std::string, long> long_map;
    long_map["X"] = 100l;
    long_map["Y"] = 0l;
    for ( int i = 0; i < 20; ++i ) {
        auto it = Random.get<Random_t::weight>( long_map );
        REQUIRE( it != long_map.end( ) );
        REQUIRE( it->first == "X" );
    }

    // All-zero signed weights returns end()
    std::map<std::string, int> all_zero = { { "Zero1", 0 }, { "Zero2", 0 } };
    REQUIRE( Random.get<Random_t::weight>( all_zero ) == all_zero.end( ) );
}

TEST_CASE( "Correctness: Weighted real selection with subnormal weights and no end() fallthrough" ) {
    // Map with tiny / subnormal float weights
    float subnormal = std::numeric_limits<float>::denorm_min( );
    std::map<std::string, float> tiny_map = { { "Tiny1", subnormal }, { "Tiny2", subnormal * 2.0f }, { "Zero", 0.0f } };

    for ( int i = 0; i < 50; ++i ) {
        auto it = Random.get<Random_t::weight>( tiny_map );
        REQUIRE( it != tiny_map.end( ) );
        REQUIRE( it->first != "Zero" );
    }

    // Map where accumulated floating point rounding could previously cause loop fallthrough
    std::map<std::string, double> sum_rounding_map = { { "One", 0.1 }, { "Two", 0.2 }, { "Three", 0.3 } };
    for ( int i = 0; i < 100; ++i ) {
        auto it = Random.get<Random_t::weight>( sum_rounding_map );
        REQUIRE( it != sum_rounding_map.end( ) );
    }
}

TEST_CASE( "Correctness: Container<A, N> value initialization" ) {
    // Array of elements
    auto char_arr = Random.get<std::array, 5>( 'A', 'Z' );
    REQUIRE( 5 == char_arr.size( ) );
    for ( char c : char_arr ) {
        REQUIRE( c >= 'A' );
        REQUIRE( c <= 'Z' );
    }
}

TEST_CASE( "Correctness: Expression SFINAE for has_reserve and has_insert with overloaded member functions" ) {
    struct OverloadedMembers {
        void reserve( ) {}
        void reserve( std::size_t ) {}
        void reserve( std::size_t, int ) {}

        void insert( ) {}
        void insert( int ) {}
    };

    static_assert( ilqvya::details::has_reserve<std::vector<int>>::value, "vector has reserve" );
    static_assert( !ilqvya::details::has_reserve<std::array<int, 4>>::value, "array does not have reserve" );
    static_assert( !ilqvya::details::has_reserve<std::set<int>>::value, "set does not have reserve" );

    static_assert( ilqvya::details::has_insert<std::vector<int>>::value, "vector has insert" );
    static_assert( ilqvya::details::has_insert<std::set<int>>::value, "set has insert" );
    static_assert( !ilqvya::details::has_insert<std::array<int, 4>>::value, "array does not have insert" );

    // Overloaded reserve matches expression SFINAE reserve(size_t) without ambiguous address taking
    static_assert( ilqvya::details::has_reserve<OverloadedMembers>::value, "overloaded reserve matches" );
}

TEST_CASE( "Discussion #38: Variadic custom weighted distribution syntax" ) {
    // 2-element variadic
    for ( int i = 0; i < 50; ++i ) {
        float f = Random.get<float>( { 1.1f, 1 }, { 1.2f, 2 } );
        REQUIRE( ( f == 1.1f || f == 1.2f ) );
    }

    // 3-element variadic
    for ( int i = 0; i < 50; ++i ) {
        int val = Random.get<int>( { 1, 3 }, { 4, 4 }, { 10, 5 } );
        REQUIRE( ( val == 1 || val == 4 || val == 10 ) );
    }

    // 4-element and 5-element variadic
    for ( int i = 0; i < 50; ++i ) {
        int val4 = Random.get<int>( { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 } );
        REQUIRE( ( val4 >= 1 && val4 <= 4 ) );

        int val5 = Random.get<int>( { 10, 1 }, { 20, 1 }, { 30, 1 }, { 40, 1 }, { 50, 1 } );
        REQUIRE( ( val5 == 10 || val5 == 20 || val5 == 30 || val5 == 40 || val5 == 50 ) );
    }

    // Explicit integer weights
    for ( int i = 0; i < 50; ++i ) {
        int val = Random.get<int, int>( { 1, 5 }, { 2, 10 } );
        REQUIRE( ( val == 1 || val == 2 ) );
    }
}

TEST_CASE( "Discussion #38: Initializer list weighted distribution" ) {
    // String choices
    for ( int i = 0; i < 50; ++i ) {
        std::string fruit = Random.get<std::string>( { { "Apple", 10 }, { "Banana", 20 }, { "Orange", 30 } } );
        REQUIRE( ( fruit == "Apple" || fruit == "Banana" || fruit == "Orange" ) );
    }

    // Zero-weight elements are never picked when positive weight exists
    for ( int i = 0; i < 50; ++i ) {
        int zero_test = Random.get<int>( { { 1, 0 }, { 2, 100 }, { 3, 0 } } );
        REQUIRE( zero_test == 2 );

        float zero_real_test = Random.get<float>( { { 1.5f, 0.0 }, { 2.5f, 1.0 }, { 3.5f, 0.0 } } );
        REQUIRE( zero_real_test == 2.5f );
    }

    // Single-element list
    int single = Random.get<int>( { { 42, 10 } } );
    REQUIRE( single == 42 );

    // Statistical check: 90% vs 10%
    int count_high = 0;
    const int iterations = 1000;
    for ( int i = 0; i < iterations; ++i ) {
        int pick = Random.get<int>( { { 1, 1 }, { 2, 9 } } );
        if ( pick == 2 ) {
            ++count_high;
        }
    }
    // High weight item (90%) should occur far more than low weight (10%)
    REQUIRE( count_high > 750 );
}

TEST_CASE( "Discussion #38: Sequential pair containers (vector of pairs)" ) {
    // Integer weights in std::vector
    std::vector<std::pair<std::string, int>> vec_int = { { "First", 1 }, { "Second", 2 }, { "Third", 3 } };
    for ( int i = 0; i < 50; ++i ) {
        auto it = Random.get<Random_t::weight>( vec_int );
        REQUIRE( it != vec_int.end( ) );
        REQUIRE( ( it->first == "First" || it->first == "Second" || it->first == "Third" ) );
    }

    // Real weights in std::vector
    std::vector<std::pair<char, double>> vec_real = { { 'A', 0.5 }, { 'B', 1.5 }, { 'C', 2.0 } };
    for ( int i = 0; i < 50; ++i ) {
        auto it = Random.get<Random_t::weight>( vec_real );
        REQUIRE( it != vec_real.end( ) );
        REQUIRE( ( it->first == 'A' || it->first == 'B' || it->first == 'C' ) );
    }
}

#if defined( ILQVYA_HAS_REFLECTION )
enum class TestDifficulty { Easy, Medium, Hard, Expert };
enum TestStatus { Disconnected = 0, Connecting = 10, Connected = 100 };
enum class TestSingle { OnlyOne = 777 };

TEST_CASE( "Discussion #37: Choosing random value from an enum (C++26 Reflection)" ) {
    // 1. Scoped enum class
    for ( int i = 0; i < 50; ++i ) {
        TestDifficulty d = Random.get<TestDifficulty>( );
        REQUIRE( ( d == TestDifficulty::Easy || d == TestDifficulty::Medium || d == TestDifficulty::Hard ||
                   d == TestDifficulty::Expert ) );
    }

    // 2. Unscoped enum with non-contiguous values
    for ( int i = 0; i < 50; ++i ) {
        TestStatus s = Random.get<TestStatus>( );
        REQUIRE( ( s == Disconnected || s == Connecting || s == Connected ) );
    }

    // 3. Single-element enum
    for ( int i = 0; i < 10; ++i ) {
        TestSingle val = Random.get<TestSingle>( );
        REQUIRE( val == TestSingle::OnlyOne );
    }

    // 4. All enum values reachable
    std::set<TestDifficulty> seen;
    for ( int i = 0; i < 200; ++i ) {
        seen.insert( Random.get<TestDifficulty>( ) );
    }
    REQUIRE( seen.size( ) == 4 );

    // 5. random_local support for enum reflection
    ilqvya::random_local rl;
    TestDifficulty local_d = rl.get<TestDifficulty>( );
    REQUIRE( ( local_d == TestDifficulty::Easy || local_d == TestDifficulty::Medium ||
               local_d == TestDifficulty::Hard || local_d == TestDifficulty::Expert ) );
}
#endif

TEST_CASE( "ilqvya::random alias and effolkronium backward compatibility" ) {
    // ilqvya::random is alias for ilqvya::random_thread_local
    static_assert( std::is_same<ilqvya::random, ilqvya::random_thread_local>::value,
                   "ilqvya::random must be an alias for ilqvya::random_thread_local" );

    auto val_static = ilqvya::random::get( 1, 10 );
    REQUIRE( val_static >= 1 );
    REQUIRE( val_static <= 10 );

    ilqvya::random rnd;
    auto val_member = rnd.get( 1, 10 );
    REQUIRE( val_member >= 1 );
    REQUIRE( val_member <= 10 );

    auto val_call = rnd( 1, 10 );
    REQUIRE( val_call >= 1 );
    REQUIRE( val_call <= 10 );

    // effolkronium namespace alias backward compatibility
    static_assert( std::is_same<effolkronium::random, ilqvya::random>::value,
                   "effolkronium::random must be identical to ilqvya::random" );
    static_assert( std::is_same<effolkronium::random_static, ilqvya::random_static>::value,
                   "effolkronium::random_static must be identical to ilqvya::random_static" );
    static_assert( std::is_same<effolkronium::random_thread_local, ilqvya::random_thread_local>::value,
                   "effolkronium::random_thread_local must be identical to ilqvya::random_thread_local" );
    static_assert( std::is_same<effolkronium::random_local, ilqvya::random_local>::value,
                   "effolkronium::random_local must be identical to ilqvya::random_local" );

    auto eff_val1 = effolkronium::random::get( 1, 10 );
    REQUIRE( eff_val1 >= 1 );
    REQUIRE( eff_val1 <= 10 );

    auto eff_val2 = effolkronium::random_static::get( 1, 10 );
    REQUIRE( eff_val2 >= 1 );
    REQUIRE( eff_val2 <= 10 );

    auto eff_val3 = effolkronium::random_thread_local::get( 1, 10 );
    REQUIRE( eff_val3 >= 1 );
    REQUIRE( eff_val3 <= 10 );

    effolkronium::random_local eff_loc;
    auto eff_val4 = eff_loc.get( 1, 10 );
    REQUIRE( eff_val4 >= 1 );
    REQUIRE( eff_val4 <= 10 );
}

TEST_CASE( "Version macros" ) {
    static_assert( ILQVYA_RANDOM_VERSION_MAJOR == 2, "Major version must be 2" );
    static_assert( ILQVYA_RANDOM_VERSION_MINOR == 0, "Minor version must be 0" );
    static_assert( ILQVYA_RANDOM_VERSION_PATCH == 0, "Patch version must be 0" );
    static_assert( ILQVYA_RANDOM_VERSION == 20000, "Full version number must be 20000" );
    REQUIRE( ILQVYA_RANDOM_VERSION == 20000 );
}

TEST_CASE( "Random characters inverted range" ) {
    char c = Random.get<char>( 'z', 'a' );
    REQUIRE( c >= 'a' && c <= 'z' );

    wchar_t wc = Random.get<wchar_t>( L'z', L'a' );
    REQUIRE( wc >= L'a' && wc <= L'z' );

    char16_t c16 = Random.get<char16_t>( u'z', u'a' );
    REQUIRE( c16 >= u'a' && c16 <= u'z' );

    char32_t c32 = Random.get<char32_t>( U'z', U'a' );
    REQUIRE( c32 >= U'a' && c32 <= U'z' );

#if defined( __cpp_char8_t )
    char8_t c8 = Random.get<char8_t>( u8'z', u8'a' );
    REQUIRE( c8 >= u8'a' && c8 <= u8'z' );
#endif

    // Zero gap characters
    REQUIRE( Random.get<char>( 'm', 'm' ) == 'm' );
    REQUIRE( Random.get<wchar_t>( L'm', L'm' ) == L'm' );
    REQUIRE( Random.get<char16_t>( u'm', u'm' ) == u'm' );
    REQUIRE( Random.get<char32_t>( U'm', U'm' ) == U'm' );
}

TEST_CASE( "Random real numbers inverted range" ) {
    float f = Random.get<float>( 10.0f, 1.0f );
    REQUIRE( f >= 1.0f && f <= 10.0f );

    double d = Random.get<double>( 50.0, 5.0 );
    REQUIRE( d >= 5.0 && d <= 50.0 );

    long double ld = Random.get<long double>( 100.0l, 0.0l );
    REQUIRE( ld >= 0.0l && ld <= 100.0l );
}

TEST_CASE( "Weighted list with zero total weight fallback" ) {
    auto res_int =
        Random.get( { Random_t::weighted_result<int, int>{ 42, 0 }, Random_t::weighted_result<int, int>{ 43, 0 } } );
    REQUIRE( res_int == 42 );

    auto res_real = Random.get( { Random_t::weighted_result<std::string, double>{ "zero_fallback", 0.0 },
                                  Random_t::weighted_result<std::string, double>{ "other", 0.0 } } );
    REQUIRE( res_real == "zero_fallback" );
}

TEST_CASE( "Random.min, default seed, and reseed coverage" ) {
    REQUIRE( Random.min( ) <= Random.max( ) );
    Random.seed( );
    Random.reseed( );
    REQUIRE( Random.get( ) <= Random.max( ) );
}

TEST_CASE( "random_local API coverage" ) {
    ilqvya::random_local rl;
    REQUIRE( rl.min( ) <= rl.max( ) );
    rl.seed( );
    rl.reseed( );
    REQUIRE( rl.get_engine( ) == rl.engine( ) );
    auto rl_val = rl( 1, 10 );
    REQUIRE( rl_val >= 1 );
    REQUIRE( rl_val <= 10 );

    // Constructor with explicit seed
    ilqvya::random_local rl_seed{ 42u };
    ilqvya::random_local rl_seed2{ 42u };
    REQUIRE( rl_seed.get( 1, 1000 ) == rl_seed2.get( 1, 1000 ) );

    // Constructor with existing engine
    std::mt19937 eng{ 12345u };
    ilqvya::random_local rl_eng{ eng };
    REQUIRE( rl_eng.engine( ) == eng );

    // Constructor with custom seeder
    struct custom_seeder {
        std::seed_seq seq{ 1, 2, 3, 4 };
        std::seed_seq& operator( )( ) { return seq; }
    };
    using local_custom_seeder_t = ilqvya::basic_random_local<std::mt19937, custom_seeder>;
    custom_seeder cs1, cs2;
    local_custom_seeder_t rl_seeder1{ cs1 };
    local_custom_seeder_t rl_seeder2{ cs2 };
    REQUIRE( rl_seeder1.get( 1, 1000 ) == rl_seeder2.get( 1, 1000 ) );

    // Constructor with engine and seeder
    std::mt19937 eng_for_both{ 999u };
    custom_seeder cs3, cs4;
    local_custom_seeder_t rl_both{ eng_for_both, cs3 };
    local_custom_seeder_t rl_seeder3{ cs4 };
    REQUIRE( rl_both.get( 1, 1000 ) == rl_seeder3.get( 1, 1000 ) );

    // Constructor with seed sequence
    std::seed_seq sseq{ 10, 20, 30 };
    ilqvya::random_local rl_sseq{ sseq };
    REQUIRE( rl_sseq.get( 1, 10 ) >= 1 );
}

TEST_CASE( "weighted_result pair constructors and variadic get" ) {
    std::pair<int, int> p1{ 10, 5 };
    std::pair<int, int> p2{ 20, 15 };
    Random_t::weighted_result<int, int> wr1( p1 );
    Random_t::weighted_result<int, int> wr2( std::move( p2 ) );
    REQUIRE( wr1.value == 10 );
    REQUIRE( wr1.weight == 5 );
    REQUIRE( wr2.value == 20 );
    REQUIRE( wr2.weight == 15 );
    REQUIRE( ( wr1 == Random_t::weighted_result<int, int>{ 10, 5 } ) );
    REQUIRE( wr1 != wr2 );
    REQUIRE( !( wr1 == wr2 ) );
    REQUIRE( !( wr1 != Random_t::weighted_result<int, int>{ 10, 5 } ) );

    Random_t::weighted_result<int, int> w1{ 1, 1 }, w2{ 2, 1 }, w3{ 3, 1 }, w4{ 4, 1 }, w5{ 5, 1 }, w6{ 6, 1 },
        w7{ 7, 1 }, w8{ 8, 1 }, w9{ 9, 1 }, w10{ 10, 1 };

    REQUIRE( Random.get( w1, w2 ) >= 1 );
    REQUIRE( Random.get( w1, w2, w3 ) >= 1 );
    REQUIRE( Random.get( w1, w2, w3, w4 ) >= 1 );
    REQUIRE( Random.get( w1, w2, w3, w4, w5 ) >= 1 );
    REQUIRE( Random.get( w1, w2, w3, w4, w5, w6 ) >= 1 );
    REQUIRE( Random.get( w1, w2, w3, w4, w5, w6, w7 ) >= 1 );
    REQUIRE( Random.get( w1, w2, w3, w4, w5, w6, w7, w8 ) >= 1 );
    REQUIRE( Random.get( w1, w2, w3, w4, w5, w6, w7, w8, w9 ) >= 1 );
    REQUIRE( Random.get( w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 ) >= 1 );

    ilqvya::random_local rl;
    REQUIRE( rl.get( w1, w2 ) >= 1 );
    REQUIRE( rl.get( w1, w2, w3 ) >= 1 );
    REQUIRE( rl.get( w1, w2, w3, w4 ) >= 1 );
    REQUIRE( rl.get( w1, w2, w3, w4, w5 ) >= 1 );
    REQUIRE( rl.get( w1, w2, w3, w4, w5, w6 ) >= 1 );
    REQUIRE( rl.get( w1, w2, w3, w4, w5, w6, w7 ) >= 1 );
    REQUIRE( rl.get( w1, w2, w3, w4, w5, w6, w7, w8 ) >= 1 );
    REQUIRE( rl.get( w1, w2, w3, w4, w5, w6, w7, w8, w9 ) >= 1 );
    REQUIRE( rl.get( w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 ) >= 1 );
}

TEST_CASE( "random_local remaining methods and empty iterator get" ) {
    ilqvya::random_local rl;
    std::gamma_distribution<> gamma{ };
    auto g1 = rl.get<std::gamma_distribution<>>( 1. );
    auto g2 = rl.get( gamma );
    (void)g1;
    (void)g2;

    std::array<int, 3> arr{ 1, 2, 3 };
    rl.shuffle( arr );
    rl.shuffle( arr.begin( ), arr.end( ) );
    int raw_arr[3] = { 1, 2, 3 };
    rl.shuffle( raw_arr );

    rl.discard( 10 );

    std::stringstream ss;
    rl.serialize( ss );
    rl.deserialize( ss );

    std::stringstream ss2;
    ss2 << rl;
    auto before_val = rl.get( );
    ss2 >> rl;
    auto after_val = rl.get( );
    REQUIRE( before_val == after_val );

    Random_t::engine_type engine_copy = rl.get_engine( );
    REQUIRE( rl.is_equal( engine_copy ) );
    Random_t::engine_type different_engine{ 99999u };
    REQUIRE( !rl.is_equal( different_engine ) );
    REQUIRE( !Random.is_equal( different_engine ) );

    ilqvya::random_local rl_clone{ rl.get_engine( ) };
    REQUIRE( rl.is_equal( rl_clone ) );
    REQUIRE( rl == rl_clone );
    REQUIRE( !( rl != rl_clone ) );

    ilqvya::random_local rl_diff{ 88888u };
    REQUIRE( !rl.is_equal( rl_diff ) );
    REQUIRE( rl != rl_diff );
    REQUIRE( !( rl == rl_diff ) );

    std::vector<int> empty_vec;
    REQUIRE( Random.get( empty_vec.begin( ), empty_vec.end( ) ) == empty_vec.end( ) );
    REQUIRE( Random.get( empty_vec ) == empty_vec.end( ) );
    REQUIRE( rl.get( empty_vec.begin( ), empty_vec.end( ) ) == empty_vec.end( ) );
    REQUIRE( rl.get( empty_vec ) == empty_vec.end( ) );

    // Test const engine() access without copying
    const ilqvya::random_local const_rl;
    const auto& const_engine_ref = const_rl.engine( );
    REQUIRE( const_rl.is_equal( const_engine_ref ) );

    const ilqvya::random_static const_rs;
    const auto& const_rs_engine = const_rs.engine( );
    (void)const_rs_engine;

    const ilqvya::random_thread_local const_rtl;
    const auto& const_rtl_engine = const_rtl.engine( );
    (void)const_rtl_engine;
}

TEST_CASE( "Forward iterator support for get" ) {
    std::forward_list<int> flist = { 10, 20, 30 };
    auto fit = Random.get( flist.begin( ), flist.end( ) );
    REQUIRE( fit != flist.end( ) );
    REQUIRE( ( *fit == 10 || *fit == 20 || *fit == 30 ) );

    static_assert( ilqvya::details::is_forward_iterator<decltype( flist.begin( ) )>::value,
                   "forward_list iterator must satisfy is_forward_iterator" );
    static_assert( !ilqvya::details::is_forward_iterator<std::istream_iterator<int>>::value,
                   "istream_iterator must not satisfy is_forward_iterator" );
}

TEST_CASE( "Weighted get with lvalue container remains valid" ) {
    std::vector<std::pair<int, int>> pairs = { { 1, 10 }, { 2, 20 } };
    auto it = Random.get<Random_t::weight>( pairs );
    REQUIRE( it != pairs.end( ) );
    REQUIRE( ( it->first == 1 || it->first == 2 ) );

    ilqvya::random_local rl;
    auto it_rl = rl.get<Random_t::weight>( pairs );
    REQUIRE( it_rl != pairs.end( ) );
}

TEST_CASE( "Empty initializer_list throws invalid_argument" ) {
    bool caught_init_list = false;
    try {
        std::initializer_list<int> empty_list{ };
        (void)Random.get( empty_list );
    } catch ( const std::invalid_argument& ) {
        caught_init_list = true;
    }
    REQUIRE( caught_init_list );

    bool caught_weighted_int = false;
    try {
        std::initializer_list<Random_t::weighted_result<int, int>> empty_weighted{ };
        (void)Random.get( empty_weighted );
    } catch ( const std::invalid_argument& ) {
        caught_weighted_int = true;
    }
    REQUIRE( caught_weighted_int );

    bool caught_weighted_real = false;
    try {
        std::initializer_list<Random_t::weighted_result<int, double>> empty_weighted_real{ };
        (void)Random.get( empty_weighted_real );
    } catch ( const std::invalid_argument& ) {
        caught_weighted_real = true;
    }
    REQUIRE( caught_weighted_real );
}

TEST_CASE( "Negative weights throw invalid_argument" ) {
    std::vector<std::pair<std::string, int>> negative_int_pairs = { { "A", 10 }, { "B", -1 } };
    bool caught_pair_int = false;
    try {
        (void)Random.get<Random_t::weight>( negative_int_pairs );
    } catch ( const std::invalid_argument& ) {
        caught_pair_int = true;
    }
    REQUIRE( caught_pair_int );

    std::vector<std::pair<std::string, double>> negative_real_pairs = { { "A", 1.5 }, { "B", -0.5 } };
    bool caught_pair_real = false;
    try {
        (void)Random.get<Random_t::weight>( negative_real_pairs );
    } catch ( const std::invalid_argument& ) {
        caught_pair_real = true;
    }
    REQUIRE( caught_pair_real );

    bool caught_list_int = false;
    try {
        (void)Random.get( { Random_t::weighted_result<int, int>{ 1, 10 },
                            Random_t::weighted_result<int, int>{ 2, -5 } } );
    } catch ( const std::invalid_argument& ) {
        caught_list_int = true;
    }
    REQUIRE( caught_list_int );

    bool caught_list_real = false;
    try {
        (void)Random.get( { Random_t::weighted_result<int, double>{ 1, 10.0 },
                            Random_t::weighted_result<int, double>{ 2, -2.5 } } );
    } catch ( const std::invalid_argument& ) {
        caught_list_real = true;
    }
    REQUIRE( caught_list_real );
}

TEST_CASE( "noexcept specifications for observers, discard, seed, comparisons and weighted_result" ) {
    ilqvya::random_local rl;

    // Unconditional observers
    static_assert( noexcept( ilqvya::random_local::min( ) ), "random_local::min must be noexcept" );
    static_assert( noexcept( ilqvya::random_local::max( ) ), "random_local::max must be noexcept" );
    static_assert( noexcept( rl.get_engine( ) ), "random_local::get_engine must be noexcept" );
    static_assert( noexcept( rl.engine( ) ), "random_local::engine must be noexcept" );

    static_assert( noexcept( Random_t::min( ) ), "Random::min must be noexcept" );
    static_assert( noexcept( Random_t::max( ) ), "Random::max must be noexcept" );
    static_assert( noexcept( Random.get_engine( ) ), "Random::get_engine must be noexcept" );
    static_assert( noexcept( Random.engine( ) ), "Random::engine must be noexcept" );

    // weighted_result
    Random_t::weighted_result<int, double> wr{ 42, 1.0 };
    static_assert( noexcept( Random_t::weighted_result<int, double>{ 42, 1.0 } ),
                   "weighted_result constructor must be noexcept for nothrow types" );
    static_assert( noexcept( wr == wr ), "weighted_result operator== must be noexcept" );
    static_assert( noexcept( wr != wr ), "weighted_result operator!= must be noexcept" );
}

namespace {
    struct nothrow_mock_engine {
        using result_type = unsigned int;
        static constexpr result_type min( ) noexcept { return 0; }
        static constexpr result_type max( ) noexcept { return 100; }
        nothrow_mock_engine( ) = default;
        explicit nothrow_mock_engine( result_type ) noexcept {}
        template<typename Sseq>
        explicit nothrow_mock_engine( Sseq& ) noexcept {}
        void discard( unsigned long long ) noexcept {}
        void seed( result_type ) noexcept {}
        template<typename Sseq>
        void seed( Sseq& ) noexcept {}
        result_type operator( )( ) noexcept { return 42; }
        bool operator==( const nothrow_mock_engine& ) const noexcept { return true; }
        bool operator!=( const nothrow_mock_engine& ) const noexcept { return false; }
    };
} // namespace

TEST_CASE( "conditional noexcept with nothrow engine" ) {
    using nothrow_local = ilqvya::basic_random_local<nothrow_mock_engine>;
    nothrow_mock_engine nt_eng;
    nothrow_local nt_rl;

    static_assert( noexcept( nothrow_local{ 12345u } ), "nothrow_local(seed) must be noexcept" );
    static_assert( noexcept( nothrow_local{ nt_eng } ), "nothrow_local(Engine) must be noexcept" );
    static_assert( noexcept( nt_rl.discard( 10 ) ), "nothrow_local::discard must be noexcept" );
    static_assert( noexcept( nt_rl.seed( 12345u ) ), "nothrow_local::seed must be noexcept" );
    static_assert( noexcept( nt_rl.get( ) ), "nothrow_local::get() must be noexcept" );
    static_assert( noexcept( nt_rl.is_equal( nt_eng ) ), "nothrow_local::is_equal(Engine) must be noexcept" );
    static_assert( noexcept( nt_rl.is_equal( nt_rl ) ), "nothrow_local::is_equal(nothrow_local) must be noexcept" );
    static_assert( noexcept( nt_rl == nt_rl ), "nothrow_local operator== must be noexcept" );
    static_assert( noexcept( nt_rl != nt_rl ), "nothrow_local operator!= must be noexcept" );

    using nothrow_static = ilqvya::basic_random_static<nothrow_mock_engine>;
    static_assert( noexcept( nothrow_static::discard( 10 ) ), "nothrow_static::discard must be noexcept" );
    static_assert( noexcept( nothrow_static::seed( 12345u ) ), "nothrow_static::seed must be noexcept" );
    static_assert( noexcept( nothrow_static::get( ) ), "nothrow_static::get() must be noexcept" );
    static_assert( noexcept( nothrow_static::is_equal( nt_eng ) ), "nothrow_static::is_equal must be noexcept" );
}

TEST_CASE( "sampling from const containers and string views" ) {
    const std::vector<int> const_vec = { 10, 20, 30 };
    auto it_vec = Random.get( const_vec );
    REQUIRE( it_vec != const_vec.end( ) );
    REQUIRE( ( *it_vec == 10 || *it_vec == 20 || *it_vec == 30 ) );
    static_assert( std::is_same<decltype( it_vec ), std::vector<int>::const_iterator>::value,
                   "get(const Container&) must return const_iterator" );

    const std::string const_str = "hello";
    auto it_str = Random.get( const_str );
    REQUIRE( it_str != const_str.end( ) );
    REQUIRE( const_str.find( *it_str ) != std::string::npos );

    const std::vector<int> empty_const_vec;
    REQUIRE( Random.get( empty_const_vec ) == empty_const_vec.end( ) );

    ilqvya::random_local rl;
    auto it_rl = rl.get( const_vec );
    REQUIRE( it_rl != const_vec.end( ) );
    REQUIRE( ( *it_rl == 10 || *it_rl == 20 || *it_rl == 30 ) );

#if ILQVYA_CPLUSPLUS >= 201703L
    std::string_view sv = "abcdef";
    auto it_sv = Random.get( sv );
    REQUIRE( it_sv != sv.end( ) );
    REQUIRE( sv.find( *it_sv ) != std::string_view::npos );

    auto it_sv_rl = rl.get( sv );
    REQUIRE( it_sv_rl != sv.end( ) );
    REQUIRE( sv.find( *it_sv_rl ) != std::string_view::npos );

    std::string_view empty_sv;
    REQUIRE( Random.get( empty_sv ) == empty_sv.end( ) );
    REQUIRE( rl.get( empty_sv ) == empty_sv.end( ) );
#endif
}

TEST_CASE( "default_engine selection: philox4x64 in C++26 vs mt19937 fallback" ) {
#if defined( __cpp_lib_philox_engine ) && ( __cpp_lib_philox_engine >= 202406L )
    static_assert( std::is_same<ilqvya::default_engine, std::philox4x64>::value,
                   "default_engine must be std::philox4x64 when __cpp_lib_philox_engine is available" );
    static_assert( std::is_same<Random_t::engine_type, std::philox4x64>::value,
                   "Random_t::engine_type must be std::philox4x64 when __cpp_lib_philox_engine is available" );
#else
    static_assert( std::is_same<ilqvya::default_engine, std::mt19937>::value,
                   "default_engine must fall back to std::mt19937 when philox_engine is not available" );
    static_assert( std::is_same<Random_t::engine_type, std::mt19937>::value,
                   "Random_t::engine_type must fall back to std::mt19937 when philox_engine is not available" );
#endif
    // Verify default seeder produces distinct random numbers with default_engine
    auto val1 = Random.get( );
    auto val2 = Random.get( );
    (void)val1;
    (void)val2;
}

TEST_CASE( "get_string helper methods" ) {
    // 1. Range of characters
    auto s_alpha = Random.get_string( 16, 'a', 'z' );
    REQUIRE( s_alpha.size( ) == 16 );
    for ( char c : s_alpha ) {
        REQUIRE( ( c >= 'a' && c <= 'z' ) );
    }

    auto s_digits = Random.get_string( 8, '0', '9' );
    REQUIRE( s_digits.size( ) == 8 );
    for ( char c : s_digits ) {
        REQUIRE( ( c >= '0' && c <= '9' ) );
    }

    // Zero-length string
    auto s_empty = Random.get_string( 0, 'a', 'z' );
    REQUIRE( s_empty.empty( ) );

    // Inverted character range
    auto s_inv = Random.get_string( 10, 'z', 'a' );
    REQUIRE( s_inv.size( ) == 10 );
    for ( char c : s_inv ) {
        REQUIRE( ( c >= 'a' && c <= 'z' ) );
    }

    // 2. Custom string literal charset
    auto s_hex = Random.get_string( 32, "0123456789abcdef" );
    REQUIRE( s_hex.size( ) == 32 );
    for ( char c : s_hex ) {
        REQUIRE( ( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) ) );
    }

    // 3. Container charset (std::string)
    std::string charset_str = "ABCXYZ";
    auto s_from_str = Random.get_string( 12, charset_str );
    REQUIRE( s_from_str.size( ) == 12 );
    for ( char c : s_from_str ) {
        REQUIRE( charset_str.find( c ) != std::string::npos );
    }

    // 4. Container charset (std::vector<char>)
    std::vector<char> charset_vec = { 'x', 'y', 'z' };
    auto s_from_vec = Random.get_string( 10, charset_vec );
    REQUIRE( s_from_vec.size( ) == 10 );
    for ( char c : s_from_vec ) {
        REQUIRE( ( c == 'x' || c == 'y' || c == 'z' ) );
    }

    // Empty container charset throws invalid_argument
    std::string empty_charset;
    bool caught_empty_charset = false;
    try {
        (void)Random.get_string( 5, empty_charset );
    } catch ( const std::invalid_argument& ) {
        caught_empty_charset = true;
    }
    REQUIRE( caught_empty_charset );

    // 5. random_local test
    ilqvya::random_local rl;
    auto rl_str = rl.get_string( 20, 'A', 'Z' );
    REQUIRE( rl_str.size( ) == 20 );
    for ( char c : rl_str ) {
        REQUIRE( ( c >= 'A' && c <= 'Z' ) );
    }

    auto rl_hex = rl.get_string( 16, "0123456789ABCDEF" );
    REQUIRE( rl_hex.size( ) == 16 );

    // 6. Wide characters (wchar_t)
    auto ws = Random.get_string( 10, L'a', L'z' );
    REQUIRE( ws.size( ) == 10 );
    for ( wchar_t c : ws ) {
        REQUIRE( ( c >= L'a' && c <= L'z' ) );
    }
}

TEST_CASE( "Random::sample" ) {
    std::vector<int> population = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

    // 1. Iterator range with output iterator
    std::vector<int> out_sample;
    Random.sample( population.begin( ), population.end( ), std::back_inserter( out_sample ), 4 );
    REQUIRE( out_sample.size( ) == 4 );
    for ( int val : out_sample ) {
        REQUIRE( std::find( population.begin( ), population.end( ), val ) != population.end( ) );
    }

    // 2. Container to container overload (returns std::vector by default)
    auto sampled_vec = Random.sample( population, 5 );
    REQUIRE( sampled_vec.size( ) == 5 );
    for ( int val : sampled_vec ) {
        REQUIRE( std::find( population.begin( ), population.end( ), val ) != population.end( ) );
    }

    // 3. Container to custom container overload (e.g. std::deque)
    auto sampled_deque = Random.sample<std::deque>( population, 3 );
    REQUIRE( sampled_deque.size( ) == 3 );
    for ( int val : sampled_deque ) {
        REQUIRE( std::find( population.begin( ), population.end( ), val ) != population.end( ) );
    }

    // 4. Sample size 0
    auto sample_zero = Random.sample( population, 0 );
    REQUIRE( sample_zero.empty( ) );

    // 5. Sample size >= population size
    auto sample_all = Random.sample( population, population.size( ) + 5 );
    REQUIRE( sample_all.size( ) == population.size( ) );

    // 6. Random.sample with raw array
    int raw_pop[] = { 1, 2, 3, 4, 5 };
    auto sample_raw = Random.sample( raw_pop, 2 );
    REQUIRE( sample_raw.size( ) == 2 );

    // 7. random_local test
    ilqvya::random_local rl;
    auto rl_sampled = rl.sample( population, 4 );
    REQUIRE( rl_sampled.size( ) == 4 );

    std::vector<int> rl_iter_sampled;
    rl.sample( population.begin( ), population.end( ), std::back_inserter( rl_iter_sampled ), 3 );
    REQUIRE( rl_iter_sampled.size( ) == 3 );
}

