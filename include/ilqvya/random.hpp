/*
______  ___   _   _______ ________  __
| ___ \/ _ \ | \ | |  _  \  _  |  \/  | Random for modern C++
| |_/ / /_\ \|  \| | | | | | | | .  . |
|    /|  _  || . ` | | | | | | | |\/| | version 2.0.0
| |\ \| | | || |\  | |/ /\ \_/ / |  | |
\_| \_\_| |_/\_| \_/___/  \___/\_|  |_/ https://github.com/ilqvya/random

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright (c) 2017-2026 ilqvya (Illia Polishchuk)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files( the "Software" ), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ILQVYA_RANDOM_HPP
#define ILQVYA_RANDOM_HPP

#define ILQVYA_RANDOM_VERSION_MAJOR 2
#define ILQVYA_RANDOM_VERSION_MINOR 0
#define ILQVYA_RANDOM_VERSION_PATCH 0
#define ILQVYA_RANDOM_VERSION                                                                                          \
    ( ILQVYA_RANDOM_VERSION_MAJOR * 10000 + ILQVYA_RANDOM_VERSION_MINOR * 100 + ILQVYA_RANDOM_VERSION_PATCH )

#include <random>
#include <chrono> // timed seed
#include <type_traits>
#include <cassert>
#include <initializer_list>
#include <utility>   // std::forward, std::declval
#include <algorithm> // std::shuffle, std::next, std::distance
#include <iterator>  // std::begin, std::end, std::iterator_traits
#include <limits>    // std::numeric_limits
#include <ostream>
#include <istream>
#include <array>
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <thread>
#include <mutex>
#include <cmath>
#include <atomic>

#if defined( _MSVC_LANG )
#    define ILQVYA_CPLUSPLUS _MSVC_LANG
#else
#    define ILQVYA_CPLUSPLUS __cplusplus
#endif

#if ILQVYA_CPLUSPLUS >= 201703L
#    include <string_view>
#endif

#if defined( __has_include )
#    if __has_include( <span> ) && ILQVYA_CPLUSPLUS >= 202002L
#        include <span>
#    endif
#endif

#if defined( __has_cpp_attribute )
#    if __has_cpp_attribute( nodiscard ) && ILQVYA_CPLUSPLUS >= 201703L
#        define ILQVYA_NODISCARD [[nodiscard]]
#    endif
#endif
#ifndef ILQVYA_NODISCARD
#    define ILQVYA_NODISCARD
#endif

#if ILQVYA_CPLUSPLUS > 202302L && ( defined( __cpp_impl_reflection ) || defined( __cpp_reflection ) )
#    if __has_include( <meta>)
#        include <meta>
#        define ILQVYA_HAS_REFLECTION 1
#    endif
#endif

namespace ilqvya {

    namespace details {
        /// Key type for getting common type numbers or objects
        struct common {};

        /// Key type for weighted random number generation
        struct weight {};

        /// True if type T is applicable by a std::uniform_int_distribution
        template<typename T>
        struct is_uniform_int {
            static constexpr bool value =
                std::is_same<T, short>::value || std::is_same<T, int>::value || std::is_same<T, long>::value ||
                std::is_same<T, long long>::value || std::is_same<T, unsigned short>::value ||
                std::is_same<T, unsigned int>::value || std::is_same<T, unsigned long>::value ||
                std::is_same<T, unsigned long long>::value;
        };

        /// True if type T is applicable by a std::uniform_real_distribution
        template<typename T>
        struct is_uniform_real {
            static constexpr bool value =
                std::is_same<T, float>::value || std::is_same<T, double>::value || std::is_same<T, long double>::value;
        };

        /// True if type T is plain byte
        template<typename T>
        struct is_byte {
            static constexpr bool value = std::is_same<T, signed char>::value || std::is_same<T, unsigned char>::value
#if ILQVYA_CPLUSPLUS >= 201703L
                                          || std::is_same<T, std::byte>::value
#endif
                ;
        };

        template<typename T>
        struct byte_limits {
            static constexpr T min( ) noexcept { return ( std::numeric_limits<T>::lowest )( ); }
            static constexpr T max( ) noexcept { return ( std::numeric_limits<T>::max )( ); }
        };

#if ILQVYA_CPLUSPLUS >= 201703L
        template<>
        struct byte_limits<std::byte> {
            static constexpr std::byte min( ) noexcept { return std::byte{ 0 }; }
            static constexpr std::byte max( ) noexcept { return std::byte{ 255 }; }
        };
#endif

        /// True if type T is plain number type
        template<typename T>
        struct is_supported_number {
            static constexpr bool value = is_byte<T>::value || is_uniform_real<T>::value || is_uniform_int<T>::value;
        };

        /// True if types A and B are supported numbers compatible for common_type selection.
        /// Disallows mixing signed and unsigned integer types to prevent negative wrap-around.
        template<typename A, typename B>
        struct is_valid_common_numbers {
            static constexpr bool value =
                is_supported_number<A>::value && is_supported_number<B>::value &&
                !( std::is_integral<A>::value && std::is_integral<B>::value &&
                   ( std::is_signed<A>::value != std::is_signed<B>::value ) );
        };

        /// True if type T is character type
        template<typename T>
        struct is_supported_character {
            static constexpr bool value = std::is_same<T, char>::value || std::is_same<T, wchar_t>::value ||
                                          std::is_same<T, char16_t>::value || std::is_same<T, char32_t>::value
#if defined( __cpp_char8_t )
                                          || std::is_same<T, char8_t>::value
#endif
                ;
        };

        template<typename CharT>
        struct default_char_range {
            static constexpr CharT min_val( ) noexcept { return static_cast<CharT>( ' ' ); }
            static constexpr CharT max_val( ) noexcept { return static_cast<CharT>( '~' ); }
        };

        template<typename T>
        constexpr typename std::enable_if<std::is_signed<T>::value, bool>::type is_negative( T val ) noexcept {
            return val < T( 0 );
        }

        template<typename T>
        constexpr typename std::enable_if<!std::is_signed<T>::value, bool>::type is_negative( T ) noexcept {
            return false;
        }

        template<typename T>
        constexpr typename std::enable_if<std::is_signed<T>::value, bool>::type is_non_positive( T val ) noexcept {
            return val <= T( 0 );
        }

        template<typename T>
        constexpr typename std::enable_if<!std::is_signed<T>::value, bool>::type is_non_positive( T val ) noexcept {
            return val == T( 0 );
        }

        template<typename CharT>
        constexpr typename std::enable_if<( sizeof( CharT ) >= 4 ), bool>::type
        exceeds_unicode_max( CharT val ) noexcept {
            return static_cast<std::uint32_t>( static_cast<typename std::make_unsigned<CharT>::type>( val ) ) > 0x10FFFF;
        }

        template<typename CharT>
        constexpr typename std::enable_if<( sizeof( CharT ) < 4 ), bool>::type
        exceeds_unicode_max( CharT ) noexcept {
            return false;
        }

        template<typename CharT>
        constexpr typename std::enable_if<( sizeof( CharT ) >= 2 ), bool>::type
        is_unicode_surrogate( CharT ch ) noexcept {
            return static_cast<std::uint32_t>( static_cast<typename std::make_unsigned<CharT>::type>( ch ) ) >= 0xD800 &&
                   static_cast<std::uint32_t>( static_cast<typename std::make_unsigned<CharT>::type>( ch ) ) <= 0xDFFF;
        }

        template<typename CharT>
        constexpr typename std::enable_if<( sizeof( CharT ) < 2 ), bool>::type
        is_unicode_surrogate( CharT ) noexcept {
            return false;
        }

        template<typename CharT>
        constexpr typename std::enable_if<( sizeof( CharT ) >= 2 ), bool>::type
        is_unicode_surrogate_range( CharT from, CharT to ) noexcept {
            return static_cast<std::uint32_t>( static_cast<typename std::make_unsigned<CharT>::type>( from ) ) >= 0xD800 &&
                   static_cast<std::uint32_t>( static_cast<typename std::make_unsigned<CharT>::type>( to ) ) <= 0xDFFF;
        }

        template<typename CharT>
        constexpr typename std::enable_if<( sizeof( CharT ) < 2 ), bool>::type
        is_unicode_surrogate_range( CharT, CharT ) noexcept {
            return false;
        }

        /// True if type T is iterator
        template<typename T>
        struct is_iterator {
        private:
            static char test( ... );

            template<typename U, typename = typename std::iterator_traits<U>::difference_type,
                     typename = typename std::iterator_traits<U>::pointer,
                     typename = typename std::iterator_traits<U>::reference,
                     typename = typename std::iterator_traits<U>::value_type,
                     typename = typename std::iterator_traits<U>::iterator_category>
            static long test( U&& );

        public:
            static constexpr bool value = std::is_same<decltype( test( std::declval<T>( ) ) ), long>::value;
        };

        /// True if type T satisfies at least ForwardIterator
        template<typename T>
        struct is_forward_iterator {
            static constexpr bool value =
                is_iterator<T>::value &&
                std::is_base_of<std::forward_iterator_tag,
                                typename std::iterator_traits<T>::iterator_category>::value;
        };

        template<typename...>
        using void_t = void;

        template<typename C, typename = void>
        struct has_reserve : std::false_type {};

        template<typename C>
        struct has_reserve<C, void_t<decltype( std::declval<C&>( ).reserve( std::declval<std::size_t>( ) ) )>>
            : std::true_type {};

        template<typename C>
        typename std::enable_if<has_reserve<C>::value>::type reserve_if_supported( C& container, std::size_t size ) {
            container.reserve( size );
        }

        template<typename C>
        typename std::enable_if<!has_reserve<C>::value>::type reserve_if_supported( C&, std::size_t ) {}

        template<typename C, typename = void>
        struct has_push_back : std::false_type {};

        template<typename C>
        struct has_push_back<C, void_t<decltype( std::declval<C&>( ).push_back( std::declval<typename C::value_type>( ) ) )>>
            : std::true_type {};

        template<typename C>
        typename std::enable_if<has_push_back<C>::value, std::back_insert_iterator<C>>::type
        make_inserter( C& container ) {
            return std::back_inserter( container );
        }

        template<typename C>
        typename std::enable_if<!has_push_back<C>::value, std::insert_iterator<C>>::type
        make_inserter( C& container ) {
            return std::inserter( container, std::end( container ) );
        }

        template<typename C, typename = void>
        struct has_insert_with_pos : std::false_type {};

        template<typename C>
        struct has_insert_with_pos<
            C, void_t<decltype( std::declval<C&>( ).insert( std::declval<typename C::const_iterator>( ),
                                                            std::declval<typename C::value_type>( ) ) )>>
            : std::true_type {};

        template<typename C, typename = void>
        struct has_insert_val : std::false_type {};

        template<typename C>
        struct has_insert_val<C,
                              void_t<decltype( std::declval<C&>( ).insert( std::declval<typename C::value_type>( ) ) )>>
            : std::true_type {};

        template<typename C>
        struct has_insert : std::integral_constant<bool, has_insert_with_pos<C>::value || has_insert_val<C>::value> {};

        template<typename Type, typename = void>
        struct is_map : public std::false_type {};

        template<typename Type>
        struct is_map<Type, void_t<typename Type::key_type, typename Type::mapped_type, typename Type::value_type>>
            : public std::true_type {};

        template<typename Container, typename = void>
        struct is_pair_container : public std::false_type {};

        template<typename Container>
        struct is_pair_container<
            Container, void_t<typename Container::value_type::first_type, typename Container::value_type::second_type>>
            : public std::true_type {};

        template<typename Target, typename Source, typename = void>
        struct can_represent {
            static bool check( Source val ) {
                using CommonReal = typename std::conditional<
                    std::is_same<Target, long double>::value || std::is_same<Source, long double>::value,
                    long double, double>::type;
                const CommonReal cval = static_cast<CommonReal>( val );
                if ( !std::isfinite( cval ) )
                    return false;
                return cval >= static_cast<CommonReal>( ( std::numeric_limits<Target>::lowest )( ) ) &&
                       cval <= static_cast<CommonReal>( ( std::numeric_limits<Target>::max )( ) );
            }
        };

        template<typename Target, typename Source>
        struct can_represent_integral {
            static bool check_impl( Source val, std::true_type, std::false_type ) noexcept {
                if ( val < Source( 0 ) )
                    return false;
                using USource = typename std::make_unsigned<Source>::type;
                return static_cast<std::uintmax_t>( static_cast<USource>( val ) ) <=
                       static_cast<std::uintmax_t>( ( std::numeric_limits<Target>::max )( ) );
            }

            static bool check_impl( Source val, std::false_type, std::true_type ) noexcept {
                return static_cast<std::uintmax_t>( val ) <=
                       static_cast<std::uintmax_t>( ( std::numeric_limits<Target>::max )( ) );
            }

            static bool check_impl( Source val, std::false_type, std::false_type ) noexcept {
                return static_cast<std::uintmax_t>( val ) <=
                       static_cast<std::uintmax_t>( ( std::numeric_limits<Target>::max )( ) );
            }

            static bool check_impl( Source val, std::true_type, std::true_type ) noexcept {
                return static_cast<std::intmax_t>( val ) >=
                           static_cast<std::intmax_t>( ( std::numeric_limits<Target>::lowest )( ) ) &&
                       static_cast<std::intmax_t>( val ) <=
                           static_cast<std::intmax_t>( ( std::numeric_limits<Target>::max )( ) );
            }

            static bool check( Source val ) noexcept {
                return check_impl( val,
                                   std::integral_constant<bool, std::is_signed<Source>::value>( ),
                                   std::integral_constant<bool, std::is_signed<Target>::value>( ) );
            }
        };

        template<typename Target, typename Source>
        struct can_represent<Target, Source,
                              typename std::enable_if<std::is_integral<Target>::value && std::is_integral<Source>::value>::type> {
            static bool check( Source val ) noexcept {
                return can_represent_integral<Target, Source>::check( val );
            }
        };

        template<typename Value, typename Weight = double>
        struct weighted_result {
            Value value{ };
            Weight weight{ };

            constexpr weighted_result( ) = default;
            constexpr weighted_result( Value val, Weight w ) noexcept(
                std::is_nothrow_move_constructible<Value>::value &&
                std::is_nothrow_move_constructible<Weight>::value )
                : value( std::move( val ) ), weight( std::move( w ) ) {}
            constexpr weighted_result( const std::pair<Value, Weight>& p ) noexcept(
                std::is_nothrow_constructible<Value, const Value&>::value &&
                std::is_nothrow_constructible<Weight, const Weight&>::value )
                : value( p.first ), weight( p.second ) {}
            constexpr weighted_result( std::pair<Value, Weight>&& p ) noexcept(
                std::is_nothrow_constructible<Value, Value&&>::value &&
                std::is_nothrow_constructible<Weight, Weight&&>::value )
                : value( std::move( p.first ) ), weight( std::move( p.second ) ) {}

            friend constexpr bool operator==( const weighted_result& lhs, const weighted_result& rhs ) noexcept(
                noexcept( lhs.value == rhs.value ) && noexcept( lhs.weight == rhs.weight ) ) {
                return lhs.value == rhs.value && lhs.weight == rhs.weight;
            }

            friend constexpr bool operator!=( const weighted_result& lhs, const weighted_result& rhs ) noexcept(
                noexcept( !( lhs == rhs ) ) ) {
                return !( lhs == rhs );
            }
        };

#if defined( ILQVYA_HAS_REFLECTION )
        template<typename E>
        consteval std::size_t enum_count( ) {
            auto enums = std::meta::enumerators_of( ^^E );
            return enums.size( );
        }

        template<typename E>
        struct enum_traits {
            static constexpr std::size_t count = enum_count<E>( );

            static consteval auto make_array( ) {
                std::array<E, count> arr{ };
                auto enums = std::meta::enumerators_of( ^^E );
                for ( std::size_t i = 0; i < count; ++i ) {
                    arr[i] = std::meta::extract<E>( enums[i] );
                }
                return arr;
            }

            static constexpr std::array<E, count> values = make_array( );
        };
#endif

    } // namespace details

    template<typename Value, typename Weight = double>
    using weighted_result = details::weighted_result<Value, Weight>;

    /// UUID / GUID specification versions (RFC 4122, RFC 9562, MongoDB)
    enum class uuid_version {
        nil = 0,
        v1 = 1,
        v4 = 4,
        v6 = 6,
        v7 = 7,
        max = 15,
        mongo_object_id = 12
    };

    /// Default seeder for 'random' classes
    struct seeder_default {
        seeder_default( ) : entropy_source( ), seed_seq( entropy_source.data.begin( ), entropy_source.data.end( ) ) {}

        /// return seed sequence
        std::seed_seq& operator( )( ) { return seed_seq; }

    private:
        // 8 x 32-bit words (256 bits) of std::random_device entropy, mixed with the high-resolution clock
        // and thread ID in case random_device is deterministic or on concurrent threads.
        // Queries clock and thread ID once and distributes both 32-bit halves across the words.
        struct seed_data {
            std::array<std::uint32_t, 8> data{ };
            seed_data( ) : data{ } {
                std::random_device rd;
                const auto now = static_cast<std::uint64_t>(
                    std::chrono::high_resolution_clock::now( ).time_since_epoch( ).count( ) );
                const auto tid = static_cast<std::uint64_t>(
                    std::hash<std::thread::id>{ }( std::this_thread::get_id( ) ) );

                const std::uint32_t time_lo = static_cast<std::uint32_t>( now );
                const std::uint32_t time_hi = static_cast<std::uint32_t>( now >> 32 );
                const std::uint32_t tid_lo  = static_cast<std::uint32_t>( tid );
                const std::uint32_t tid_hi  = static_cast<std::uint32_t>( tid >> 32 );

                for ( std::size_t i = 0; i < data.size( ); ++i ) {
                    const std::uint32_t time_half = ( ( i & 1 ) == 0 ) ? time_lo : time_hi;
                    const std::uint32_t tid_half  = ( ( i & 2 ) == 0 ) ? tid_lo : tid_hi;
                    data[i] = static_cast<std::uint32_t>( rd( ) ) ^ time_half ^ tid_half ^ static_cast<std::uint32_t>( i );
                }
            }
        };

        seed_data entropy_source;
        std::seed_seq seed_seq;
    };

    namespace details {

        /**
         * \brief Stateless core implementation containing algorithms taking Engine& engine
         */
        template<typename Engine, typename Seeder = seeder_default,
                 template<typename> class IntegerDist = std::uniform_int_distribution,
                 template<typename> class RealDist = std::uniform_real_distribution,
                 typename BoolDist = std::bernoulli_distribution>
        struct random_impl {
            using engine_type = Engine;
            using seeder_type = Seeder;
            template<typename T>
            using integer_dist_t = IntegerDist<T>;
            template<typename T>
            using real_dist_t = RealDist<T>;
            using bool_dist_t = BoolDist;
            using common = details::common;
            using weight = details::weight;
            using uuid_version = ilqvya::uuid_version;

            ILQVYA_NODISCARD static constexpr typename Engine::result_type( min )( ) noexcept {
                return (Engine::min)( );
            }

            ILQVYA_NODISCARD static constexpr typename Engine::result_type( max )( ) noexcept {
                return (Engine::max)( );
            }

            static void discard( Engine& engine, const unsigned long long z ) noexcept(
                noexcept( engine.discard( z ) ) ) {
                engine.discard( z );
            }

            static void reseed( Engine& engine ) {
                Seeder seeder;
                seed( engine, seeder( ) );
            }

            static void seed( Engine& engine, const typename Engine::result_type value = Engine::default_seed ) noexcept(
                noexcept( engine.seed( value ) ) ) {
                engine.seed( value );
            }

            template<typename Sseq>
            static void seed( Engine& engine, Sseq& seq ) noexcept( noexcept( engine.seed( seq ) ) ) {
                engine.seed( seq );
            }

            ILQVYA_NODISCARD static typename Engine::result_type get( Engine& engine ) noexcept(
                noexcept( engine( ) ) ) {
                return engine( );
            }

            ILQVYA_NODISCARD static bool is_equal( const Engine& engine, const Engine& other ) noexcept(
                noexcept( engine == other ) ) {
                return engine == other;
            }

            template<typename CharT, typename Traits>
            static void serialize( const Engine& engine, std::basic_ostream<CharT, Traits>& ost ) {
                ost << engine;
            }

            template<typename CharT, typename Traits>
            static void deserialize( Engine& engine, std::basic_istream<CharT, Traits>& ist ) {
                ist >> engine;
            }

            template<typename T>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_uniform_int<T>::value, T>::type
            get( Engine& engine, T from = ( std::numeric_limits<T>::lowest )( ),
                 T to = ( std::numeric_limits<T>::max )( ) ) {
                if ( from == to )
                    return from;
                if ( from < to )
                    return IntegerDist<T>{ from, to }( engine );
                return IntegerDist<T>{ to, from }( engine );
            }

            template<typename T>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_uniform_real<T>::value, T>::type
            get( Engine& engine, T from = T( 0 ), T to = T( 1 ) ) {
                if ( !std::isfinite( from ) || !std::isfinite( to ) )
                    throw std::invalid_argument( "Random::get: range bounds must be finite numbers" );
                const T diff = ( from < to ) ? ( to - from ) : ( from - to );
                if ( !std::isfinite( diff ) )
                    throw std::overflow_error( "Random::get: range width exceeds representable floating-point range" );
                if ( from == to )
                    return from;
                if ( from < to )
                    return RealDist<T>{ from, to }( engine );
                return RealDist<T>{ to, from }( engine );
            }

            template<typename T>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_byte<T>::value, T>::type
            get( Engine& engine, T from = details::byte_limits<T>::min( ),
                 T to = details::byte_limits<T>::max( ) ) {
                using short_t = typename std::conditional<std::is_signed<T>::value, short, unsigned short>::type;
                if ( from == to )
                    return from;
                if ( from < to )
                    return static_cast<T>( get<short_t>( engine, static_cast<short_t>( from ), static_cast<short_t>( to ) ) );
                return static_cast<T>( get<short_t>( engine, static_cast<short_t>( to ), static_cast<short_t>( from ) ) );
            }

            template<typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
            ILQVYA_NODISCARD static
                typename std::enable_if<std::is_same<Key, common>::value &&
                                            details::is_valid_common_numbers<A, B>::value,
                                        C>::type
                get( Engine& engine, A from = ( std::numeric_limits<A>::lowest )( ),
                     B to = ( std::numeric_limits<B>::max )( ) ) {
                return get( engine, static_cast<C>( from ), static_cast<C>( to ) );
            }

            template<typename T>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<T>::value, T>::type
            get( Engine& engine, T from = ( std::numeric_limits<T>::lowest )( ),
                 T to = ( std::numeric_limits<T>::max )( ) ) {
                using int_t = typename std::conditional<std::is_signed<T>::value, std::int64_t, std::uint64_t>::type;
                if ( from == to )
                    return from;
                if ( from < to )
                    return static_cast<T>( IntegerDist<int_t>{ static_cast<int_t>( from ),
                                                               static_cast<int_t>( to ) }( engine ) );
                return static_cast<T>( IntegerDist<int_t>{ static_cast<int_t>( to ),
                                                           static_cast<int_t>( from ) }( engine ) );
            }

            template<typename T>
            ILQVYA_NODISCARD static typename std::enable_if<std::is_same<T, bool>::value, bool>::type
            get( Engine& engine, const double probability = 0.5 ) {
                if ( std::isnan( probability ) || probability < 0.0 || probability > 1.0 )
                    throw std::invalid_argument( "Random::get<bool>: probability must be between 0.0 and 1.0" );
                return BoolDist{ probability }( engine );
            }

            ILQVYA_NODISCARD static bool flip( Engine& engine ) {
                return get<bool>( engine, 0.5 );
            }

            ILQVYA_NODISCARD static bool chance( Engine& engine, const double probability ) {
                return get<bool>( engine, probability );
            }

            ILQVYA_NODISCARD static bool chance_pct( Engine& engine, const double percentage ) {
                if ( std::isnan( percentage ) || percentage < 0.0 || percentage > 100.0 )
                    throw std::invalid_argument( "Random::chance_pct: percentage must be between 0.0 and 100.0" );
                return get<bool>( engine, percentage / 100.0 );
            }

            template<typename RealType = double, typename Mean = RealType, typename Stddev = RealType,
                     typename EffectiveReal = typename std::conditional<details::is_uniform_real<RealType>::value, RealType, double>::type>
            ILQVYA_NODISCARD static EffectiveReal normal( Engine& engine, Mean mean = Mean( 0 ),
                                                          Stddev stddev = Stddev( 1 ) ) {
                const auto m = static_cast<EffectiveReal>( mean );
                const auto s = static_cast<EffectiveReal>( stddev );
                if ( !std::isfinite( m ) || !std::isfinite( s ) || s <= EffectiveReal( 0 ) )
                    throw std::invalid_argument( "Random::normal: stddev must be greater than 0" );
                std::normal_distribution<EffectiveReal> dist( m, s );
                return dist( engine );
            }

            template<typename RealType = double, typename Lambda = RealType,
                     typename EffectiveReal = typename std::conditional<details::is_uniform_real<RealType>::value, RealType, double>::type>
            ILQVYA_NODISCARD static EffectiveReal exponential( Engine& engine, Lambda lambda = Lambda( 1 ) ) {
                const auto l = static_cast<EffectiveReal>( lambda );
                if ( !std::isfinite( l ) || l <= EffectiveReal( 0 ) )
                    throw std::invalid_argument( "Random::exponential: lambda must be greater than 0" );
                std::exponential_distribution<EffectiveReal> dist( l );
                return dist( engine );
            }

            template<typename IntType = int, typename Mean = double>
            ILQVYA_NODISCARD static typename std::enable_if<std::is_integral<IntType>::value, IntType>::type
            poisson( Engine& engine, Mean mean = Mean( 1 ) ) {
                const auto m = static_cast<double>( mean );
                if ( !std::isfinite( m ) || m <= 0.0 )
                    throw std::invalid_argument( "Random::poisson: mean must be greater than 0" );
                std::poisson_distribution<IntType> dist( m );
                return dist( engine );
            }

            template<typename T>
            ILQVYA_NODISCARD static T get( Engine& engine, std::initializer_list<T> init_list ) {
                if ( 0u == init_list.size( ) )
                    throw std::invalid_argument( "Random::get: initializer list must not be empty" );
                return *get( engine, init_list.begin( ), init_list.end( ) );
            }

            template<typename ForwardIt>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_iterator<ForwardIt>::value, ForwardIt>::type
            get( Engine& engine, ForwardIt first, ForwardIt last ) {
                static_assert( details::is_forward_iterator<ForwardIt>::value,
                               "Random::get(first, last) requires forward iterators. "
                               "Single-pass input iterators (e.g., std::istream_iterator) cannot be traversed twice." );
                const auto size = std::distance( first, last );
                if ( details::is_non_positive( size ) )
                    return last;
                using diff_t = typename std::iterator_traits<ForwardIt>::difference_type;
                return std::next( first, get<diff_t>( engine, diff_t( 0 ), static_cast<diff_t>( size - 1 ) ) );
            }

            template<typename Container>
            ILQVYA_NODISCARD static auto get( Engine& engine, Container& container ) ->
                typename std::enable_if<details::is_iterator<decltype( std::begin( container ) )>::value,
                                        decltype( std::begin( container ) )>::type {
                return get( engine, std::begin( container ), std::end( container ) );
            }

            template<typename Container>
            ILQVYA_NODISCARD static auto get( Engine& engine, const Container& container ) ->
                typename std::enable_if<details::is_iterator<decltype( std::begin( container ) )>::value,
                                        decltype( std::begin( container ) )>::type {
                return get( engine, std::begin( container ), std::end( container ) );
            }

            template<typename Container>
            ILQVYA_NODISCARD static auto get( Engine& engine, Container&& container ) ->
                typename std::enable_if<
                    !std::is_lvalue_reference<Container>::value &&
                        details::is_iterator<decltype( std::begin( container ) )>::value,
                    typename std::iterator_traits<decltype( std::begin( container ) )>::value_type>::type {
                auto first = std::begin( container );
                auto last  = std::end( container );
                if ( first == last )
                    throw std::invalid_argument( "Random::get: container must not be empty" );
                auto it = get( engine, first, last );
                return std::move( *it );
            }

            template<template<typename...> class Container, typename A>
            ILQVYA_NODISCARD static Container<A>
            get( Engine& engine, A from, A to, std::size_t size ) {
                Container<A> container;
                details::reserve_if_supported( container, size );
                for ( std::size_t i = 0; i < size; ++i )
                    container.insert( std::end( container ), get( engine, from, to ) );
                return container;
            } // LCOV_EXCL_LINE

            template<template<typename...> class Container, typename Key, typename A, typename B,
                     typename C = typename std::common_type<A, B>::type>
            ILQVYA_NODISCARD static
                typename std::enable_if<std::is_same<Key, common>::value, Container<C>>::type
                get( Engine& engine, A start, B end, std::size_t size ) {
                Container<C> container;
                details::reserve_if_supported( container, size );
                for ( std::size_t i = 0; i < size; ++i )
                    container.insert( std::end( container ), get<common>( engine, start, end ) );
                return container;
            } // LCOV_EXCL_LINE

            template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A>
            ILQVYA_NODISCARD static
                typename std::enable_if<!details::has_insert<Container<A, N>>::value, Container<A, N>>::type
                get( Engine& engine, A start, A end ) {
                Container<A, N> container{ };
                for ( std::size_t i = 0; i < N; ++i )
                    container[i] = get( engine, start, end );
                return container;
            }

            template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename Key,
                     typename A, typename B, typename C = typename std::common_type<A, B>::type>
            ILQVYA_NODISCARD static typename std::enable_if<
                std::is_same<Key, common>::value && !details::has_insert<Container<C, N>>::value, Container<C, N>>::type
            get( Engine& engine, A start, B end ) {
                Container<C, N> container{ };
                for ( std::size_t i = 0; i < N; ++i )
                    container[i] = get<common>( engine, start, end );
                return container;
            }

            template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A = int>
            ILQVYA_NODISCARD static typename std::enable_if<!details::has_insert<Container<A, N>>::value, Container<A, N>>::type
            get( Engine& engine ) {
                Container<A, N> container{ };
                for ( std::size_t i = 0; i < N; ++i )
                    container[i] = get<A>( engine );
                return container;
            } // LCOV_EXCL_LINE

            template<std::size_t N, typename T>
            ILQVYA_NODISCARD static std::array<T, N> get_array( Engine& engine, T from, T to ) {
                std::array<T, N> arr{ };
                for ( std::size_t i = 0; i < N; ++i ) {
                    arr[i] = get( engine, from, to );
                }
                return arr;
            } // LCOV_EXCL_LINE

            template<std::size_t N, typename Key, typename A, typename B,
                     typename C = typename std::common_type<A, B>::type>
            ILQVYA_NODISCARD static typename std::enable_if<std::is_same<Key, common>::value &&
                                                                details::is_valid_common_numbers<A, B>::value,
                                                            std::array<C, N>>::type
            get_array( Engine& engine, A from, B to ) {
                std::array<C, N> arr{ };
                for ( std::size_t i = 0; i < N; ++i ) {
                    arr[i] = get<Key, A, B, C>( engine, from, to );
                }
                return arr;
            } // LCOV_EXCL_LINE

            template<std::size_t N, typename T = int>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_number<T>::value, std::array<T, N>>::type
            get_array( Engine& engine ) {
                std::array<T, N> arr{ };
                for ( std::size_t i = 0; i < N; ++i ) {
                    arr[i] = get<T>( engine );
                }
                return arr;
            } // LCOV_EXCL_LINE

            template<typename CharT, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                            const CharT*>::type
            get( Engine& engine, const CharT ( &literal )[N] ) {
                static_assert( N > 1, "Random::get: string literal must not be empty" );
                return std::addressof( literal[get<std::size_t>( engine, std::size_t( 0 ), static_cast<std::size_t>( N - 2 ) )] );
            }

            template<typename T, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<!details::is_supported_character<T>::value, T*>::type
            get( Engine& engine, T ( &array )[N] ) {
                static_assert( N > 0, "Random::get: built-in array must not be empty" );
                return std::addressof( array[get<std::size_t>( engine, std::size_t( 0 ), static_cast<std::size_t>( N - 1 ) )] );
            }

            template<typename T, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<!details::is_supported_character<T>::value,
                                                            typename std::remove_cv<T>::type>::type
            get( Engine& engine, T ( &&array )[N] ) {
                static_assert( N > 0, "Random::get: built-in array must not be empty" );
                return array[get<std::size_t>( engine, std::size_t( 0 ), static_cast<std::size_t>( N - 1 ) )];
            }

            template<typename Dist, typename... Args>
            ILQVYA_NODISCARD static typename Dist::result_type get( Engine& engine, Args&&... args ) {
                return Dist{ std::forward<Args>( args )... }( engine );
            }

            template<typename Dist>
            ILQVYA_NODISCARD static typename std::decay<Dist>::type::result_type get( Engine& engine, Dist&& dist ) {
                return std::forward<Dist>( dist )( engine );
            }

        private:
            template<typename PairContainer>
            ILQVYA_NODISCARD static auto sample_weighted_container_impl( Engine& engine, PairContainer& pair_container,
                                                                         std::true_type ) ->
                decltype( std::begin( pair_container ) ) {
                using WeightType = typename PairContainer::value_type::second_type;
                using IteratorType = decltype( std::begin( pair_container ) );

                WeightType total_weight = 0;
                IteratorType last_nonzero_it = std::end( pair_container );
                for ( IteratorType it = std::begin( pair_container ); it != std::end( pair_container ); ++it ) {
                    if ( details::is_negative( it->second ) )
                        throw std::invalid_argument( "Random::get: weights must be non-negative" );
                    if ( it->second > WeightType( 0 ) ) {
                        last_nonzero_it = it;
                    }
                    if ( ( std::numeric_limits<WeightType>::max )( ) - total_weight < it->second )
                        throw std::overflow_error( "Random::get: total weight exceeds maximum integer value" );
                    total_weight += it->second;
                }
                if ( details::is_non_positive( total_weight ) )
                    return std::end( pair_container );

                WeightType random_weight = get( engine, WeightType( 0 ), total_weight - 1 );
                WeightType sum = 0;
                for ( IteratorType it = std::begin( pair_container ); it != last_nonzero_it; ++it ) {
                    sum += it->second;
                    if ( sum > random_weight )
                        return it;
                }
                return last_nonzero_it;
            }

            template<typename PairContainer>
            ILQVYA_NODISCARD static auto sample_weighted_container_impl( Engine& engine, PairContainer& pair_container,
                                                                         std::false_type ) ->
                decltype( std::begin( pair_container ) ) {
                using WeightType = typename PairContainer::value_type::second_type;
                using IteratorType = decltype( std::begin( pair_container ) );

                WeightType total_weight = 0;
                IteratorType last_nonzero_it = std::end( pair_container );
                for ( IteratorType it = std::begin( pair_container ); it != std::end( pair_container ); ++it ) {
                    if ( !std::isfinite( it->second ) )
                        throw std::invalid_argument( "Random::get: weights must be finite numbers" );
                    if ( details::is_negative( it->second ) )
                        throw std::invalid_argument( "Random::get: weights must be non-negative" );
                    if ( it->second > WeightType( 0 ) ) {
                        last_nonzero_it = it;
                    }
                    total_weight += it->second;
                }
                if ( details::is_non_positive( total_weight ) || !std::isfinite( total_weight ) )
                    return std::end( pair_container );

                WeightType random_weight = get( engine, WeightType( 0 ), total_weight );
                WeightType sum = 0;
                for ( IteratorType it = std::begin( pair_container ); it != last_nonzero_it; ++it ) {
                    sum += it->second;
                    if ( sum > random_weight )
                        return it;
                }
                return last_nonzero_it;
            }

            template<typename PairContainer>
            ILQVYA_NODISCARD static auto sample_weighted_container( Engine& engine, PairContainer& pair_container ) ->
                decltype( std::begin( pair_container ) ) {
                using WeightType = typename PairContainer::value_type::second_type;
                return sample_weighted_container_impl(
                    engine, pair_container,
                    std::integral_constant<bool, details::is_uniform_int<WeightType>::value>( ) );
            }

        public:
            template<typename Key, class PairContainer>
            ILQVYA_NODISCARD static auto get( Engine& engine, PairContainer& pair_container ) ->
                typename std::enable_if<
                    details::is_pair_container<PairContainer>::value &&
                        details::is_iterator<decltype( std::begin( pair_container ) )>::value &&
                        ( details::is_uniform_int<typename PairContainer::value_type::second_type>::value ||
                          details::is_uniform_real<typename PairContainer::value_type::second_type>::value ) &&
                        std::is_same<Key, details::weight>::value,
                    decltype( std::begin( pair_container ) )>::type {
                return sample_weighted_container( engine, pair_container );
            }

            template<typename Key, class PairContainer>
            ILQVYA_NODISCARD static auto get( Engine& engine, const PairContainer& pair_container ) ->
                typename std::enable_if<
                    details::is_pair_container<PairContainer>::value &&
                        details::is_iterator<decltype( std::begin( pair_container ) )>::value &&
                        ( details::is_uniform_int<typename PairContainer::value_type::second_type>::value ||
                          details::is_uniform_real<typename PairContainer::value_type::second_type>::value ) &&
                        std::is_same<Key, details::weight>::value,
                    decltype( std::begin( pair_container ) )>::type {
                return sample_weighted_container( engine, pair_container );
            }

            template<typename Key, class PairContainer>
            static auto get( Engine&, PairContainer&& ) -> typename std::enable_if<
                details::is_pair_container<typename std::remove_reference<PairContainer>::type>::value &&
                    std::is_same<Key, details::weight>::value,
                void>::type = delete;

            template<typename Key, class PairContainer>
            static auto get( Engine&, const PairContainer&& ) -> typename std::enable_if<
                details::is_pair_container<typename std::remove_reference<PairContainer>::type>::value &&
                    std::is_same<Key, details::weight>::value,
                void>::type = delete;

            template<typename Target, typename Weight>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_uniform_int<Weight>::value, Target>::type
            get_weighted_list( Engine& engine, std::initializer_list<details::weighted_result<Target, Weight>> list ) {
                if ( 0u == list.size( ) )
                    throw std::invalid_argument( "Random::get: weighted list must not be empty" );
                Weight total_weight = 0;
                const details::weighted_result<Target, Weight>* last_nonzero = nullptr;
                for ( const auto& item : list ) {
                    if ( details::is_negative( item.weight ) )
                        throw std::invalid_argument( "Random::get: weights must be non-negative" );
                    if ( item.weight > Weight( 0 ) ) {
                        last_nonzero = &item;
                    }
                    if ( ( std::numeric_limits<Weight>::max )( ) - total_weight < item.weight )
                        throw std::overflow_error( "Random::get: total weight exceeds maximum integer value" );
                    total_weight += item.weight;
                }
                if ( details::is_non_positive( total_weight ) ) {
                    return list.begin( )->value;
                }
                Weight random_weight = get( engine, Weight( 0 ), total_weight - 1 );
                Weight sum = 0;
                const auto last = last_nonzero;
                for ( const auto& item : list ) {
                    if ( &item == last ) {
                        break;
                    }
                    sum += item.weight;
                    if ( sum > random_weight ) {
                        return item.value;
                    }
                }
                if ( last != nullptr ) {
                    return last->value;
                }
                return list.begin( )->value;
            }

            template<typename Target, typename Weight>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_uniform_real<Weight>::value, Target>::type
            get_weighted_list( Engine& engine, std::initializer_list<details::weighted_result<Target, Weight>> list ) {
                if ( 0u == list.size( ) )
                    throw std::invalid_argument( "Random::get: weighted list must not be empty" );
                Weight total_weight = 0;
                const details::weighted_result<Target, Weight>* last_nonzero = nullptr;
                for ( const auto& item : list ) {
                    if ( !std::isfinite( item.weight ) )
                        throw std::invalid_argument( "Random::get: weights must be finite numbers" );
                    if ( details::is_negative( item.weight ) )
                        throw std::invalid_argument( "Random::get: weights must be non-negative" );
                    if ( item.weight > Weight( 0 ) ) {
                        last_nonzero = &item;
                    }
                    total_weight += item.weight;
                }
                if ( details::is_non_positive( total_weight ) || !std::isfinite( total_weight ) ) {
                    return list.begin( )->value;
                }
                Weight random_weight = get( engine, Weight( 0 ), total_weight );
                Weight sum = 0;
                const auto last = last_nonzero;
                for ( const auto& item : list ) {
                    if ( &item == last ) {
                        break;
                    }
                    sum += item.weight;
                    if ( sum > random_weight ) {
                        return item.value;
                    }
                }
                if ( last != nullptr ) {
                    return last->value;
                }
                return list.begin( )->value;
            }

            template<typename Target, typename Weight = double>
            ILQVYA_NODISCARD static Target get( Engine& engine,
                                                std::initializer_list<details::weighted_result<Target, Weight>> list ) {
                return get_weighted_list<Target, Weight>( engine, list );
            }

#if defined( ILQVYA_HAS_REFLECTION )
            template<typename T>
            ILQVYA_NODISCARD static typename std::enable_if<std::is_enum<T>::value, T>::type get( Engine& engine ) {
                static_assert( details::enum_traits<T>::count > 0, "Cannot pick random value from an empty enum" );
                const auto idx = get( engine, std::size_t( 0 ), details::enum_traits<T>::count - 1 );
                return details::enum_traits<T>::values[idx];
            }
#endif

            template<typename ForwardIt, typename T>
            static typename std::enable_if<details::is_iterator<ForwardIt>::value>::type
            fill( Engine& engine, ForwardIt first, ForwardIt last, T from, T to ) {
                using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
                if ( !details::can_represent<ValueType, T>::check( from ) ||
                     !details::can_represent<ValueType, T>::check( to ) ) {
                    throw std::out_of_range( "Random::fill: range bounds exceed container element type limits" );
                }
                const auto v_from = static_cast<ValueType>( from );
                const auto v_to   = static_cast<ValueType>( to );
                for ( ; first != last; ++first ) {
                    *first = get<ValueType>( engine, v_from, v_to );
                }
            }

            template<typename ForwardIt>
            static typename std::enable_if<details::is_iterator<ForwardIt>::value>::type
            fill( Engine& engine, ForwardIt first, ForwardIt last ) {
                using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
                for ( ; first != last; ++first ) {
                    *first = get<ValueType>( engine );
                }
            }

            template<typename Key, typename ForwardIt, typename A, typename B>
            static typename std::enable_if<std::is_same<Key, common>::value && details::is_iterator<ForwardIt>::value>::type
            fill( Engine& engine, ForwardIt first, ForwardIt last, A from, B to ) {
                using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
                if ( !details::can_represent<ValueType, A>::check( from ) ||
                     !details::can_represent<ValueType, B>::check( to ) ) {
                    throw std::out_of_range( "Random::fill: range bounds exceed container element type limits" );
                }
                const auto v_from = static_cast<ValueType>( from );
                const auto v_to   = static_cast<ValueType>( to );
                for ( ; first != last; ++first ) {
                    *first = get<ValueType>( engine, v_from, v_to );
                }
            }

            template<typename Container, typename T>
            static typename std::enable_if<
                !std::is_array<typename std::remove_reference<Container>::type>::value &&
                details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
                !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
            fill( Engine& engine, Container&& container, T from, T to ) {
                fill( engine, std::begin( container ), std::end( container ), from, to );
            }

            template<typename Container>
            static typename std::enable_if<
                !std::is_array<typename std::remove_reference<Container>::type>::value &&
                details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
                !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
            fill( Engine& engine, Container&& container ) {
                fill( engine, std::begin( container ), std::end( container ) );
            }

            template<typename Key, typename Container, typename A, typename B>
            static typename std::enable_if<
                std::is_same<Key, common>::value &&
                !std::is_array<typename std::remove_reference<Container>::type>::value &&
                details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
                !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
            fill( Engine& engine, Container&& container, A from, B to ) {
                fill<Key>( engine, std::begin( container ), std::end( container ), from, to );
            }

            template<typename T, std::size_t N, typename U>
            static typename std::enable_if<!std::is_const<T>::value>::type
            fill( Engine& engine, T ( &array )[N], U from, U to ) {
                fill( engine, std::begin( array ), std::end( array ), from, to );
            }

            template<typename T, std::size_t N>
            static typename std::enable_if<!std::is_const<T>::value>::type
            fill( Engine& engine, T ( &array )[N] ) {
                fill( engine, std::begin( array ), std::end( array ) );
            }

            template<typename Key, typename T, std::size_t N, typename A, typename B>
            static typename std::enable_if<std::is_same<Key, common>::value && !std::is_const<T>::value>::type
            fill( Engine& engine, T ( &array )[N], A from, B to ) {
                fill<Key>( engine, std::begin( array ), std::end( array ), from, to );
            }

            template<typename RandomIt>
            static void shuffle( Engine& engine, RandomIt first, RandomIt last ) {
                std::shuffle( first, last, engine );
            }

            template<typename Container>
            static typename std::enable_if<
                !std::is_array<typename std::remove_reference<Container>::type>::value &&
                details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
                !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
            shuffle( Engine& engine, Container&& container ) {
                shuffle( engine, std::begin( container ), std::end( container ) );
            }

            template<typename T, std::size_t N>
            static typename std::enable_if<!std::is_const<T>::value>::type
            shuffle( Engine& engine, T ( &array )[N] ) {
                shuffle( engine, std::begin( array ), std::end( array ) );
            }

            template<typename PopulationIt, typename SampleIt, typename Distance>
            static SampleIt sample( Engine& engine, PopulationIt first, PopulationIt last, SampleIt out, Distance n ) {
                if ( details::is_non_positive( n ) )
                    return out;
#if ILQVYA_CPLUSPLUS >= 201703L
                return std::sample( first, last, out, n, engine );
#else
                using pop_category = typename std::iterator_traits<PopulationIt>::iterator_category;
                return sample_impl( engine, first, last, pop_category( ), out, n );
#endif
            }

            template<template<typename...> class Container = std::vector, typename Range>
            ILQVYA_NODISCARD static auto sample( Engine& engine, const Range& range, std::size_t n ) ->
                typename std::enable_if<
                    details::is_iterator<decltype( std::begin( range ) )>::value,
                    Container<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
                using ValueType = typename std::iterator_traits<decltype( std::begin( range ) )>::value_type;
                Container<ValueType> result;
                const std::size_t pop_size = static_cast<std::size_t>( std::distance( std::begin( range ), std::end( range ) ) );
                const std::size_t actual_n = ( std::min )( n, pop_size );
                details::reserve_if_supported( result, actual_n );
                sample( engine, std::begin( range ), std::end( range ), details::make_inserter( result ), actual_n );
                return result;
            } // LCOV_EXCL_LINE

            template<template<typename...> class Container = std::vector, typename T, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<!details::is_supported_character<T>::value,
                                                            Container<typename std::remove_cv<T>::type>>::type
            sample( Engine& engine, const T ( &array )[N], std::size_t n ) {
                static_assert( N > 0, "Random::sample: built-in array must not be empty" );
                const std::size_t actual_n = ( std::min )( n, N );
                Container<typename std::remove_cv<T>::type> result;
                details::reserve_if_supported( result, actual_n );
                sample( engine, array, array + N, details::make_inserter( result ), actual_n );
                return result;
            } // LCOV_EXCL_LINE

            template<typename CharT, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                            std::basic_string<CharT>>::type
            sample( Engine& engine, const CharT ( &literal )[N], std::size_t n ) {
                static_assert( N > 1, "Random::sample: string literal must not be empty" );
                std::basic_string<CharT> result;
                const std::size_t actual_n = ( std::min )( n, N - 1 );
                result.reserve( actual_n );
                sample( engine, literal, literal + ( N - 1 ), std::back_inserter( result ), actual_n );
                return result;
            } // LCOV_EXCL_LINE

            template<typename Range>
            ILQVYA_NODISCARD static auto sample_string( Engine& engine, const Range& range, std::size_t n ) ->
                typename std::enable_if<
                    details::is_iterator<decltype( std::begin( range ) )>::value &&
                        details::is_supported_character<
                            typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>::value,
                    std::basic_string<
                        typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
                return sample<std::basic_string>( engine, range, n );
            }

            template<typename CharT, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                            std::basic_string<CharT>>::type
            sample_string( Engine& engine, const CharT ( &literal )[N], std::size_t n ) {
                return sample( engine, literal, n );
            }

#if ILQVYA_CPLUSPLUS < 201703L
        private:
            template<typename PopulationIt, typename SampleIt, typename Distance>
            static SampleIt sample_impl( Engine& engine, PopulationIt first, PopulationIt last,
                                         std::forward_iterator_tag, SampleIt out, Distance n ) {
                using dist_t = typename std::iterator_traits<PopulationIt>::difference_type;
                dist_t unsampled_sz = std::distance( first, last );
                if ( details::is_non_positive( unsampled_sz ) )
                    return out;
                dist_t needed = ( std::min )( static_cast<dist_t>( n ), unsampled_sz );
                if ( details::is_non_positive( needed ) )
                    return out;
                for ( ; needed > 0 && unsampled_sz > 0; ++first ) {
                    dist_t r = get<dist_t>( engine, dist_t( 0 ), --unsampled_sz );
                    if ( r < needed ) {
                        *out++ = *first;
                        --needed;
                    }
                }
                return out;
            }

            template<typename PopulationIt, typename SampleIt, typename Distance>
            static SampleIt sample_impl( Engine& engine, PopulationIt first, PopulationIt last,
                                         std::input_iterator_tag, SampleIt out, Distance n ) {
                using ValueType = typename std::iterator_traits<PopulationIt>::value_type;
                if ( details::is_non_positive( n ) )
                    return out;
                const std::size_t un = static_cast<std::size_t>( n );
                std::vector<ValueType> reservoir;
                reservoir.reserve( un );
                for ( ; first != last && reservoir.size( ) < un; ++first ) {
                    reservoir.push_back( *first );
                }
                std::size_t k = reservoir.size( );
                std::size_t i = k;
                for ( ; first != last; ++first, ++i ) {
                    std::size_t j = get<std::size_t>( engine, std::size_t( 0 ), i );
                    if ( j < k ) {
                        reservoir[j] = *first;
                    }
                }
                return std::copy( reservoir.begin( ), reservoir.end( ), out );
            } // LCOV_EXCL_LINE
        public:
#endif

            template<typename CharT = char>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                            std::basic_string<CharT>>::type
            get_string( Engine& engine, std::size_t length,
                        CharT from = details::default_char_range<CharT>::min_val( ),
                        CharT to = details::default_char_range<CharT>::max_val( ) ) {
                if ( from > to )
                    std::swap( from, to );
                if ( std::is_same<CharT, char32_t>::value && details::exceeds_unicode_max( to ) )
                    throw std::invalid_argument( "Random::get_string: 'to' exceeds maximum Unicode code point (0x10FFFF)" );
                if ( ( std::is_same<CharT, char16_t>::value || std::is_same<CharT, char32_t>::value ) &&
                     details::is_unicode_surrogate_range( from, to ) ) {
                    throw std::invalid_argument( "Random::get_string: specified range contains only Unicode surrogate code points" );
                }
                std::basic_string<CharT> result;
                result.reserve( length );
                for ( std::size_t i = 0; i < length; ++i ) {
                    CharT ch = get<CharT>( engine, from, to );
                    if ( std::is_same<CharT, char16_t>::value || std::is_same<CharT, char32_t>::value ) {
                        while ( details::is_unicode_surrogate( ch ) ) {
                            ch = get<CharT>( engine, from, to );
                        }
                    }
                    result.push_back( ch );
                }
                return result;
            } // LCOV_EXCL_LINE

            template<typename Container>
            ILQVYA_NODISCARD static auto get_string( Engine& engine, std::size_t length, const Container& charset ) ->
                typename std::enable_if<
                    details::is_iterator<decltype( std::begin( charset ) )>::value,
                    std::basic_string<typename std::iterator_traits<decltype( std::begin( charset ) )>::value_type>>::type {
                using CharT = typename std::iterator_traits<decltype( std::begin( charset ) )>::value_type;
                if ( std::begin( charset ) == std::end( charset ) )
                    throw std::invalid_argument( "Random::get_string: charset must not be empty" );
                std::basic_string<CharT> result;
                result.reserve( length );
                for ( std::size_t i = 0; i < length; ++i )
                    result.push_back( *get( engine, charset ) );
                return result;
            } // LCOV_EXCL_LINE

            template<typename CharT, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                            std::basic_string<CharT>>::type
            get_string( Engine& engine, std::size_t length, const CharT ( &literal )[N] ) {
                static_assert( N > 1, "Random::get_string: string literal charset must not be empty" );
                std::basic_string<CharT> result;
                result.reserve( length );
                for ( std::size_t i = 0; i < length; ++i ) {
                    const std::size_t idx = get<std::size_t>( engine, std::size_t( 0 ), static_cast<std::size_t>( N - 2 ) );
                    result.push_back( literal[idx] );
                }
                return result;
            } // LCOV_EXCL_LINE

            template<typename PopulationIt, typename OutIt, typename Distance>
            static OutIt choice( Engine& engine, PopulationIt first, PopulationIt last, OutIt out, Distance n ) {
                if ( details::is_non_positive( n ) )
                    return out;
                const auto size = std::distance( first, last );
                if ( details::is_non_positive( size ) )
                    throw std::invalid_argument( "Random::choice: population must not be empty" );
                using diff_t = typename std::iterator_traits<PopulationIt>::difference_type;
                for ( Distance i = 0; i < n; ++i ) {
                    const diff_t idx = get<diff_t>( engine, diff_t( 0 ), static_cast<diff_t>( size - 1 ) );
                    *out++ = *std::next( first, idx );
                }
                return out;
            }

            template<template<typename...> class Container = std::vector, typename Range>
            ILQVYA_NODISCARD static auto choice( Engine& engine, const Range& range, std::size_t n ) ->
                typename std::enable_if<
                    details::is_iterator<decltype( std::begin( range ) )>::value,
                    Container<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
                using ValueType = typename std::iterator_traits<decltype( std::begin( range ) )>::value_type;
                Container<ValueType> result;
                if ( n == 0 )
                    return result;
                if ( std::begin( range ) == std::end( range ) )
                    throw std::invalid_argument( "Random::choice: population must not be empty" );
                details::reserve_if_supported( result, n );
                choice( engine, std::begin( range ), std::end( range ), details::make_inserter( result ), n );
                return result;
            } // LCOV_EXCL_LINE

            template<template<typename...> class Container = std::vector, typename T, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<!details::is_supported_character<T>::value,
                                                            Container<typename std::remove_cv<T>::type>>::type
            choice( Engine& engine, const T ( &array )[N], std::size_t n ) {
                static_assert( N > 0, "Random::choice: built-in array must not be empty" );
                Container<typename std::remove_cv<T>::type> result;
                if ( n == 0 )
                    return result;
                details::reserve_if_supported( result, n );
                choice( engine, array, array + N, details::make_inserter( result ), n );
                return result;
            } // LCOV_EXCL_LINE

            template<typename CharT, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                            std::basic_string<CharT>>::type
            choice( Engine& engine, const CharT ( &literal )[N], std::size_t n ) {
                static_assert( N > 1, "Random::choice: string literal must not be empty" );
                std::basic_string<CharT> result;
                if ( n == 0 )
                    return result;
                result.reserve( n );
                choice( engine, literal, literal + ( N - 1 ), std::back_inserter( result ), n );
                return result;
            } // LCOV_EXCL_LINE

            template<typename Range>
            ILQVYA_NODISCARD static auto choice_string( Engine& engine, const Range& range, std::size_t n ) ->
                typename std::enable_if<
                    details::is_iterator<decltype( std::begin( range ) )>::value &&
                        details::is_supported_character<
                            typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>::value,
                    std::basic_string<
                        typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
                return choice<std::basic_string>( engine, range, n );
            }

            template<typename CharT, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                            std::basic_string<CharT>>::type
            choice_string( Engine& engine, const CharT ( &literal )[N], std::size_t n ) {
                return choice( engine, literal, n );
            }

        private:
            static std::string format_uuid_bytes( const std::uint8_t ( &bytes )[16] ) {
                static const char hex_chars[] = "0123456789abcdef";
                std::string s;
                s.resize( 36 );
                std::size_t out_idx = 0;
                for ( std::size_t i = 0; i < 16; ++i ) {
                    if ( i == 4 || i == 6 || i == 8 || i == 10 ) {
                        s[out_idx++] = '-';
                    }
                    s[out_idx++] = hex_chars[( bytes[i] >> 4 ) & 0x0f];
                    s[out_idx++] = hex_chars[bytes[i] & 0x0f];
                }
                return s;
            } // LCOV_EXCL_LINE

            static std::string format_mongo_id_bytes( const std::uint8_t ( &bytes )[12] ) {
                static const char hex_chars[] = "0123456789abcdef";
                std::string s;
                s.resize( 24 );
                for ( std::size_t i = 0; i < 12; ++i ) {
                    s[i * 2]     = hex_chars[( bytes[i] >> 4 ) & 0x0f];
                    s[i * 2 + 1] = hex_chars[bytes[i] & 0x0f];
                }
                return s;
            } // LCOV_EXCL_LINE

        public:
            ILQVYA_NODISCARD static std::string get_hex( Engine& engine, std::size_t length, bool uppercase = false ) {
                static const char lower_digits[] = "0123456789abcdef";
                static const char upper_digits[] = "0123456789ABCDEF";
                const char* digits = uppercase ? upper_digits : lower_digits;
                std::string result;
                result.reserve( length );
                for ( std::size_t i = 0; i < length; ++i ) {
                    result.push_back( digits[get<std::size_t>( engine, std::size_t( 0 ), std::size_t( 15 ) )] );
                }
                return result;
            } // LCOV_EXCL_LINE

            ILQVYA_NODISCARD static std::string get_uuid( Engine& engine, uuid_version version = uuid_version::v4 ) {
                switch ( version ) {
                    case uuid_version::nil:
                        return "00000000-0000-0000-0000-000000000000";
                    case uuid_version::max:
                        return "ffffffff-ffff-ffff-ffff-ffffffffffff";
                    case uuid_version::v4: {
                        std::uint8_t bytes[16]{ };
                        for ( std::size_t i = 0; i < 16; ++i ) {
                            bytes[i] = get<std::uint8_t>( engine );
                        }
                        bytes[6] = static_cast<std::uint8_t>( ( bytes[6] & 0x0f ) | 0x40 );
                        bytes[8] = static_cast<std::uint8_t>( ( bytes[8] & 0x3f ) | 0x80 );
                        return format_uuid_bytes( bytes );
                    }
                    case uuid_version::v7: {
                        std::uint8_t bytes[16]{ };
                        const auto ms = static_cast<std::uint64_t>(
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now( ).time_since_epoch( ) )
                                .count( ) );

                        // RFC 9562 Monotonicity sequence counter for sub-millisecond collisions.
                        // Packs 48 bits of timestamp ms with 16 bits of sequence state into a single atomic word
                        // to guarantee atomic monotonicity and race-free updates across concurrent callers.
                        static std::atomic<std::uint64_t> last_v7_state{ 0 };
                        const std::uint16_t seed_seq = static_cast<std::uint16_t>(
                            get<std::uint16_t>( engine, std::uint16_t( 0 ), std::uint16_t( 0x03ff ) ) );
                        std::uint64_t current = last_v7_state.load( std::memory_order_relaxed );
                        std::uint64_t final_ms = 0;
                        std::uint16_t final_seq = 0;
                        while ( true ) {
                            const std::uint64_t cur_ms = current >> 16;
                            const std::uint16_t cur_seq = static_cast<std::uint16_t>( current & 0xffff );
                            std::uint64_t next_ms = ms;
                            std::uint16_t next_seq = 0;
                            if ( ms > cur_ms ) {
                                next_ms = ms;
                                next_seq = static_cast<std::uint16_t>( seed_seq & 0x03ff );
                            } else {
                                if ( cur_seq >= 0x0fff ) {
                                    next_ms = cur_ms + 1;
                                    next_seq = 0;
                                } else {
                                    next_ms = cur_ms;
                                    next_seq = static_cast<std::uint16_t>( cur_seq + 1 );
                                }
                            }
                            const std::uint64_t next_state = ( next_ms << 16 ) | static_cast<std::uint64_t>( next_seq & 0x0fff );
                            if ( last_v7_state.compare_exchange_weak( current, next_state,
                                                                      std::memory_order_relaxed,
                                                                      std::memory_order_relaxed ) ) {
                                final_ms = next_ms;
                                final_seq = next_seq;
                                break;
                            }
                        }

                        bytes[0] = static_cast<std::uint8_t>( ( final_ms >> 40 ) & 0xff );
                        bytes[1] = static_cast<std::uint8_t>( ( final_ms >> 32 ) & 0xff );
                        bytes[2] = static_cast<std::uint8_t>( ( final_ms >> 24 ) & 0xff );
                        bytes[3] = static_cast<std::uint8_t>( ( final_ms >> 16 ) & 0xff );
                        bytes[4] = static_cast<std::uint8_t>( ( final_ms >> 8 ) & 0xff );
                        bytes[5] = static_cast<std::uint8_t>( final_ms & 0xff );
                        bytes[6] = static_cast<std::uint8_t>( ( ( final_seq >> 8 ) & 0x0f ) | 0x70 );
                        bytes[7] = static_cast<std::uint8_t>( final_seq & 0xff );

                        for ( std::size_t i = 8; i < 16; ++i ) {
                            bytes[i] = get<std::uint8_t>( engine );
                        }
                        bytes[8] = static_cast<std::uint8_t>( ( bytes[8] & 0x3f ) | 0x80 );
                        return format_uuid_bytes( bytes );
                    }
                    case uuid_version::v1: {
                        std::uint8_t bytes[16]{ };
                        const auto now_ns = static_cast<std::uint64_t>(
                            std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::system_clock::now( ).time_since_epoch( ) )
                                .count( ) );
                        const std::uint64_t intervals = ( now_ns / 100 ) + 0x01B21DD213814000ULL;
                        const std::uint32_t time_low = static_cast<std::uint32_t>( intervals & 0xffffffff );
                        const std::uint16_t time_mid = static_cast<std::uint16_t>( ( intervals >> 32 ) & 0xffff );
                        const std::uint16_t time_hi = static_cast<std::uint16_t>( ( intervals >> 48 ) & 0x0fff );

                        bytes[0] = static_cast<std::uint8_t>( ( time_low >> 24 ) & 0xff );
                        bytes[1] = static_cast<std::uint8_t>( ( time_low >> 16 ) & 0xff );
                        bytes[2] = static_cast<std::uint8_t>( ( time_low >> 8 ) & 0xff );
                        bytes[3] = static_cast<std::uint8_t>( time_low & 0xff );
                        bytes[4] = static_cast<std::uint8_t>( ( time_mid >> 8 ) & 0xff );
                        bytes[5] = static_cast<std::uint8_t>( time_mid & 0xff );
                        bytes[6] = static_cast<std::uint8_t>( ( ( time_hi >> 8 ) & 0x0f ) | 0x10 );
                        bytes[7] = static_cast<std::uint8_t>( time_hi & 0xff );

                        for ( std::size_t i = 8; i < 16; ++i ) {
                            bytes[i] = get<std::uint8_t>( engine );
                        }
                        bytes[8] = static_cast<std::uint8_t>( ( bytes[8] & 0x3f ) | 0x80 );
                        bytes[10] = static_cast<std::uint8_t>( bytes[10] | 0x01 );
                        return format_uuid_bytes( bytes );
                    }
                    case uuid_version::v6: {
                        std::uint8_t bytes[16]{ };
                        const auto now_ns = static_cast<std::uint64_t>(
                            std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::system_clock::now( ).time_since_epoch( ) )
                                .count( ) );
                        const std::uint64_t intervals = ( now_ns / 100 ) + 0x01B21DD213814000ULL;
                        const std::uint32_t time_high_32 = static_cast<std::uint32_t>( ( intervals >> 28 ) & 0xffffffff );
                        const std::uint16_t time_mid_16 = static_cast<std::uint16_t>( ( intervals >> 12 ) & 0xffff );
                        const std::uint16_t time_low_12 = static_cast<std::uint16_t>( intervals & 0x0fff );

                        bytes[0] = static_cast<std::uint8_t>( ( time_high_32 >> 24 ) & 0xff );
                        bytes[1] = static_cast<std::uint8_t>( ( time_high_32 >> 16 ) & 0xff );
                        bytes[2] = static_cast<std::uint8_t>( ( time_high_32 >> 8 ) & 0xff );
                        bytes[3] = static_cast<std::uint8_t>( time_high_32 & 0xff );
                        bytes[4] = static_cast<std::uint8_t>( ( time_mid_16 >> 8 ) & 0xff );
                        bytes[5] = static_cast<std::uint8_t>( time_mid_16 & 0xff );
                        bytes[6] = static_cast<std::uint8_t>( ( ( time_low_12 >> 8 ) & 0x0f ) | 0x60 );
                        bytes[7] = static_cast<std::uint8_t>( time_low_12 & 0xff );

                        for ( std::size_t i = 8; i < 16; ++i ) {
                            bytes[i] = get<std::uint8_t>( engine );
                        }
                        bytes[8] = static_cast<std::uint8_t>( ( bytes[8] & 0x3f ) | 0x80 );
                        bytes[10] = static_cast<std::uint8_t>( bytes[10] | 0x01 );
                        return format_uuid_bytes( bytes );
                    }
                    case uuid_version::mongo_object_id: {
                        std::uint8_t bytes[12]{ };
                        const auto sec = static_cast<std::uint32_t>(
                            std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::system_clock::now( ).time_since_epoch( ) )
                                .count( ) );
                        bytes[0] = static_cast<std::uint8_t>( ( sec >> 24 ) & 0xff );
                        bytes[1] = static_cast<std::uint8_t>( ( sec >> 16 ) & 0xff );
                        bytes[2] = static_cast<std::uint8_t>( ( sec >> 8 ) & 0xff );
                        bytes[3] = static_cast<std::uint8_t>( sec & 0xff );

                        static const std::array<std::uint8_t, 5> process_random = []( ) {
                            std::random_device rd;
                            std::array<std::uint8_t, 5> rnd{ };
                            for ( std::size_t i = 0; i < 5; ++i ) {
                                rnd[i] = static_cast<std::uint8_t>( rd( ) & 0xff );
                            }
                            return rnd;
                        }( );
                        bytes[4] = process_random[0];
                        bytes[5] = process_random[1];
                        bytes[6] = process_random[2];
                        bytes[7] = process_random[3];
                        bytes[8] = process_random[4];

                        static std::atomic<std::uint32_t> mongo_counter{ []( ) {
                            std::random_device rd;
                            return static_cast<std::uint32_t>( rd( ) & 0x00ffffff );
                        }( ) };
                        const std::uint32_t c = mongo_counter.fetch_add( 1, std::memory_order_relaxed ) & 0x00ffffff;
                        bytes[9]  = static_cast<std::uint8_t>( ( c >> 16 ) & 0xff );
                        bytes[10] = static_cast<std::uint8_t>( ( c >> 8 ) & 0xff );
                        bytes[11] = static_cast<std::uint8_t>( c & 0xff );

                        return format_mongo_id_bytes( bytes );
                    }
                    default:
                        throw std::invalid_argument( "Random::get_uuid: unsupported uuid version" );
                }
            } // LCOV_EXCL_LINE
        };

    } // namespace details

#define ILQVYA_WEIGHTED_VARIADIC_MEMBERS( ... )                                                                        \
    template<typename Target, typename Weight = double>                                                                \
    ILQVYA_NODISCARD __VA_ARGS__ Target get( details::weighted_result<Target, Weight> p1,                              \
                                             details::weighted_result<Target, Weight> p2 ) {                           \
        return get<Target, Weight>( { p1, p2 } );                                                                      \
    }                                                                                                                  \
    template<typename Target, typename Weight = double>                                                                \
    ILQVYA_NODISCARD __VA_ARGS__ Target get( details::weighted_result<Target, Weight> p1,                              \
                                             details::weighted_result<Target, Weight> p2,                              \
                                             details::weighted_result<Target, Weight> p3 ) {                           \
        return get<Target, Weight>( { p1, p2, p3 } );                                                                  \
    }                                                                                                                  \
    template<typename Target, typename Weight = double>                                                                \
    ILQVYA_NODISCARD __VA_ARGS__ Target get(                                                                           \
        details::weighted_result<Target, Weight> p1, details::weighted_result<Target, Weight> p2,                      \
        details::weighted_result<Target, Weight> p3, details::weighted_result<Target, Weight> p4 ) {                   \
        return get<Target, Weight>( { p1, p2, p3, p4 } );                                                              \
    }                                                                                                                  \
    template<typename Target, typename Weight = double>                                                                \
    ILQVYA_NODISCARD __VA_ARGS__ Target get(                                                                           \
        details::weighted_result<Target, Weight> p1, details::weighted_result<Target, Weight> p2,                      \
        details::weighted_result<Target, Weight> p3, details::weighted_result<Target, Weight> p4,                      \
        details::weighted_result<Target, Weight> p5 ) {                                                                \
        return get<Target, Weight>( { p1, p2, p3, p4, p5 } );                                                          \
    }                                                                                                                  \
    template<typename Target, typename Weight = double>                                                                \
    ILQVYA_NODISCARD __VA_ARGS__ Target get(                                                                           \
        details::weighted_result<Target, Weight> p1, details::weighted_result<Target, Weight> p2,                      \
        details::weighted_result<Target, Weight> p3, details::weighted_result<Target, Weight> p4,                      \
        details::weighted_result<Target, Weight> p5, details::weighted_result<Target, Weight> p6 ) {                   \
        return get<Target, Weight>( { p1, p2, p3, p4, p5, p6 } );                                                      \
    }                                                                                                                  \
    template<typename Target, typename Weight = double>                                                                \
    ILQVYA_NODISCARD __VA_ARGS__ Target get(                                                                           \
        details::weighted_result<Target, Weight> p1, details::weighted_result<Target, Weight> p2,                      \
        details::weighted_result<Target, Weight> p3, details::weighted_result<Target, Weight> p4,                      \
        details::weighted_result<Target, Weight> p5, details::weighted_result<Target, Weight> p6,                      \
        details::weighted_result<Target, Weight> p7 ) {                                                                \
        return get<Target, Weight>( { p1, p2, p3, p4, p5, p6, p7 } );                                                  \
    }                                                                                                                  \
    template<typename Target, typename Weight = double>                                                                \
    ILQVYA_NODISCARD __VA_ARGS__ Target get(                                                                           \
        details::weighted_result<Target, Weight> p1, details::weighted_result<Target, Weight> p2,                      \
        details::weighted_result<Target, Weight> p3, details::weighted_result<Target, Weight> p4,                      \
        details::weighted_result<Target, Weight> p5, details::weighted_result<Target, Weight> p6,                      \
        details::weighted_result<Target, Weight> p7, details::weighted_result<Target, Weight> p8 ) {                   \
        return get<Target, Weight>( { p1, p2, p3, p4, p5, p6, p7, p8 } );                                              \
    }                                                                                                                  \
    template<typename Target, typename Weight = double>                                                                \
    ILQVYA_NODISCARD __VA_ARGS__ Target get(                                                                           \
        details::weighted_result<Target, Weight> p1, details::weighted_result<Target, Weight> p2,                      \
        details::weighted_result<Target, Weight> p3, details::weighted_result<Target, Weight> p4,                      \
        details::weighted_result<Target, Weight> p5, details::weighted_result<Target, Weight> p6,                      \
        details::weighted_result<Target, Weight> p7, details::weighted_result<Target, Weight> p8,                      \
        details::weighted_result<Target, Weight> p9 ) {                                                                \
        return get<Target, Weight>( { p1, p2, p3, p4, p5, p6, p7, p8, p9 } );                                          \
    }                                                                                                                  \
    template<typename Target, typename Weight = double>                                                                \
    ILQVYA_NODISCARD __VA_ARGS__ Target get(                                                                           \
        details::weighted_result<Target, Weight> p1, details::weighted_result<Target, Weight> p2,                      \
        details::weighted_result<Target, Weight> p3, details::weighted_result<Target, Weight> p4,                      \
        details::weighted_result<Target, Weight> p5, details::weighted_result<Target, Weight> p6,                      \
        details::weighted_result<Target, Weight> p7, details::weighted_result<Target, Weight> p8,                      \
        details::weighted_result<Target, Weight> p9, details::weighted_result<Target, Weight> p10 ) {                  \
        return get<Target, Weight>( { p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 } );                                     \
    }

    /**
     * \brief Base template class for random with static API and static internal member storage
     */
    template<typename Derived, typename Engine, typename Seeder = seeder_default,
             template<typename> class IntegerDist = std::uniform_int_distribution,
             template<typename> class RealDist = std::uniform_real_distribution,
             typename BoolDist = std::bernoulli_distribution>
    class basic_random_base {
        using impl = details::random_impl<Engine, Seeder, IntegerDist, RealDist, BoolDist>;

    public:
        using engine_type = Engine;
        using seeder_type = Seeder;
        template<typename T>
        using integer_dist_t = IntegerDist<T>;
        template<typename T>
        using real_dist_t = RealDist<T>;
        using bool_dist_t = BoolDist;
        using common = details::common;
        using weight = details::weight;
        using uuid_version = ilqvya::uuid_version;
        template<typename Value, typename Weight = double>
        using weighted_result = details::weighted_result<Value, Weight>;

        constexpr basic_random_base( ) = default;

        ILQVYA_NODISCARD static constexpr typename Engine::result_type( min )( ) noexcept { return impl::min( ); }
        ILQVYA_NODISCARD static constexpr typename Engine::result_type( max )( ) noexcept { return impl::max( ); }
        static void discard( const unsigned long long z ) noexcept(
            noexcept( impl::discard( engine_instance( ), z ) ) ) {
            impl::discard( engine_instance( ), z );
        }
        static void reseed( ) { impl::reseed( engine_instance( ) ); }
        static void seed( const typename Engine::result_type value = Engine::default_seed ) noexcept(
            noexcept( impl::seed( engine_instance( ), value ) ) ) {
            impl::seed( engine_instance( ), value );
        }
        template<typename Sseq>
        static void seed( Sseq& seq ) noexcept( noexcept( impl::seed( engine_instance( ), seq ) ) ) {
            impl::seed( engine_instance( ), seq );
        }
        ILQVYA_NODISCARD static typename Engine::result_type get( ) noexcept(
            noexcept( impl::get( engine_instance( ) ) ) ) {
            return impl::get( engine_instance( ) );
        }
        ILQVYA_NODISCARD static bool is_equal( const Engine& other ) noexcept(
            noexcept( impl::is_equal( engine_instance( ), other ) ) ) {
            return impl::is_equal( engine_instance( ), other );
        }
        template<typename CharT, typename Traits>
        static void serialize( std::basic_ostream<CharT, Traits>& ost ) {
            impl::serialize( engine_instance( ), ost );
        }
        template<typename CharT, typename Traits>
        static void deserialize( std::basic_istream<CharT, Traits>& ist ) {
            impl::deserialize( engine_instance( ), ist );
        }

        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>& operator<<( std::basic_ostream<CharT, Traits>& ost,
                                                              const basic_random_base& rnd ) {
            rnd.serialize( ost );
            return ost;
        }

        template<typename CharT, typename Traits>
        friend std::basic_istream<CharT, Traits>& operator>>( std::basic_istream<CharT, Traits>& ist,
                                                              basic_random_base& rnd ) {
            rnd.deserialize( ist );
            return ist;
        }

        template<typename T>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_uniform_int<T>::value, T>::type
        get( T from = ( std::numeric_limits<T>::lowest )( ), T to = ( std::numeric_limits<T>::max )( ) ) {
            return impl::template get<T>( engine_instance( ), from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_uniform_real<T>::value, T>::type
        get( T from = T( 0 ), T to = T( 1 ) ) {
            return impl::template get<T>( engine_instance( ), from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_byte<T>::value, T>::type
        get( T from = details::byte_limits<T>::min( ), T to = details::byte_limits<T>::max( ) ) {
            return impl::template get<T>( engine_instance( ), from, to );
        }

        template<typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        ILQVYA_NODISCARD static
            typename std::enable_if<std::is_same<Key, common>::value &&
                                        details::is_valid_common_numbers<A, B>::value,
                                    C>::type
            get( A from = ( std::numeric_limits<A>::lowest )( ), B to = ( std::numeric_limits<B>::max )( ) ) {
            return impl::template get<Key, A, B, C>( engine_instance( ), from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<T>::value, T>::type
        get( T from = ( std::numeric_limits<T>::lowest )( ), T to = ( std::numeric_limits<T>::max )( ) ) {
            return impl::template get<T>( engine_instance( ), from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD static typename std::enable_if<std::is_same<T, bool>::value, bool>::type
        get( const double probability = 0.5 ) {
            return impl::template get<T>( engine_instance( ), probability );
        }

        ILQVYA_NODISCARD static bool flip( ) {
            return impl::flip( engine_instance( ) );
        }

        ILQVYA_NODISCARD static bool chance( const double probability ) {
            return impl::chance( engine_instance( ), probability );
        }

        ILQVYA_NODISCARD static bool chance_pct( const double percentage ) {
            return impl::chance_pct( engine_instance( ), percentage );
        }

        template<typename RealType = double, typename Mean = RealType, typename Stddev = RealType,
                 typename EffectiveReal = typename std::conditional<details::is_uniform_real<RealType>::value, RealType, double>::type>
        ILQVYA_NODISCARD static EffectiveReal normal( Mean mean = Mean( 0 ), Stddev stddev = Stddev( 1 ) ) {
            return impl::template normal<RealType, Mean, Stddev>( engine_instance( ), mean, stddev );
        }

        template<typename RealType = double, typename Lambda = RealType,
                 typename EffectiveReal = typename std::conditional<details::is_uniform_real<RealType>::value, RealType, double>::type>
        ILQVYA_NODISCARD static EffectiveReal exponential( Lambda lambda = Lambda( 1 ) ) {
            return impl::template exponential<RealType, Lambda>( engine_instance( ), lambda );
        }

        template<typename IntType = int, typename Mean = double>
        ILQVYA_NODISCARD static typename std::enable_if<std::is_integral<IntType>::value, IntType>::type
        poisson( Mean mean = Mean( 1 ) ) {
            return impl::template poisson<IntType, Mean>( engine_instance( ), mean );
        }

        template<typename T>
        ILQVYA_NODISCARD static T get( std::initializer_list<T> init_list ) {
            return impl::template get<T>( engine_instance( ), init_list );
        }

        template<typename ForwardIt>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_iterator<ForwardIt>::value, ForwardIt>::type
        get( ForwardIt first, ForwardIt last ) {
            return impl::get( engine_instance( ), first, last );
        }

        template<typename Container>
        ILQVYA_NODISCARD static auto get( Container& container ) ->
            typename std::enable_if<details::is_iterator<decltype( std::begin( container ) )>::value,
                                    decltype( std::begin( container ) )>::type {
            return impl::get( engine_instance( ), container );
        }

        template<typename Container>
        ILQVYA_NODISCARD static auto get( const Container& container ) ->
            typename std::enable_if<details::is_iterator<decltype( std::begin( container ) )>::value,
                                    decltype( std::begin( container ) )>::type {
            return impl::get( engine_instance( ), container );
        }

        template<typename Container>
        ILQVYA_NODISCARD static auto get( Container&& container ) ->
            typename std::enable_if<
                !std::is_lvalue_reference<Container>::value &&
                    details::is_iterator<decltype( std::begin( container ) )>::value,
                typename std::iterator_traits<decltype( std::begin( container ) )>::value_type>::type {
            return impl::get( engine_instance( ), std::forward<Container>( container ) );
        }

        template<template<typename...> class Container, typename A>
        ILQVYA_NODISCARD static Container<A> get( A from, A to, std::size_t size ) {
            return impl::template get<Container, A>( engine_instance( ), from, to, size );
        }

        template<template<typename...> class Container, typename Key, typename A, typename B,
                 typename C = typename std::common_type<A, B>::type>
        ILQVYA_NODISCARD static
            typename std::enable_if<std::is_same<Key, common>::value, Container<C>>::type
            get( A start, B end, std::size_t size ) {
            return impl::template get<Container, Key, A, B, C>( engine_instance( ), start, end, size );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A>
        ILQVYA_NODISCARD static
            typename std::enable_if<!details::has_insert<Container<A, N>>::value, Container<A, N>>::type
            get( A start, A end ) {
            return impl::template get<Container, N, A>( engine_instance( ), start, end );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename Key,
                 typename A, typename B, typename C = typename std::common_type<A, B>::type>
        ILQVYA_NODISCARD static
            typename std::enable_if<std::is_same<Key, common>::value && !details::has_insert<Container<C, N>>::value,
                                    Container<C, N>>::type
            get( A start, B end ) {
            return impl::template get<Container, N, Key, A, B, C>( engine_instance( ), start, end );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A = int>
        ILQVYA_NODISCARD static typename std::enable_if<!details::has_insert<Container<A, N>>::value, Container<A, N>>::type
        get( ) {
            return impl::template get<Container, N, A>( engine_instance( ) );
        }

        template<std::size_t N, typename T>
        ILQVYA_NODISCARD static std::array<T, N> get_array( T from, T to ) {
            return impl::template get_array<N, T>( engine_instance( ), from, to );
        }

        template<std::size_t N, typename Key, typename A, typename B,
                 typename C = typename std::common_type<A, B>::type>
        ILQVYA_NODISCARD static typename std::enable_if<std::is_same<Key, common>::value &&
                                                            details::is_valid_common_numbers<A, B>::value,
                                                        std::array<C, N>>::type
        get_array( A from, B to ) {
            return impl::template get_array<N, Key, A, B, C>( engine_instance( ), from, to );
        }

        template<std::size_t N, typename T = int>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_number<T>::value, std::array<T, N>>::type
        get_array( ) {
            return impl::template get_array<N, T>( engine_instance( ) );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                        const CharT*>::type
        get( const CharT ( &literal )[N] ) {
            static_assert( N > 1, "Random::get: string literal must not be empty" );
            return impl::get( engine_instance( ), literal );
        }

        template<typename T, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<!details::is_supported_character<T>::value, T*>::type
        get( T ( &array )[N] ) {
            static_assert( N > 0, "Random::get: built-in array must not be empty" );
            return impl::get( engine_instance( ), array );
        }

        template<typename T, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<!details::is_supported_character<T>::value,
                                                        typename std::remove_cv<T>::type>::type
        get( T ( &&array )[N] ) {
            static_assert( N > 0, "Random::get: built-in array must not be empty" );
            return impl::get( engine_instance( ), std::move( array ) );
        }

        template<typename Dist, typename... Args>
        ILQVYA_NODISCARD static typename Dist::result_type get( Args&&... args ) {
            return impl::template get<Dist>( engine_instance( ), std::forward<Args>( args )... );
        }

        template<typename Dist>
        ILQVYA_NODISCARD static typename std::decay<Dist>::type::result_type get( Dist&& dist ) {
            return impl::get( engine_instance( ), std::forward<Dist>( dist ) );
        }

        template<typename Key, class PairContainer>
        ILQVYA_NODISCARD static auto get( PairContainer& pair_container ) -> typename std::enable_if<
            details::is_pair_container<PairContainer>::value &&
                details::is_iterator<decltype( std::begin( pair_container ) )>::value &&
                ( details::is_uniform_int<typename PairContainer::value_type::second_type>::value ||
                  details::is_uniform_real<typename PairContainer::value_type::second_type>::value ) &&
                std::is_same<Key, details::weight>::value,
            decltype( std::begin( pair_container ) )>::type {
            return impl::template get<Key>( engine_instance( ), pair_container );
        }

        template<typename Key, class PairContainer>
        ILQVYA_NODISCARD static auto get( const PairContainer& pair_container ) -> typename std::enable_if<
            details::is_pair_container<PairContainer>::value &&
                details::is_iterator<decltype( std::begin( pair_container ) )>::value &&
                ( details::is_uniform_int<typename PairContainer::value_type::second_type>::value ||
                  details::is_uniform_real<typename PairContainer::value_type::second_type>::value ) &&
                std::is_same<Key, details::weight>::value,
            decltype( std::begin( pair_container ) )>::type {
            return impl::template get<Key>( engine_instance( ), pair_container );
        }

        template<typename Key, class PairContainer>
        static auto get( PairContainer&& ) -> typename std::enable_if<
            details::is_pair_container<typename std::remove_reference<PairContainer>::type>::value &&
                std::is_same<Key, details::weight>::value,
            void>::type = delete;

        template<typename Key, class PairContainer>
        static auto get( const PairContainer&& ) -> typename std::enable_if<
            details::is_pair_container<typename std::remove_reference<PairContainer>::type>::value &&
                std::is_same<Key, details::weight>::value,
            void>::type = delete;

        template<typename Target, typename Weight = double>
        ILQVYA_NODISCARD static Target get( std::initializer_list<details::weighted_result<Target, Weight>> list ) {
            return impl::template get<Target, Weight>( engine_instance( ), list );
        }

        ILQVYA_WEIGHTED_VARIADIC_MEMBERS( static )

#if defined( ILQVYA_HAS_REFLECTION )
        template<typename T>
        ILQVYA_NODISCARD static typename std::enable_if<std::is_enum<T>::value, T>::type get( ) {
            return impl::template get<T>( engine_instance( ) );
        }
#endif

        template<typename ForwardIt, typename T>
        static typename std::enable_if<details::is_iterator<ForwardIt>::value>::type
        fill( ForwardIt first, ForwardIt last, T from, T to ) {
            impl::fill( engine_instance( ), first, last, from, to );
        }

        template<typename ForwardIt>
        static typename std::enable_if<details::is_iterator<ForwardIt>::value>::type
        fill( ForwardIt first, ForwardIt last ) {
            impl::fill( engine_instance( ), first, last );
        }

        template<typename Key, typename ForwardIt, typename A, typename B>
        static typename std::enable_if<std::is_same<Key, common>::value && details::is_iterator<ForwardIt>::value>::type
        fill( ForwardIt first, ForwardIt last, A from, B to ) {
            impl::template fill<Key>( engine_instance( ), first, last, from, to );
        }

        template<typename Container, typename T>
        static typename std::enable_if<
            !std::is_array<typename std::remove_reference<Container>::type>::value &&
            details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
            !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
        fill( Container&& container, T from, T to ) {
            impl::fill( engine_instance( ), std::forward<Container>( container ), from, to );
        }

        template<typename Container>
        static typename std::enable_if<
            !std::is_array<typename std::remove_reference<Container>::type>::value &&
            details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
            !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
        fill( Container&& container ) {
            impl::fill( engine_instance( ), std::forward<Container>( container ) );
        }

        template<typename Key, typename Container, typename A, typename B>
        static typename std::enable_if<
            std::is_same<Key, common>::value &&
            !std::is_array<typename std::remove_reference<Container>::type>::value &&
            details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
            !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
        fill( Container&& container, A from, B to ) {
            impl::template fill<Key>( engine_instance( ), std::forward<Container>( container ), from, to );
        }

        template<typename T, std::size_t N, typename U>
        static typename std::enable_if<!std::is_const<T>::value>::type
        fill( T ( &array )[N], U from, U to ) {
            impl::fill( engine_instance( ), array, from, to );
        }

        template<typename T, std::size_t N>
        static typename std::enable_if<!std::is_const<T>::value>::type
        fill( T ( &array )[N] ) {
            impl::fill( engine_instance( ), array );
        }

        template<typename Key, typename T, std::size_t N, typename A, typename B>
        static typename std::enable_if<std::is_same<Key, common>::value && !std::is_const<T>::value>::type
        fill( T ( &array )[N], A from, B to ) {
            impl::template fill<Key>( engine_instance( ), array, from, to );
        }

        template<typename RandomIt>
        static void shuffle( RandomIt first, RandomIt last ) {
            impl::shuffle( engine_instance( ), first, last );
        }

        template<typename Container>
        static typename std::enable_if<
            !std::is_array<typename std::remove_reference<Container>::type>::value &&
            details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
            !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
        shuffle( Container&& container ) {
            impl::shuffle( engine_instance( ), std::forward<Container>( container ) );
        }

        template<typename T, std::size_t N>
        static typename std::enable_if<!std::is_const<T>::value>::type
        shuffle( T ( &array )[N] ) {
            impl::shuffle( engine_instance( ), array );
        }

        template<typename PopulationIt, typename SampleIt, typename Distance>
        static SampleIt sample( PopulationIt first, PopulationIt last, SampleIt out, Distance n ) {
            return impl::sample( engine_instance( ), first, last, out, n );
        }

        template<template<typename...> class Container = std::vector, typename Range>
        ILQVYA_NODISCARD static auto sample( const Range& range, std::size_t n ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( range ) )>::value,
            Container<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
            return impl::template sample<Container>( engine_instance( ), range, n );
        }

        template<template<typename...> class Container = std::vector, typename T, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<!details::is_supported_character<T>::value,
                                                        Container<typename std::remove_cv<T>::type>>::type
        sample( const T ( &array )[N], std::size_t n ) {
            return impl::template sample<Container>( engine_instance( ), array, n );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                        std::basic_string<CharT>>::type
        sample( const CharT ( &literal )[N], std::size_t n ) {
            return impl::sample( engine_instance( ), literal, n );
        }

        template<typename Range>
        ILQVYA_NODISCARD static auto sample_string( const Range& range, std::size_t n ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( range ) )>::value &&
                details::is_supported_character<
                    typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>::value,
            std::basic_string<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
            return impl::sample_string( engine_instance( ), range, n );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                        std::basic_string<CharT>>::type
        sample_string( const CharT ( &literal )[N], std::size_t n ) {
            return impl::sample_string( engine_instance( ), literal, n );
        }

        template<typename PopulationIt, typename OutIt, typename Distance>
        static OutIt choice( PopulationIt first, PopulationIt last, OutIt out, Distance n ) {
            return impl::choice( engine_instance( ), first, last, out, n );
        }

        template<template<typename...> class Container = std::vector, typename Range>
        ILQVYA_NODISCARD static auto choice( const Range& range, std::size_t n ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( range ) )>::value,
            Container<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
            return impl::template choice<Container>( engine_instance( ), range, n );
        }

        template<template<typename...> class Container = std::vector, typename T, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<!details::is_supported_character<T>::value,
                                                        Container<typename std::remove_cv<T>::type>>::type
        choice( const T ( &array )[N], std::size_t n ) {
            return impl::template choice<Container>( engine_instance( ), array, n );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                        std::basic_string<CharT>>::type
        choice( const CharT ( &literal )[N], std::size_t n ) {
            return impl::choice( engine_instance( ), literal, n );
        }

        template<typename Range>
        ILQVYA_NODISCARD static auto choice_string( const Range& range, std::size_t n ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( range ) )>::value &&
                details::is_supported_character<
                    typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>::value,
            std::basic_string<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
            return impl::choice_string( engine_instance( ), range, n );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                        std::basic_string<CharT>>::type
        choice_string( const CharT ( &literal )[N], std::size_t n ) {
            return impl::choice_string( engine_instance( ), literal, n );
        }

        template<typename CharT = char>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                        std::basic_string<CharT>>::type
        get_string( std::size_t length, CharT from = details::default_char_range<CharT>::min_val( ),
                    CharT to = details::default_char_range<CharT>::max_val( ) ) {
            return impl::template get_string<CharT>( engine_instance( ), length, from, to );
        }

        template<typename Container>
        ILQVYA_NODISCARD static auto get_string( std::size_t length, const Container& charset ) ->
            typename std::enable_if<
                details::is_iterator<decltype( std::begin( charset ) )>::value,
                std::basic_string<typename std::iterator_traits<decltype( std::begin( charset ) )>::value_type>>::type {
            return impl::get_string( engine_instance( ), length, charset );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                        std::basic_string<CharT>>::type
        get_string( std::size_t length, const CharT ( &literal )[N] ) {
            return impl::get_string( engine_instance( ), length, literal );
        }

        ILQVYA_NODISCARD static std::string get_hex( std::size_t length, bool uppercase = false ) {
            return impl::get_hex( engine_instance( ), length, uppercase );
        }

        ILQVYA_NODISCARD static std::string get_uuid( uuid_version version = uuid_version::v4 ) {
            return impl::get_uuid( engine_instance( ), version );
        }

        template<typename... Args>
        ILQVYA_NODISCARD auto operator( )( Args&&... args ) const -> decltype( get( std::forward<Args>( args )... ) ) {
            return get( std::forward<Args>( args )... );
        }

    private:
        static Engine& engine_instance( ) { return Derived::engine( ); }
    };

    /**
     * \brief Random with static API and static internal storage
     */
    template<typename Engine, typename Seeder = seeder_default,
             template<typename> class IntegerDist = std::uniform_int_distribution,
             template<typename> class RealDist = std::uniform_real_distribution,
             typename BoolDist = std::bernoulli_distribution>
    class basic_random_static
        : public basic_random_base<basic_random_static<Engine, Seeder, IntegerDist, RealDist, BoolDist>, Engine, Seeder,
                                   IntegerDist, RealDist, BoolDist> {
    public:
        using seeder_type = Seeder;
        template<typename T>
        using integer_dist_t = IntegerDist<T>;
        template<typename T>
        using real_dist_t = RealDist<T>;
        using bool_dist_t = BoolDist;
        using common = details::common;
        using weight = details::weight;
        using uuid_version = ilqvya::uuid_version;
        template<typename Value, typename Weight = double>
        using weighted_result = details::weighted_result<Value, Weight>;

        basic_random_static( ) = default;

        ILQVYA_NODISCARD static Engine get_engine( ) { return engine_instance( ); }
        static Engine& engine( ) { return engine_instance( ); }

    private:
        static Engine& engine_instance( ) {
            static Engine engine{ Seeder{ }( ) };
            return engine;
        }
    };

    /**
     * \brief Random with thread_local API and thread_local internal storage
     */
    template<typename Engine, typename Seeder = seeder_default,
             template<typename> class IntegerDist = std::uniform_int_distribution,
             template<typename> class RealDist = std::uniform_real_distribution,
             typename BoolDist = std::bernoulli_distribution>
    class basic_random_thread_local
        : public basic_random_base<basic_random_thread_local<Engine, Seeder, IntegerDist, RealDist, BoolDist>, Engine,
                                   Seeder, IntegerDist, RealDist, BoolDist> {
    public:
        using seeder_type = Seeder;
        template<typename T>
        using integer_dist_t = IntegerDist<T>;
        template<typename T>
        using real_dist_t = RealDist<T>;
        using bool_dist_t = BoolDist;
        using common = details::common;
        using weight = details::weight;
        using uuid_version = ilqvya::uuid_version;
        template<typename Value, typename Weight = double>
        using weighted_result = details::weighted_result<Value, Weight>;

        basic_random_thread_local( ) = default;

        ILQVYA_NODISCARD static Engine get_engine( ) { return engine_instance( ); }
        static Engine& engine( ) { return engine_instance( ); }

    private:
        static Engine& engine_instance( ) {
            thread_local Engine engine{ Seeder{ }( ) };
            return engine;
        }
    };

    /**
     * \brief Random with local API and local member storage
     */
    template<typename Engine, typename Seeder = seeder_default,
             template<typename> class IntegerDist = std::uniform_int_distribution,
             template<typename> class RealDist = std::uniform_real_distribution,
             typename BoolDist = std::bernoulli_distribution>
    class basic_random_local {
        using impl = details::random_impl<Engine, Seeder, IntegerDist, RealDist, BoolDist>;

    public:
        using engine_type = Engine;
        using seeder_type = Seeder;
        template<typename T>
        using integer_dist_t = IntegerDist<T>;
        template<typename T>
        using real_dist_t = RealDist<T>;
        using bool_dist_t = BoolDist;
        using common = details::common;
        using weight = details::weight;
        using uuid_version = ilqvya::uuid_version;
        template<typename Value, typename Weight = double>
        using weighted_result = details::weighted_result<Value, Weight>;

        basic_random_local( ) = default;

        explicit basic_random_local( const typename Engine::result_type seed_val ) noexcept(
            std::is_nothrow_constructible<Engine, typename Engine::result_type>::value )
            : m_engine( seed_val ) {}

        template<typename Sseq,
                 typename = typename std::enable_if<!std::is_same<typename std::decay<Sseq>::type, basic_random_local>::value &&
                                                    !std::is_same<typename std::decay<Sseq>::type, Engine>::value &&
                                                    !std::is_same<typename std::decay<Sseq>::type, Seeder>::value &&
                                                    !std::is_convertible<Sseq, typename Engine::result_type>::value>::type>
        explicit basic_random_local( Sseq& sseq ) noexcept(
            std::is_nothrow_constructible<Engine, Sseq&>::value )
            : m_engine( sseq ) {}

        explicit basic_random_local( Engine engine ) noexcept(
            std::is_nothrow_move_constructible<Engine>::value )
            : m_engine( std::move( engine ) ) {}

        explicit basic_random_local( Seeder& seeder ) : m_engine( make_seeded_engine( seeder ) ) {}

        basic_random_local( Engine engine, Seeder& seeder ) : m_engine( std::move( engine ) ) {
            auto&& seq = seeder( );
            m_engine.seed( seq );
        }

        ILQVYA_NODISCARD static constexpr typename Engine::result_type( min )( ) noexcept { return impl::min( ); }
        ILQVYA_NODISCARD static constexpr typename Engine::result_type( max )( ) noexcept { return impl::max( ); }
        void discard( const unsigned long long z ) noexcept(
            noexcept( impl::discard( m_engine, z ) ) ) {
            impl::discard( m_engine, z );
        }
        void reseed( ) { impl::reseed( m_engine ); }
        void seed( const typename Engine::result_type value = Engine::default_seed ) noexcept(
            noexcept( impl::seed( m_engine, value ) ) ) {
            impl::seed( m_engine, value );
        }
        template<typename Sseq>
        void seed( Sseq& seq ) noexcept( noexcept( impl::seed( m_engine, seq ) ) ) {
            impl::seed( m_engine, seq );
        }
        ILQVYA_NODISCARD typename Engine::result_type get( ) noexcept(
            noexcept( impl::get( m_engine ) ) ) {
            return impl::get( m_engine );
        }
        ILQVYA_NODISCARD bool is_equal( const Engine& other ) const noexcept(
            noexcept( impl::is_equal( m_engine, other ) ) ) {
            return impl::is_equal( m_engine, other );
        }
        ILQVYA_NODISCARD bool is_equal( const basic_random_local& other ) const noexcept(
            noexcept( is_equal( other.m_engine ) ) ) {
            return is_equal( other.m_engine );
        }

        friend bool operator==( const basic_random_local& lhs, const basic_random_local& rhs ) noexcept(
            noexcept( lhs.is_equal( rhs ) ) ) {
            return lhs.is_equal( rhs );
        }

        friend bool operator!=( const basic_random_local& lhs, const basic_random_local& rhs ) noexcept(
            noexcept( !( lhs == rhs ) ) ) {
            return !( lhs == rhs );
        }
        template<typename CharT, typename Traits>
        void serialize( std::basic_ostream<CharT, Traits>& ost ) const {
            impl::serialize( m_engine, ost );
        }
        template<typename CharT, typename Traits>
        void deserialize( std::basic_istream<CharT, Traits>& ist ) {
            impl::deserialize( m_engine, ist );
        }

        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>& operator<<( std::basic_ostream<CharT, Traits>& ost,
                                                              const basic_random_local& rnd ) {
            rnd.serialize( ost );
            return ost;
        }

        template<typename CharT, typename Traits>
        friend std::basic_istream<CharT, Traits>& operator>>( std::basic_istream<CharT, Traits>& ist,
                                                              basic_random_local& rnd ) {
            rnd.deserialize( ist );
            return ist;
        }

        template<typename T>
        ILQVYA_NODISCARD typename std::enable_if<details::is_uniform_int<T>::value, T>::type
        get( T from = ( std::numeric_limits<T>::lowest )( ), T to = ( std::numeric_limits<T>::max )( ) ) {
            return impl::template get<T>( m_engine, from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD typename std::enable_if<details::is_uniform_real<T>::value, T>::type get( T from = T( 0 ),
                                                                                                   T to = T( 1 ) ) {
            return impl::template get<T>( m_engine, from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD typename std::enable_if<details::is_byte<T>::value, T>::type
        get( T from = details::byte_limits<T>::min( ), T to = details::byte_limits<T>::max( ) ) {
            return impl::template get<T>( m_engine, from, to );
        }

        template<typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        ILQVYA_NODISCARD
            typename std::enable_if<std::is_same<Key, common>::value &&
                                        details::is_valid_common_numbers<A, B>::value,
                                    C>::type
            get( A from = ( std::numeric_limits<A>::lowest )( ), B to = ( std::numeric_limits<B>::max )( ) ) {
            return impl::template get<Key, A, B, C>( m_engine, from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<T>::value, T>::type
        get( T from = ( std::numeric_limits<T>::lowest )( ), T to = ( std::numeric_limits<T>::max )( ) ) {
            return impl::template get<T>( m_engine, from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD typename std::enable_if<std::is_same<T, bool>::value, bool>::type
        get( const double probability = 0.5 ) {
            return impl::template get<T>( m_engine, probability );
        }

        ILQVYA_NODISCARD bool flip( ) {
            return impl::flip( m_engine );
        }

        ILQVYA_NODISCARD bool chance( const double probability ) {
            return impl::chance( m_engine, probability );
        }

        ILQVYA_NODISCARD bool chance_pct( const double percentage ) {
            return impl::chance_pct( m_engine, percentage );
        }

        template<typename RealType = double, typename Mean = RealType, typename Stddev = RealType,
                 typename EffectiveReal = typename std::conditional<details::is_uniform_real<RealType>::value, RealType, double>::type>
        ILQVYA_NODISCARD EffectiveReal normal( Mean mean = Mean( 0 ), Stddev stddev = Stddev( 1 ) ) {
            return impl::template normal<RealType, Mean, Stddev>( m_engine, mean, stddev );
        }

        template<typename RealType = double, typename Lambda = RealType,
                 typename EffectiveReal = typename std::conditional<details::is_uniform_real<RealType>::value, RealType, double>::type>
        ILQVYA_NODISCARD EffectiveReal exponential( Lambda lambda = Lambda( 1 ) ) {
            return impl::template exponential<RealType, Lambda>( m_engine, lambda );
        }

        template<typename IntType = int, typename Mean = double>
        ILQVYA_NODISCARD typename std::enable_if<std::is_integral<IntType>::value, IntType>::type
        poisson( Mean mean = Mean( 1 ) ) {
            return impl::template poisson<IntType, Mean>( m_engine, mean );
        }

        template<typename T>
        ILQVYA_NODISCARD T get( std::initializer_list<T> init_list ) {
            return impl::template get<T>( m_engine, init_list );
        }

        template<typename ForwardIt>
        ILQVYA_NODISCARD typename std::enable_if<details::is_iterator<ForwardIt>::value, ForwardIt>::type
        get( ForwardIt first, ForwardIt last ) {
            return impl::get( m_engine, first, last );
        }

        template<typename Container>
        ILQVYA_NODISCARD auto get( Container& container ) ->
            typename std::enable_if<details::is_iterator<decltype( std::begin( container ) )>::value,
                                    decltype( std::begin( container ) )>::type {
            return impl::get( m_engine, container );
        }

        template<typename Container>
        ILQVYA_NODISCARD auto get( const Container& container ) ->
            typename std::enable_if<details::is_iterator<decltype( std::begin( container ) )>::value,
                                    decltype( std::begin( container ) )>::type {
            return impl::get( m_engine, container );
        }

        template<typename Container>
        ILQVYA_NODISCARD auto get( Container&& container ) ->
            typename std::enable_if<
                !std::is_lvalue_reference<Container>::value &&
                    details::is_iterator<decltype( std::begin( container ) )>::value,
                typename std::iterator_traits<decltype( std::begin( container ) )>::value_type>::type {
            return impl::get( m_engine, std::forward<Container>( container ) );
        }

        template<template<typename...> class Container, typename A>
        ILQVYA_NODISCARD Container<A> get( A from, A to, std::size_t size ) {
            return impl::template get<Container, A>( m_engine, from, to, size );
        }

        template<template<typename...> class Container, typename Key, typename A, typename B,
                 typename C = typename std::common_type<A, B>::type>
        ILQVYA_NODISCARD
            typename std::enable_if<std::is_same<Key, common>::value, Container<C>>::type
            get( A start, B end, std::size_t size ) {
            return impl::template get<Container, Key, A, B, C>( m_engine, start, end, size );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A>
        ILQVYA_NODISCARD typename std::enable_if<!details::has_insert<Container<A, N>>::value, Container<A, N>>::type
        get( A start, A end ) {
            return impl::template get<Container, N, A>( m_engine, start, end );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename Key,
                 typename A, typename B, typename C = typename std::common_type<A, B>::type>
        ILQVYA_NODISCARD
            typename std::enable_if<std::is_same<Key, common>::value && !details::has_insert<Container<C, N>>::value,
                                    Container<C, N>>::type
            get( A start, B end ) {
            return impl::template get<Container, N, Key, A, B, C>( m_engine, start, end );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A = int>
        ILQVYA_NODISCARD typename std::enable_if<!details::has_insert<Container<A, N>>::value, Container<A, N>>::type
        get( ) {
            return impl::template get<Container, N, A>( m_engine );
        }

        template<std::size_t N, typename T>
        ILQVYA_NODISCARD std::array<T, N> get_array( T from, T to ) {
            return impl::template get_array<N, T>( m_engine, from, to );
        }

        template<std::size_t N, typename Key, typename A, typename B,
                 typename C = typename std::common_type<A, B>::type>
        ILQVYA_NODISCARD typename std::enable_if<std::is_same<Key, common>::value &&
                                                    details::is_valid_common_numbers<A, B>::value,
                                                std::array<C, N>>::type
        get_array( A from, B to ) {
            return impl::template get_array<N, Key, A, B, C>( m_engine, from, to );
        }

        template<std::size_t N, typename T = int>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_number<T>::value, std::array<T, N>>::type
        get_array( ) {
            return impl::template get_array<N, T>( m_engine );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<CharT>::value, const CharT*>::type
        get( const CharT ( &literal )[N] ) {
            static_assert( N > 1, "Random::get: string literal must not be empty" );
            return impl::get( m_engine, literal );
        }

        template<typename T, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<!details::is_supported_character<T>::value, T*>::type
        get( T ( &array )[N] ) {
            static_assert( N > 0, "Random::get: built-in array must not be empty" );
            return impl::get( m_engine, array );
        }

        template<typename T, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<!details::is_supported_character<T>::value,
                                                typename std::remove_cv<T>::type>::type
        get( T ( &&array )[N] ) {
            static_assert( N > 0, "Random::get: built-in array must not be empty" );
            return impl::get( m_engine, std::move( array ) );
        }

        template<typename Dist, typename... Args>
        ILQVYA_NODISCARD typename Dist::result_type get( Args&&... args ) {
            return impl::template get<Dist>( m_engine, std::forward<Args>( args )... );
        }

        template<typename Dist>
        ILQVYA_NODISCARD typename std::decay<Dist>::type::result_type get( Dist&& dist ) {
            return impl::get( m_engine, std::forward<Dist>( dist ) );
        }

        template<typename Key, class PairContainer>
        ILQVYA_NODISCARD auto get( PairContainer& pair_container ) -> typename std::enable_if<
            details::is_pair_container<PairContainer>::value &&
                details::is_iterator<decltype( std::begin( pair_container ) )>::value &&
                ( details::is_uniform_int<typename PairContainer::value_type::second_type>::value ||
                  details::is_uniform_real<typename PairContainer::value_type::second_type>::value ) &&
                std::is_same<Key, details::weight>::value,
            decltype( std::begin( pair_container ) )>::type {
            return impl::template get<Key>( m_engine, pair_container );
        }

        template<typename Key, class PairContainer>
        ILQVYA_NODISCARD auto get( const PairContainer& pair_container ) -> typename std::enable_if<
            details::is_pair_container<PairContainer>::value &&
                details::is_iterator<decltype( std::begin( pair_container ) )>::value &&
                ( details::is_uniform_int<typename PairContainer::value_type::second_type>::value ||
                  details::is_uniform_real<typename PairContainer::value_type::second_type>::value ) &&
                std::is_same<Key, details::weight>::value,
            decltype( std::begin( pair_container ) )>::type {
            return impl::template get<Key>( m_engine, pair_container );
        }

        template<typename Key, class PairContainer>
        auto get( PairContainer&& ) -> typename std::enable_if<
            details::is_pair_container<typename std::remove_reference<PairContainer>::type>::value &&
                std::is_same<Key, details::weight>::value,
            void>::type = delete;

        template<typename Key, class PairContainer>
        auto get( const PairContainer&& ) -> typename std::enable_if<
            details::is_pair_container<typename std::remove_reference<PairContainer>::type>::value &&
                std::is_same<Key, details::weight>::value,
            void>::type = delete;

        template<typename Target, typename Weight = double>
        ILQVYA_NODISCARD Target get( std::initializer_list<details::weighted_result<Target, Weight>> list ) {
            return impl::template get<Target, Weight>( m_engine, list );
        }

        ILQVYA_WEIGHTED_VARIADIC_MEMBERS( )

#if defined( ILQVYA_HAS_REFLECTION )
        template<typename T>
        ILQVYA_NODISCARD typename std::enable_if<std::is_enum<T>::value, T>::type get( ) {
            return impl::template get<T>( m_engine );
        }
#endif

        template<typename ForwardIt, typename T>
        typename std::enable_if<details::is_iterator<ForwardIt>::value>::type
        fill( ForwardIt first, ForwardIt last, T from, T to ) {
            impl::fill( m_engine, first, last, from, to );
        }

        template<typename ForwardIt>
        typename std::enable_if<details::is_iterator<ForwardIt>::value>::type
        fill( ForwardIt first, ForwardIt last ) {
            impl::fill( m_engine, first, last );
        }

        template<typename Key, typename ForwardIt, typename A, typename B>
        typename std::enable_if<std::is_same<Key, common>::value && details::is_iterator<ForwardIt>::value>::type
        fill( ForwardIt first, ForwardIt last, A from, B to ) {
            impl::template fill<Key>( m_engine, first, last, from, to );
        }

        template<typename Container, typename T>
        typename std::enable_if<
            !std::is_array<typename std::remove_reference<Container>::type>::value &&
            details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
            !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
        fill( Container&& container, T from, T to ) {
            impl::fill( m_engine, std::forward<Container>( container ), from, to );
        }

        template<typename Container>
        typename std::enable_if<
            !std::is_array<typename std::remove_reference<Container>::type>::value &&
            details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
            !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
        fill( Container&& container ) {
            impl::fill( m_engine, std::forward<Container>( container ) );
        }

        template<typename Key, typename Container, typename A, typename B>
        typename std::enable_if<
            std::is_same<Key, common>::value &&
            !std::is_array<typename std::remove_reference<Container>::type>::value &&
            details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
            !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
        fill( Container&& container, A from, B to ) {
            impl::template fill<Key>( m_engine, std::forward<Container>( container ), from, to );
        }

        template<typename T, std::size_t N, typename U>
        typename std::enable_if<!std::is_const<T>::value>::type
        fill( T ( &array )[N], U from, U to ) {
            impl::fill( m_engine, array, from, to );
        }

        template<typename T, std::size_t N>
        typename std::enable_if<!std::is_const<T>::value>::type
        fill( T ( &array )[N] ) {
            impl::fill( m_engine, array );
        }

        template<typename Key, typename T, std::size_t N, typename A, typename B>
        typename std::enable_if<std::is_same<Key, common>::value && !std::is_const<T>::value>::type
        fill( T ( &array )[N], A from, B to ) {
            impl::template fill<Key>( m_engine, array, from, to );
        }

        template<typename RandomIt>
        void shuffle( RandomIt first, RandomIt last ) {
            impl::shuffle( m_engine, first, last );
        }

        template<typename Container>
        typename std::enable_if<
            !std::is_array<typename std::remove_reference<Container>::type>::value &&
            details::is_iterator<decltype( std::begin( std::declval<Container&>( ) ) )>::value &&
            !std::is_const<typename std::remove_reference<decltype( *std::begin( std::declval<Container&>( ) ) )>::type>::value>::type
        shuffle( Container&& container ) {
            impl::shuffle( m_engine, std::forward<Container>( container ) );
        }

        template<typename T, std::size_t N>
        typename std::enable_if<!std::is_const<T>::value>::type
        shuffle( T ( &array )[N] ) {
            impl::shuffle( m_engine, array );
        }

        template<typename PopulationIt, typename SampleIt, typename Distance>
        SampleIt sample( PopulationIt first, PopulationIt last, SampleIt out, Distance n ) {
            return impl::sample( m_engine, first, last, out, n );
        }

        template<template<typename...> class Container = std::vector, typename Range>
        ILQVYA_NODISCARD auto sample( const Range& range, std::size_t n ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( range ) )>::value,
            Container<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
            return impl::template sample<Container>( m_engine, range, n );
        }

        template<template<typename...> class Container = std::vector, typename T, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<!details::is_supported_character<T>::value,
                                                Container<typename std::remove_cv<T>::type>>::type
        sample( const T ( &array )[N], std::size_t n ) {
            return impl::template sample<Container>( m_engine, array, n );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<CharT>::value,
                                                std::basic_string<CharT>>::type
        sample( const CharT ( &literal )[N], std::size_t n ) {
            return impl::sample( m_engine, literal, n );
        }

        template<typename Range>
        ILQVYA_NODISCARD auto sample_string( const Range& range, std::size_t n ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( range ) )>::value &&
                details::is_supported_character<
                    typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>::value,
            std::basic_string<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
            return impl::sample_string( m_engine, range, n );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<CharT>::value,
                                                std::basic_string<CharT>>::type
        sample_string( const CharT ( &literal )[N], std::size_t n ) {
            return impl::sample_string( m_engine, literal, n );
        }

        template<typename PopulationIt, typename OutIt, typename Distance>
        OutIt choice( PopulationIt first, PopulationIt last, OutIt out, Distance n ) {
            return impl::choice( m_engine, first, last, out, n );
        }

        template<template<typename...> class Container = std::vector, typename Range>
        ILQVYA_NODISCARD auto choice( const Range& range, std::size_t n ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( range ) )>::value,
            Container<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
            return impl::template choice<Container>( m_engine, range, n );
        }

        template<template<typename...> class Container = std::vector, typename T, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<!details::is_supported_character<T>::value,
                                                Container<typename std::remove_cv<T>::type>>::type
        choice( const T ( &array )[N], std::size_t n ) {
            return impl::template choice<Container>( m_engine, array, n );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<CharT>::value,
                                                std::basic_string<CharT>>::type
        choice( const CharT ( &literal )[N], std::size_t n ) {
            return impl::choice( m_engine, literal, n );
        }

        template<typename Range>
        ILQVYA_NODISCARD auto choice_string( const Range& range, std::size_t n ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( range ) )>::value &&
                details::is_supported_character<
                    typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>::value,
            std::basic_string<typename std::iterator_traits<decltype( std::begin( range ) )>::value_type>>::type {
            return impl::choice_string( m_engine, range, n );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<CharT>::value,
                                                std::basic_string<CharT>>::type
        choice_string( const CharT ( &literal )[N], std::size_t n ) {
            return impl::choice_string( m_engine, literal, n );
        }

        template<typename CharT = char>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<CharT>::value,
                                                std::basic_string<CharT>>::type
        get_string( std::size_t length, CharT from = details::default_char_range<CharT>::min_val( ),
                    CharT to = details::default_char_range<CharT>::max_val( ) ) {
            return impl::template get_string<CharT>( m_engine, length, from, to );
        }

        template<typename Container>
        ILQVYA_NODISCARD auto get_string( std::size_t length, const Container& charset ) ->
            typename std::enable_if<
                details::is_iterator<decltype( std::begin( charset ) )>::value,
                std::basic_string<typename std::iterator_traits<decltype( std::begin( charset ) )>::value_type>>::type {
            return impl::get_string( m_engine, length, charset );
        }

        template<typename CharT, std::size_t N>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<CharT>::value,
                                                std::basic_string<CharT>>::type
        get_string( std::size_t length, const CharT ( &literal )[N] ) {
            return impl::get_string( m_engine, length, literal );
        }

        ILQVYA_NODISCARD std::string get_hex( std::size_t length, bool uppercase = false ) {
            return impl::get_hex( m_engine, length, uppercase );
        }

        ILQVYA_NODISCARD std::string get_uuid( uuid_version version = uuid_version::v4 ) {
            return impl::get_uuid( m_engine, version );
        }

        template<typename... Args>
        ILQVYA_NODISCARD auto operator( )( Args&&... args ) -> decltype( this->get( std::forward<Args>( args )... ) ) {
            return this->get( std::forward<Args>( args )... );
        }

        ILQVYA_NODISCARD Engine get_engine( ) const noexcept { return m_engine; }
        ILQVYA_NODISCARD const Engine& engine( ) const noexcept { return m_engine; }
        Engine& engine( ) noexcept { return m_engine; }

    private:
        static Engine make_seeded_engine( ) { return Engine{ Seeder{ }( ) }; }
        static Engine make_seeded_engine( Seeder& seeder ) { return Engine{ seeder( ) }; }
        Engine m_engine{ make_seeded_engine( ) };
    };

#undef ILQVYA_WEIGHTED_VARIADIC_MEMBERS

#if defined( __cpp_lib_philox_engine ) && ( __cpp_lib_philox_engine >= 202406L )
    /// C++26 default pseudo-random number engine (counter-based, optimal performance and state)
    using default_engine = std::philox4x64;
#else
    /// Default pseudo-random number engine for C++11 through C++23
    using default_engine = std::mt19937;
#endif

    /**
     * \brief The basic static random alias based on the default engine (std::philox4x64 in C++26, std::mt19937 fallback)
     */
    using random_static = basic_random_static<default_engine>;

    /**
     * \brief The basic thread_local random alias based on the default engine
     */
    using random_thread_local = basic_random_thread_local<default_engine>;

    /**
     * \brief The default random alias based on the default engine with thread_local storage
     */
    using random = random_thread_local;

    /**
     * \brief The basic local random alias based on the default engine
     */
    using random_local = basic_random_local<default_engine>;

    /**
     * \brief Convenient alias matching README and common usage
     */
    using Random = random;

} // namespace ilqvya

/// Backward compatibility namespace alias
namespace effolkronium = ilqvya;

#endif // #ifndef ILQVYA_RANDOM_HPP
