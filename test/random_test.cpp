#include "test_framework.hpp"
#include <limits>
#include <sstream>
#include <array>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>

#ifdef _WIN32 // Unit test for case when builds fail 'cause of min\max macro included from Windows.h before random.hpp
#    include "Windows.h"
#endif

#include "ilqvya/random.hpp"
#include "effolkronium/random.hpp"

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
    auto count = 1000;

    do {

        const auto firstRandomNumber =
            Random.get( std::numeric_limits<long long>::min( ), std::numeric_limits<long long>::max( ) );

        const auto secondRandomNumber =
            Random.get( std::numeric_limits<long long>::min( ), std::numeric_limits<long long>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    } while ( !isDifferentNumber && count-- );

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
    auto count = std::numeric_limits<std::uintmax_t>::min( );

    do {
        const auto firstRandomNumber =
            Random.get( std::numeric_limits<float>::min( ), std::numeric_limits<float>::max( ) );

        const auto secondRandomNumber =
            Random.get( std::numeric_limits<float>::min( ), std::numeric_limits<float>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    } while ( !isDifferentNumber && count-- );

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
}

TEST_CASE( "Random byte numbers is truly random" ) {
    bool isDifferentNumber{ false };
    auto count = 1000;

    do {
        const auto firstRandomNumber =
            Random.get( std::numeric_limits<signed char>::min( ), std::numeric_limits<signed char>::max( ) );

        const auto secondRandomNumber =
            Random.get( std::numeric_limits<signed char>::min( ), std::numeric_limits<signed char>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    } while ( !isDifferentNumber && count-- );

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
}

TEST_CASE( "Random common type numbers is truly random" ) {
    bool isDifferentNumber{ false };
    auto count = 1000;

    do {

        const auto firstRandomNumber = Random.get<Random_t::common>( std::numeric_limits<float>::min( ),
                                                                     std::numeric_limits<signed char>::max( ) );

        const auto secondRandomNumber = Random.get<Random_t::common>( std::numeric_limits<signed char>::min( ),
                                                                      std::numeric_limits<float>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    } while ( !isDifferentNumber && count-- );

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
        // Random.get<bool>( 1.1 );
        // Random.get<bool>( -1.1 );
        // assert occurred!
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

    auto vec_common = Random.get<std::vector, Random_t::common>( 1u, 2ul, 6 );
    auto set_common = Random.get<std::set, Random_t::common>( 1.f, 2.l, 7 );
    auto arr_common = Random.get<std::array, 8, Random_t::common>( 1, 2l );

    static_assert( std::is_same<int, decltype( vec )::value_type>::value, "" );
    static_assert( std::is_same<double, decltype( set )::value_type>::value, "" );
    static_assert( std::is_same<char, decltype( arr )::value_type>::value, "" );

    static_assert( std::is_same<unsigned long, decltype( vec_common )::value_type>::value, "" );
    static_assert( std::is_same<long double, decltype( set_common )::value_type>::value, "" );
    static_assert( std::is_same<long, decltype( arr_common )::value_type>::value, "" );

    REQUIRE( 3 == vec.size( ) );
    REQUIRE( set.size( ) <= 4 );
    REQUIRE( set.size( ) >= 1 );
    REQUIRE( 5 == arr.size( ) );

    REQUIRE( 6 == vec_common.size( ) );
    REQUIRE( set_common.size( ) <= 7 );
    REQUIRE( set_common.size( ) >= 1 );
    REQUIRE( 8 == arr_common.size( ) );

    {
        bool unique = false;

        auto try_count = 1000;
        while ( try_count-- > 0 ) {
            auto vec2 = Random.get<std::vector>( 1, 100, 3 );
            auto set2 = Random.get<std::set>( 1.0, 200.0, 4 );
            auto arr2 = Random.get<std::array, 5>( '1', '9' );

            unique = vec != vec2 && set != set2 && arr != arr2;

            if ( unique )
                break;
        }

        REQUIRE( true == unique );
    }

    {
        bool unique2 = false;

        auto try_count = 1000;
        while ( try_count-- > 0 ) {
            auto vec2_common = Random.get<std::vector, Random_t::common>( 1u, 200ul, 6 );
            auto set2_common = Random.get<std::set, Random_t::common>( 1.f, 200.l, 7 );
            auto arr2_common = Random.get<std::array, 8, Random_t::common>( 1, 200l );

            unique2 = vec_common != vec2_common && set_common != set2_common && arr_common != arr2_common;

            if ( unique2 )
                break;
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
    std::unordered_map<std::string, int> signed_umap = { { "Orange", 1 }, { "Apple", 2 }, { "Banana", 3 } };

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
    auto count = 1000;

    do {

        const auto firstRandomNumber =
            Random.get<char>( std::numeric_limits<char>::min( ), std::numeric_limits<char>::max( ) );

        const auto secondRandomNumber =
            Random.get<char>( std::numeric_limits<char>::min( ), std::numeric_limits<char>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    } while ( !isDifferentNumber && count-- );

    CHECK( isDifferentNumber );
}

TEST_CASE( "Random iterator assignment" ) {
    std::vector<int> testData{ 1, 2, 3, 4 };
    int newVal = 5;

    *( Random.get( testData.begin( ), testData.end( ) ) ) = newVal;

    bool testDataHasNewVal = std::find( testData.cbegin( ), testData.cend( ), newVal ) != testData.end( );
    CHECK( testDataHasNewVal );
}

TEST_CASE( "Random container assignment" ) {
    std::vector<int> testData{ 1, 2, 3, 4 };
    int newVal = 5;

    *( Random.get( testData ) ) = newVal;

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
    std::unordered_map<std::string, unsigned long> zero_long_umap = {
        { "Orange", 0ul }, { "Apple", 0ul }, { "Banana", 0ul } };
    std::unordered_map<std::string, unsigned> zero_uint_umap = { { "Orange", 0u }, { "Apple", 0u }, { "Banana", 0u } };
    std::unordered_map<std::string, float> zero_float_umap = {
        { "Orange", 0.0f }, { "Apple", 0.0f }, { "Banana", 0.0f } };
    std::unordered_map<std::string, double> zero_double_umap = {
        { "Orange", 0.0 }, { "Apple", 0.0 }, { "Banana", 0.0 } };

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
    std::unordered_map<std::string, unsigned long> nonzero_ulong_umap = {
        { "Orange", 1ul }, { "Apple", 2ul }, { "Banana", 3ul } };
    std::unordered_map<std::string, unsigned> nonzero_uint_umap = {
        { "Orange", 1u }, { "Apple", 2u }, { "Banana", 3u } };
    std::unordered_map<std::string, float> nonzero_float_umap = {
        { "Orange", 1.0f }, { "Apple", 2.0f }, { "Banana", 3.0f } };
    std::unordered_map<std::string, double> nonzero_double_umap = {
        { "Orange", 1.0 }, { "Apple", 2.0 }, { "Banana", 3.0 } };

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
    std::unordered_map<std::string, long> long_map = { { "X", 100l }, { "Y", 0l } };
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

#if defined( ILQVYA_HAS_REFLECTION )
    static_assert( EFFOLKRONIUM_HAS_REFLECTION == 1,
                   "EFFOLKRONIUM_HAS_REFLECTION macro must be defined when ILQVYA_HAS_REFLECTION is defined" );
#endif
}
