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

#if defined( _MSVC_LANG )
#    define ILQVYA_CPLUSPLUS _MSVC_LANG
#else
#    define ILQVYA_CPLUSPLUS __cplusplus
#endif

#if ILQVYA_CPLUSPLUS >= 201703L
#    include <string_view>
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
            static constexpr T min( ) noexcept { return ( std::numeric_limits<T>::min )( ); }
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

        template<typename Value, typename Weight = double>
        struct weighted_result {
            Value value;
            Weight weight;

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
            std::array<std::uint32_t, 8> data;
            seed_data( ) {
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
            get( Engine& engine, T from = ( std::numeric_limits<T>::min )( ),
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
                get( Engine& engine, A from = ( std::numeric_limits<A>::min )( ),
                     B to = ( std::numeric_limits<B>::max )( ) ) {
                return get( engine, static_cast<C>( from ), static_cast<C>( to ) );
            }

            template<typename T>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<T>::value, T>::type
            get( Engine& engine, T from = ( std::numeric_limits<T>::min )( ),
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
                if ( probability < 0.0 || probability > 1.0 )
                    throw std::invalid_argument( "Random::get<bool>: probability must be between 0.0 and 1.0" );
                return BoolDist{ probability }( engine );
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
                if ( 0 == size )
                    return last;
                using diff_t = typename std::iterator_traits<ForwardIt>::difference_type;
                return std::next( first, get<diff_t>( engine, 0, size - 1 ) );
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
            static auto get( Engine&, const Container&& ) -> typename std::enable_if<
                details::is_iterator<decltype( std::begin( std::declval<const Container&>( ) ) )>::value,
                void>::type = delete;

            template<template<typename...> class Container, typename A>
            ILQVYA_NODISCARD static Container<A>
            get( Engine& engine, A from, A to, std::size_t size ) {
                Container<A> container;
                details::reserve_if_supported( container, size );
                for ( std::size_t i = 0; i < size; ++i )
                    container.insert( std::end( container ), get( engine, from, to ) );
                return container;
            }

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
            }

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

            template<typename T, std::size_t N>
            ILQVYA_NODISCARD static T* get( Engine& engine, T ( &array )[N] ) {
                static_assert( N > 0, "Random::get: built-in array must not be empty" );
                return std::addressof( array[get<std::size_t>( engine, 0, N - 1 )] );
            }

            template<typename T, std::size_t N>
            static T* get( Engine&, const T ( &&array )[N] ) = delete;

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
            ILQVYA_NODISCARD static auto sample_weighted_container( Engine& engine, PairContainer& pair_container ) ->
                decltype( std::begin( pair_container ) ) {
                using WeightType = typename PairContainer::value_type::second_type;
                using IteratorType = decltype( std::begin( pair_container ) );

                WeightType total_weight = 0;
                IteratorType last_nonzero_it = std::end( pair_container );
                for ( IteratorType it = std::begin( pair_container ); it != std::end( pair_container ); ++it ) {
                    if ( it->second < WeightType( 0 ) )
                        throw std::invalid_argument( "Random::get: weights must be non-negative" );
                    if ( it->second > WeightType( 0 ) ) {
                        last_nonzero_it = it;
                    }
                    total_weight += it->second;
                }
                if ( total_weight <= WeightType( 0 ) )
                    return std::end( pair_container );

                WeightType random_weight = details::is_uniform_int<WeightType>::value
                                               ? get( engine, WeightType( 0 ), total_weight - 1 )
                                               : get( engine, WeightType( 0 ), total_weight );
                WeightType sum = 0;
                for ( IteratorType it = std::begin( pair_container ); it != last_nonzero_it; ++it ) {
                    sum += it->second;
                    if ( sum > random_weight )
                        return it;
                }
                return last_nonzero_it;
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
                    if ( item.weight < Weight( 0 ) )
                        throw std::invalid_argument( "Random::get: weights must be non-negative" );
                    if ( item.weight > Weight( 0 ) ) {
                        last_nonzero = &item;
                    }
                    total_weight += item.weight;
                }
                if ( total_weight == Weight( 0 ) ) {
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
                return last->value;
            }

            template<typename Target, typename Weight>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_uniform_real<Weight>::value, Target>::type
            get_weighted_list( Engine& engine, std::initializer_list<details::weighted_result<Target, Weight>> list ) {
                if ( 0u == list.size( ) )
                    throw std::invalid_argument( "Random::get: weighted list must not be empty" );
                Weight total_weight = 0;
                const details::weighted_result<Target, Weight>* last_nonzero = nullptr;
                for ( const auto& item : list ) {
                    if ( item.weight < Weight( 0 ) )
                        throw std::invalid_argument( "Random::get: weights must be non-negative" );
                    if ( item.weight > Weight( 0 ) ) {
                        last_nonzero = &item;
                    }
                    total_weight += item.weight;
                }
                if ( total_weight <= Weight( 0 ) ) {
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
                return last->value;
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

            template<typename RandomIt>
            static void shuffle( Engine& engine, RandomIt first, RandomIt last ) {
                std::shuffle( first, last, engine );
            }

            template<typename Container>
            static void shuffle( Engine& engine, Container& container ) {
                shuffle( engine, std::begin( container ), std::end( container ) );
            }

            template<typename Container>
            static void shuffle( Engine&, const Container&& ) = delete;

            template<typename T, std::size_t N>
            static void shuffle( Engine& engine, T ( &array )[N] ) {
                shuffle( engine, std::begin( array ), std::end( array ) );
            }

            template<typename T, std::size_t N>
            static void shuffle( Engine&, const T ( &&array )[N] ) = delete;

            template<typename PopulationIt, typename SampleIt, typename Distance>
            static SampleIt sample( Engine& engine, PopulationIt first, PopulationIt last, SampleIt out, Distance n ) {
                if ( n <= 0 )
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
                details::reserve_if_supported( result, n );
                sample( engine, std::begin( range ), std::end( range ), std::back_inserter( result ), n );
                return result;
            }

            template<template<typename...> class Container = std::vector, typename T, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<!details::is_supported_character<T>::value,
                                                            Container<typename std::remove_cv<T>::type>>::type
            sample( Engine& engine, const T ( &array )[N], std::size_t n ) {
                static_assert( N > 0, "Random::sample: built-in array must not be empty" );
                const std::size_t actual_n = ( std::min )( n, N );
                Container<typename std::remove_cv<T>::type> result;
                details::reserve_if_supported( result, actual_n );
                sample( engine, array, array + N, std::back_inserter( result ), actual_n );
                return result;
            }

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
            }

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
                                        std::random_access_iterator_tag, SampleIt out, Distance n ) {
                if ( n <= 0 )
                    return out;
                using dist_t = typename std::iterator_traits<PopulationIt>::difference_type;
                dist_t unsampled_sz = std::distance( first, last );
                for ( n = ( std::min )( static_cast<dist_t>( n ), unsampled_sz ); n != 0; ++first ) {
                    dist_t r = get<dist_t>( engine, 0, --unsampled_sz );
                    if ( r < n ) {
                        *out++ = *first;
                        --n;
                    }
                }
                return out;
            }

            template<typename PopulationIt, typename SampleIt, typename Distance>
            static SampleIt sample_impl( Engine& engine, PopulationIt first, PopulationIt last,
                                        std::input_iterator_tag, SampleIt out, Distance n ) {
                if ( n <= 0 )
                    return out;
                using ValueType = typename std::iterator_traits<PopulationIt>::value_type;
                std::vector<ValueType> reservoir;
                reservoir.reserve( static_cast<std::size_t>( n ) );
                for ( ; first != last && reservoir.size( ) < static_cast<std::size_t>( n ); ++first ) {
                    reservoir.push_back( *first );
                }
                std::size_t k = reservoir.size( );
                std::size_t i = k;
                for ( ; first != last; ++first, ++i ) {
                    std::size_t j = get<std::size_t>( engine, 0, i );
                    if ( j < k ) {
                        reservoir[j] = *first;
                    }
                }
                return std::copy( reservoir.begin( ), reservoir.end( ), out );
            }
        public:
#endif

            template<typename CharT>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                            std::basic_string<CharT>>::type
            get_string( Engine& engine, std::size_t length, CharT from = ( std::numeric_limits<CharT>::min )( ),
                        CharT to = ( std::numeric_limits<CharT>::max )( ) ) {
                std::basic_string<CharT> result;
                result.reserve( length );
                for ( std::size_t i = 0; i < length; ++i )
                    result.push_back( get<CharT>( engine, from, to ) );
                return result;
            }

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
            }

            template<typename CharT, std::size_t N>
            ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                            std::basic_string<CharT>>::type
            get_string( Engine& engine, std::size_t length, const CharT ( &literal )[N] ) {
                static_assert( N > 1, "Random::get_string: string literal charset must not be empty" );
                std::basic_string<CharT> result;
                result.reserve( length );
                for ( std::size_t i = 0; i < length; ++i ) {
                    const std::size_t idx = get<std::size_t>( engine, 0, N - 2 );
                    result.push_back( literal[idx] );
                }
                return result;
            }
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
        get( T from = ( std::numeric_limits<T>::min )( ), T to = ( std::numeric_limits<T>::max )( ) ) {
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
            get( A from = ( std::numeric_limits<A>::min )( ), B to = ( std::numeric_limits<B>::max )( ) ) {
            return impl::template get<Key, A, B, C>( engine_instance( ), from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<T>::value, T>::type
        get( T from = ( std::numeric_limits<T>::min )( ), T to = ( std::numeric_limits<T>::max )( ) ) {
            return impl::template get<T>( engine_instance( ), from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD static typename std::enable_if<std::is_same<T, bool>::value, bool>::type
        get( const double probability = 0.5 ) {
            return impl::template get<T>( engine_instance( ), probability );
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
        static auto get( const Container&& ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( std::declval<const Container&>( ) ) )>::value,
            void>::type = delete;

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

        template<typename T, std::size_t N>
        ILQVYA_NODISCARD static T* get( T ( &array )[N] ) {
            static_assert( N > 0, "Random::get: built-in array must not be empty" );
            return impl::get( engine_instance( ), array );
        }

        template<typename T, std::size_t N>
        static T* get( const T ( &&array )[N] ) = delete;

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

        template<typename RandomIt>
        static void shuffle( RandomIt first, RandomIt last ) {
            impl::shuffle( engine_instance( ), first, last );
        }

        template<typename Container>
        static void shuffle( Container& container ) {
            impl::shuffle( engine_instance( ), container );
        }

        template<typename Container>
        static void shuffle( const Container&& ) = delete;

        template<typename T, std::size_t N>
        static void shuffle( T ( &array )[N] ) {
            impl::shuffle( engine_instance( ), array );
        }

        template<typename T, std::size_t N>
        static void shuffle( const T ( &&array )[N] ) = delete;

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

        template<typename CharT>
        ILQVYA_NODISCARD static typename std::enable_if<details::is_supported_character<CharT>::value,
                                                        std::basic_string<CharT>>::type
        get_string( std::size_t length, CharT from = ( std::numeric_limits<CharT>::min )( ),
                    CharT to = ( std::numeric_limits<CharT>::max )( ) ) {
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

        template<typename... Args>
        ILQVYA_NODISCARD auto operator( )( Args&&... args ) const -> decltype( get( std::forward<Args>( args )... ) ) {
            return get( std::forward<Args>( args )... );
        }

    private:
        static Engine& engine_instance( ) noexcept { return Derived::engine( ); }
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
        template<typename Value, typename Weight = double>
        using weighted_result = details::weighted_result<Value, Weight>;

        basic_random_static( ) = default;

        ILQVYA_NODISCARD static Engine get_engine( ) noexcept { return engine_instance( ); }
        static Engine& engine( ) noexcept { return engine_instance( ); }

    private:
        static Engine& engine_instance( ) noexcept {
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
        template<typename Value, typename Weight = double>
        using weighted_result = details::weighted_result<Value, Weight>;

        basic_random_thread_local( ) = default;

        ILQVYA_NODISCARD static Engine get_engine( ) noexcept { return engine_instance( ); }
        static Engine& engine( ) noexcept { return engine_instance( ); }

    private:
        static Engine& engine_instance( ) noexcept {
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
        get( T from = ( std::numeric_limits<T>::min )( ), T to = ( std::numeric_limits<T>::max )( ) ) {
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
            get( A from = ( std::numeric_limits<A>::min )( ), B to = ( std::numeric_limits<B>::max )( ) ) {
            return impl::template get<Key, A, B, C>( m_engine, from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<T>::value, T>::type
        get( T from = ( std::numeric_limits<T>::min )( ), T to = ( std::numeric_limits<T>::max )( ) ) {
            return impl::template get<T>( m_engine, from, to );
        }

        template<typename T>
        ILQVYA_NODISCARD typename std::enable_if<std::is_same<T, bool>::value, bool>::type
        get( const double probability = 0.5 ) {
            return impl::template get<T>( m_engine, probability );
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
        auto get( const Container&& ) -> typename std::enable_if<
            details::is_iterator<decltype( std::begin( std::declval<const Container&>( ) ) )>::value,
            void>::type = delete;

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

        template<typename T, std::size_t N>
        ILQVYA_NODISCARD T* get( T ( &array )[N] ) {
            static_assert( N > 0, "Random::get: built-in array must not be empty" );
            return impl::get( m_engine, array );
        }

        template<typename T, std::size_t N>
        T* get( const T ( &&array )[N] ) = delete;

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

        template<typename RandomIt>
        void shuffle( RandomIt first, RandomIt last ) {
            impl::shuffle( m_engine, first, last );
        }

        template<typename Container>
        void shuffle( Container& container ) {
            impl::shuffle( m_engine, container );
        }

        template<typename Container>
        void shuffle( const Container&& ) = delete;

        template<typename T, std::size_t N>
        void shuffle( T ( &array )[N] ) {
            impl::shuffle( m_engine, array );
        }

        template<typename T, std::size_t N>
        void shuffle( const T ( &&array )[N] ) = delete;

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

        template<typename CharT>
        ILQVYA_NODISCARD typename std::enable_if<details::is_supported_character<CharT>::value,
                                                std::basic_string<CharT>>::type
        get_string( std::size_t length, CharT from = ( std::numeric_limits<CharT>::min )( ),
                    CharT to = ( std::numeric_limits<CharT>::max )( ) ) {
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

} // namespace ilqvya

/// Backward compatibility namespace alias
namespace effolkronium = ilqvya;

#endif // #ifndef ILQVYA_RANDOM_HPP
