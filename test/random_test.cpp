#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <limits>
#include <sstream>
#include <array>
#include <thread>
#include <vector>

#include "current_random.hpp"

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

    do {

        const auto firstRandomNumber = Random DOT get(
            std::numeric_limits<long long>::min( ),
            std::numeric_limits<long long>::max( ) );

        const auto secondRandomNumber = Random DOT get(
            std::numeric_limits<long long>::min( ),
            std::numeric_limits<long long>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    } while ( !isDifferentNumber );

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
    auto count = std::numeric_limits<std::uintmax_t>::min( );

    do {
        const auto firstRandomNumber = Random DOT get(
            std::numeric_limits<float>::min( ),
            std::numeric_limits<float>::max( ) );

        const auto secondRandomNumber = Random DOT get(
            std::numeric_limits<float>::min( ),
            std::numeric_limits<float>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    } while( !isDifferentNumber && count-- );

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

    do {
        const auto firstRandomNumber = Random DOT get(
            std::numeric_limits<signed char>::min( ),
            std::numeric_limits<signed char>::max( ) );

        const auto secondRandomNumber = Random DOT get(
            std::numeric_limits<signed char>::min( ),
            std::numeric_limits<signed char>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    } while( !isDifferentNumber );

    CHECK( isDifferentNumber );
}

TEST_CASE( "Range overflow for random common_type numbers" ) {
    bool isRangeOverflow = false;

    // From lower to greater
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( )
         && !isRangeOverflow;
         ++i ) {
        const auto randomNumber = Random DOT get<Random_t::common>(
                                                 static_cast<signed char>( -1 ),
                                                 short{ 1 } );

        isRangeOverflow = randomNumber < -1 || randomNumber > 1;
    }
    REQUIRE( !isRangeOverflow );

    // From greater to lower
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get<Random_t::common>(
                                           1ull, 0u );

        isRangeOverflow = 1 < randomNumber;
    }
    REQUIRE( !isRangeOverflow );

    // Range with 0 gap
    for( std::uint8_t i{ 0u };
         i < std::numeric_limits<std::uint8_t>::max( );
         ++i ) {
        const auto randomNumber = Random DOT get<Random_t::common>(
                                                static_cast<signed char>( 0 ),
                                                0ll );
        isRangeOverflow = randomNumber != 0;
    }
    REQUIRE( !isRangeOverflow );
}

TEST_CASE( "Type deduction for random common_type numbers" ) {
    // signed char
    static_assert( std::is_same<signed char,
                   decltype( Random DOT get<Random_t::common>(
                       static_cast<signed char>( 0 ),
                       static_cast<signed char>( 0 ) ) ) > ::value, "" );
    // unsigned long
    static_assert( std::is_same<unsigned long,
                   decltype( Random DOT get<Random_t::common>(
                       static_cast<unsigned char>( 0u ), 0ul ) ) > ::value, "" );
    // double
    static_assert( std::is_same<double,
                   decltype( Random DOT get<Random_t::common>(
                       0.,
                       std::int8_t{ 0 } ) ) > ::value, "" );
    // long double
    static_assert( std::is_same<long double,
                   decltype( Random DOT get<Random_t::common>(
                       0.f, 0.l ) ) > ::value, "" );
}

TEST_CASE( "Random common type numbres is truly random" ) {
    bool isDifferentNumber{ false };

    do {

        const auto firstRandomNumber = Random DOT get<Random_t::common>(
            std::numeric_limits<float>::min( ),
            std::numeric_limits<signed char>::max( ) );

        const auto secondRandomNumber = Random DOT get<Random_t::common>(
            std::numeric_limits<signed char>::min( ),
            std::numeric_limits<float>::max( ) );

        isDifferentNumber = firstRandomNumber != secondRandomNumber;
    } while( !isDifferentNumber );

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

TEST_CASE( "Random value from initilizer list" ) {
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

TEST_CASE( "Move constructor usage in get from initilizer list" ) { 
    static size_t copied_num{ 0u };
    static size_t moved_num{ 0u };
    class NoexceptMoveNoexceptCopy {
    public:
        NoexceptMoveNoexceptCopy( ) = default;
        NoexceptMoveNoexceptCopy(
            const NoexceptMoveNoexceptCopy& ) noexcept  { 
            ++copied_num;
        };
        NoexceptMoveNoexceptCopy(
            NoexceptMoveNoexceptCopy&& ) noexcept {
            ++moved_num;
        };
    };

    NoexceptMoveNoexceptCopy instance{ };
    Random DOT get( { std::move( instance ) } );
    REQUIRE( 1 == copied_num );
    REQUIRE( 1 == moved_num );
}

TEST_CASE( "Random value from initilizer list by reference" ) {
    int i{ 1 };
    auto val = Random DOT get( { std::ref( i ) } );
    val.get( ) = 5;
    REQUIRE( 5 == i );
}

TEST_CASE( "Random value from initilizer list by pointer" ) {
    int i{ 1 };
    auto val = Random DOT get( { &i } );
    *val = 5;
    REQUIRE( 5 == i );
}

TEST_CASE( "Shuffle" ) {
    {
        SECTION( "Range" ) {
            std::array<int, 3> arr = { { 1, 2, 3 } };
            const auto arr_copy = arr;
            do {
                Random DOT shuffle( arr );
            } while( arr_copy == arr );

            REQUIRE( true == true );
        }
    }

    {
        SECTION( "Container" ) {
            std::array<int, 3> arr = { { 1, 2, 3 } };
            const auto arr_copy = arr;
            do {
                Random DOT shuffle( std::begin( arr ), std::end( arr ) );
            } while( arr_copy == arr );

            REQUIRE( true == true );
        }
    }
}

TEST_CASE( "Get without arguments" ) {
    auto val = Random DOT get( );
    REQUIRE( val <= Random DOT max( ) );
}

TEST_CASE( "discard" ) {
    std::stringstream strStream;

    Random DOT serialize( strStream );
    Random DOT discard( 500 );
    auto firstVal = Random DOT get( );

    Random DOT deserialize( strStream );
    Random DOT discard( 500 );
    auto secondVal = Random DOT get( );

    REQUIRE( firstVal == secondVal );
}

TEST_CASE( "Not equal" ) {
    std::stringstream strStream;

    Random DOT serialize( strStream );
    Random DOT discard( 1 );

    Random_t::engine_type engine;
    strStream >> engine;

    bool isEqual = Random DOT is_equal( engine );

    REQUIRE( false == isEqual );
}

TEST_CASE( "Equal" ) {
    std::stringstream strStream;

    Random DOT serialize( strStream );

    Random_t::engine_type engine;
    strStream >> engine;

    bool isEqual = Random DOT is_equal( engine );

    REQUIRE( true == isEqual );
}

TEST_CASE( "seed" ) {
    Random DOT seed( 500 );
    auto firstVal = Random DOT get( );

    Random DOT seed( 500 );
    auto secondVal = Random DOT get( );

    REQUIRE( firstVal == secondVal );
}

TEST_CASE( "seed_seq" ) {
    std::seed_seq sseq{ {1,2,3,4} };
    Random DOT seed( sseq );
    auto firstVal = Random DOT get( );

    Random DOT seed( sseq );
    auto secondVal = Random DOT get( );

    REQUIRE( firstVal == secondVal );
}

TEST_CASE( "serialize & deserialize" ) {
    std::stringstream strStream;
    
    Random DOT serialize( strStream );
    auto firstVal = Random DOT get( );
    
    Random DOT deserialize( strStream );
    auto secondVal = Random DOT get( );
    
    REQUIRE( firstVal == secondVal );
}

TEST_CASE( "custom distribution" ) {
    Random DOT get<std::gamma_distribution<>>( 1. );
}

TEST_CASE( "custom distribution by argument" ) {
    std::gamma_distribution<> gamma{ };
    Random DOT get( gamma );
}

TEST_CASE( "custom seeder" ) {
    constexpr auto currentSeed = 42u;
    struct Seeder {
        unsigned operator() ( ) {
            return currentSeed;
        }
    };
    std::mt19937 mt{ currentSeed };

    #ifdef RANDOM_STATIC

    using tRandom = effolkronium::basic_random_static<std::mt19937, Seeder>;

    #endif
    #ifdef RANDOM_THREAD_LOCAL

    using tRandom = effolkronium::basic_random_thread_local<std::mt19937, Seeder>;

    #endif
    #ifdef RANDOM_LOCAL

    effolkronium::basic_random_local<std::mt19937, Seeder> tRandom;

    #endif

    REQUIRE( mt( ) == tRandom DOT get( ) );
}

TEST_CASE( "seed by default seeder" ) { 
    struct Seeder_42 {
        unsigned operator() ( ) {
            return 42u;
        }
    };

    #ifdef RANDOM_STATIC

    using tRandom = effolkronium::basic_random_static<std::mt19937_64, Seeder_42>;

    #endif
    #ifdef RANDOM_THREAD_LOCAL

    using tRandom = effolkronium::basic_random_thread_local<std::mt19937_64, Seeder_42>;

    #endif
    #ifdef RANDOM_LOCAL

    effolkronium::basic_random_local<std::mt19937_64, Seeder_42> tRandom;

    #endif

    std::mt19937_64 engine{ 42u };
    
    REQUIRE( tRandom DOT is_equal( engine ) );

    tRandom DOT seed( 12345 );
    
    //REQUIRE( !tRandom DOT isEqual( engine ) );

    tRandom DOT reseed( );

    REQUIRE( tRandom DOT is_equal( engine ) );
}

TEST_CASE( "custom seeder with seedSeq" ) {
    struct Seeder {
        std::seed_seq& operator() ( ) {
            return seed_seq_;
        }

        std::seed_seq seed_seq_{ { 1, 2, 3, 4, 5 } };
    };

    std::seed_seq seed_seq_{ { 1, 2, 3, 4, 5 } };
    std::mt19937 mt{ seed_seq_ };

    #ifdef RANDOM_STATIC

    using tRandom = effolkronium::basic_random_static<std::mt19937, Seeder>;

    #endif
    #ifdef RANDOM_THREAD_LOCAL

    using tRandom = effolkronium::basic_random_thread_local<std::mt19937, Seeder>;

    #endif
    #ifdef RANDOM_LOCAL

    effolkronium::basic_random_local<std::mt19937, Seeder> tRandom;

    #endif

    REQUIRE( mt( ) == tRandom DOT get( ) );
}

TEST_CASE( "default Seeder generate random seed" ) {
    std::uint16_t counter = std::numeric_limits<decltype( counter )>::max( );
    std::vector<std::seed_seq::result_type> seeds_first( 10u );
    bool isSeedRandom{ false };

    effolkronium::seeder_default{ }( ).generate( seeds_first.begin( ),
                                                 seeds_first.end( ) );
    do {
        std::vector<std::seed_seq::result_type> seeds_second( 10u );
        effolkronium::seeder_default{ }( ).generate( seeds_second.begin( ),
                                                     seeds_second.end( ) );
        effolkronium::seeder_default seeder;
        isSeedRandom = seeds_first != seeds_second;
    } while( !isSeedRandom && 0 != counter-- );

    REQUIRE( isSeedRandom );
}

TEST_CASE( "get engine" ) {
    auto engine = Random DOT get_engine( );
    REQUIRE( Random DOT is_equal( engine ) );
}

TEST_CASE( "return random iterator from iterator range" ) {
    SECTION( "Matches" ) {
        std::array<int, 5> array = { { 1, 2, 3, 4, 5 } };
        std::uintmax_t counter{ std::numeric_limits<std::uintmax_t>::max( ) };
        bool is1{ false }, is2{ false }, is3{ false },
            is4{ false }, is5{ false }, isEnd{ false };
        do {
            auto it = Random DOT get( array.begin( ), array.end( ) );
            if( it == array.end( ) ) {
                isEnd = true;
                break;
            }
            switch( *it ) {
                case 1: is1 = true; break;
                case 2: is2 = true; break;
                case 3: is3 = true; break;
                case 4: is4 = true; break;
                case 5: is5 = true; break;
            }
        } while( !( is1 && is2 && is3 && is4 && is5 ) && counter--);

        bool isAllMatches = is1 && is2 && is3 && is4 && is5;

        REQUIRE( isAllMatches );
        REQUIRE( false == isEnd );
    }

    SECTION( "0 elems" ) { 
        std::vector<int> vector;
        REQUIRE( Random DOT get( vector.begin( ), vector.end( ) )
                 == vector.end( ) );
    }
}

TEST_CASE( "return random iterator from container" ) {
    SECTION( "Matches" ) {
        std::uintmax_t counter{ std::numeric_limits<std::uintmax_t>::max( ) };
        std::array<int, 5> array = { { 1, 2, 3, 4, 5 } };
        bool is1{ false }, is2{ false }, is3{ false },
            is4{ false }, is5{ false }, isEnd{ false };
        do {
            auto it = Random DOT get( array );
            if( it == array.end( ) ) {
                isEnd = true;
                break;
            }
            switch( *it ) {
                case 1: is1 = true; break;
                case 2: is2 = true; break;
                case 3: is3 = true; break;
                case 4: is4 = true; break;
                case 5: is5 = true; break;
            }
        } while( !( is1 && is2 && is3 && is4 && is5 ) && counter-- );

        bool isAllMatches = is1 && is2 && is3 && is4 && is5;

        REQUIRE( isAllMatches );
        REQUIRE( false == isEnd );
    }

    SECTION( "0 elems" ) {
        std::vector<int> vector;

        bool isEnd = Random DOT get( vector ) == vector.end( );
        bool isZero = std::begin( vector ) == vector.end( );
        REQUIRE( isZero );
        REQUIRE( isEnd );
    }
}

TEST_CASE( "return random pointer from built-in array" ) {
    SECTION( "Matches" ) {
		int carray [5] = { 1, 2, 3, 4, 5 };
        std::uintmax_t counter{ std::numeric_limits<std::uintmax_t>::max( ) };
        bool is1{ false }, is2{ false }, is3{ false },
            is4{ false }, is5{ false }, isEnd{ false };
        do {
            auto ptr = Random DOT get( carray );
            if( ptr == std::end( carray ) ) {
                isEnd = true;
                break;
            }
            switch( *ptr ) {
                case 1: is1 = true; break;
                case 2: is2 = true; break;
                case 3: is3 = true; break;
                case 4: is4 = true; break;
                case 5: is5 = true; break;
            }
        } while( !( is1 && is2 && is3 && is4 && is5 ) && counter--);

        bool isAllMatches = is1 && is2 && is3 && is4 && is5;

        REQUIRE( isAllMatches );
        REQUIRE( false == isEnd );
    }
}

TEST_CASE( "Random range with default arguments" ) {
    Random DOT get<uint8_t>( );
    Random DOT get<uint16_t>( );
    Random DOT get<float>( );
    Random DOT get( 1 );
    Random DOT get( 1.f );
    Random DOT get( uint8_t{ 0u } );
}

#ifdef RANDOM_THREAD_LOCAL

TEST_CASE( "is truly thread local" ) {
    auto engine = Random DOT get_engine( );
    REQUIRE( Random DOT is_equal( engine ) );

    static std::uint8_t seedCount{ 0u };
    static Random::engine_type::result_type 
        firstThreadNumber, secondThreadNumber;

    struct Seeder {
        unsigned operator() ( ) {
            ++seedCount;
            return 42u;
        }
    };

    using customRandom =
        effolkronium::basic_random_thread_local<std::mt19937, Seeder>;

    std::thread thread_t{ [ ] {
        firstThreadNumber = customRandom::get( );
    } };

    secondThreadNumber = customRandom::get( );

    thread_t.join( );

    REQUIRE( 2 == seedCount );
    REQUIRE( firstThreadNumber == secondThreadNumber );
}

#endif

TEST_CASE("Type deduction for random character") {
	static_assert(std::is_same<char,
		decltype(Random DOT get(
			'a', 'b')) > ::value, "");

	static_assert(std::is_same<wchar_t,
		decltype(Random DOT get(
			L'a', L'b')) > ::value, "");

	static_assert(std::is_same<char16_t,
		decltype(Random DOT get(
			char16_t{ 1 }, char16_t{ 2 })) > ::value, "");

	static_assert(std::is_same<char32_t,
		decltype(Random DOT get(
			char32_t{ 1 }, char32_t{ 2 })) > ::value, "");
}

TEST_CASE("Random characters is truly random") {
	bool isDifferentNumber{ false };

	do {

		const auto firstRandomNumber = Random DOT get<char>(
			std::numeric_limits<char>::min(),
			std::numeric_limits<char>::max());

		const auto secondRandomNumber = Random DOT get<char>(
			std::numeric_limits<char>::min(),
			std::numeric_limits<char>::max());

		isDifferentNumber = firstRandomNumber != secondRandomNumber;
	} while (!isDifferentNumber);

	CHECK(isDifferentNumber);
}

TEST_CASE("Random iterator assigment") {
    std::vector<int> testData{ 1,2,3,4 };
    int newVal = 5;

    *(Random DOT get(testData.begin(), testData.end())) = newVal;

    bool testDataHasNewVal = std::find(testData.cbegin(), testData.cend(), newVal) != testData.end();
    CHECK(testDataHasNewVal);
}

TEST_CASE("Random container assigment") {
    std::vector<int> testData{ 1,2,3,4 };
    int newVal = 5;
    
    *(Random DOT get(testData)) = newVal;

    bool testDataHasNewVal = std::find(testData.cbegin(), testData.cend(), newVal) != testData.end();
    CHECK(testDataHasNewVal);
}

TEST_CASE("Get random engine by value") {
    auto engine = Random DOT get_engine();

    bool isEngineTheSame = true;
    for (std::uint8_t i{ 0u };
        i < std::numeric_limits<std::uint8_t>::max();
        ++i) {
        auto engineRandom = engine();
        auto randomRandom = Random DOT get();

        if (engineRandom != randomRandom)
        {
            isEngineTheSame = false;
            break;
        }
    }

    REQUIRE(isEngineTheSame);
}

TEST_CASE("Get random engine by reference:)") {
    auto& engine = Random DOT engine();

    bool isEngineNotTheSame = false;
    for (std::uint8_t i{ 0u };
        i < std::numeric_limits<std::uint8_t>::max();
        ++i) {
        auto engineRandom = engine();
        auto randomRandom = Random DOT get();

        if (engineRandom != randomRandom)
        {
            isEngineNotTheSame = true;
            break;
        }
    }

    REQUIRE(isEngineNotTheSame);
}

TEST_CASE("Construct Seeder only once") {
	constexpr auto currentSeed = 42u;
	thread_local int constructCount = 0;
	constructCount = 0;
	thread_local int invokeCount = 0;
	invokeCount = 0;

	struct Seeder {
		Seeder() { ++constructCount; }
		unsigned operator() () {
			++invokeCount;
			return currentSeed;
		}
	};

#ifdef RANDOM_STATIC

	using tRandom = effolkronium::basic_random_static<std::mt19937, Seeder>;

#endif
#ifdef RANDOM_THREAD_LOCAL

	using tRandom = effolkronium::basic_random_thread_local<std::mt19937, Seeder>;

#endif
#ifdef RANDOM_LOCAL

	effolkronium::basic_random_local<std::mt19937, Seeder> tRandom;

#endif

	std::vector<int> randomValues;
	for(int i = 0; i < 256; ++i)
		tRandom DOT get();

	REQUIRE(constructCount == 1);
	REQUIRE(invokeCount == 1);

	tRandom DOT reseed();

	REQUIRE(constructCount == 2);
	REQUIRE(invokeCount == 2);
}