/*
______  ___   _   _______ ________  __
| ___ \/ _ \ | \ | |  _  \  _  |  \/  | Random for modern C++
| |_/ / /_\ \|  \| | | | | | | | .  . |
|    /|  _  || . ` | | | | | | | |\/| | version 1.5.0
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

#ifndef EFFOLKRONIUM_RANDOM_HPP
#define EFFOLKRONIUM_RANDOM_HPP

#include <random>
#include <chrono> // timed seed
#include <type_traits>
#include <cassert>
#include <initializer_list>
#include <utility> // std::forward, std::declval
#include <algorithm> // std::shuffle, std::next, std::distance
#include <iterator> // std::begin, std::end, std::iterator_traits
#include <limits> // std::numeric_limits
#include <ostream>
#include <istream>

namespace effolkronium {

    namespace details {
        /// Key type for getting common type numbers or objects
        struct common{ };

        /// Key type for weighted random number generation
        struct weight{ };

        /// True if type T is applicable by a std::uniform_int_distribution
        template<typename T>
        struct is_uniform_int {
            static constexpr bool value =
                   std::is_same<T,              short>::value
                || std::is_same<T,                int>::value
                || std::is_same<T,               long>::value
                || std::is_same<T,          long long>::value
                || std::is_same<T,     unsigned short>::value
                || std::is_same<T,       unsigned int>::value
                || std::is_same<T,      unsigned long>::value
                || std::is_same<T, unsigned long long>::value;
        };

        /// True if type T is applicable by a std::uniform_real_distribution
        template<typename T>
        struct is_uniform_real {
            static constexpr bool value =
                   std::is_same<T,       float>::value
                || std::is_same<T,      double>::value
                || std::is_same<T, long double>::value;
        };

        /// True if type T is plain byte
        template<typename T>
        struct is_byte {
            static constexpr bool value =
                   std::is_same<T,   signed char>::value
                || std::is_same<T, unsigned char>::value;
        };

        /// True if type T is plain number type
        template<typename T>
        struct is_supported_number {
            static constexpr bool value =
                   is_byte        <T>::value
                || is_uniform_real<T>::value
                || is_uniform_int <T>::value;
        };

        /// True if type T is character type
        template<typename T>
        struct is_supported_character {
            static constexpr bool value =
                   std::is_same<T, char>::value
                || std::is_same<T, wchar_t>::value
                || std::is_same<T, char16_t>::value
                || std::is_same<T, char32_t>::value;
        };

        /// True if type T is iterator
        template<typename T>
        struct is_iterator {
        private:
            static char test( ... );

            template <typename U,
                typename = typename std::iterator_traits<U>::difference_type,
                typename = typename std::iterator_traits<U>::pointer,
                typename = typename std::iterator_traits<U>::reference,
                typename = typename std::iterator_traits<U>::value_type,
                typename = typename std::iterator_traits<U>::iterator_category
            > static long test( U&& );
        public:
            static constexpr bool value = std::is_same<
                decltype( test( std::declval<T>( ) ) ), long>::value;
        };

        template <typename T>
        class has_reserve
        {
        private:
            template <typename C>
            static char test(...);

            template <typename C>
            static long test(decltype(&C::reserve));
        public:
            static constexpr bool value = std::is_same<
                decltype(test<T>(0)), long>::value;
        };

        template <typename T>
        class has_insert
        {
        private:
            template <typename C>
            static char test(...);

            template <typename C>
            static long test(decltype(&C::insert));
        public:
            static constexpr bool value = std::is_same<
                decltype(test<T>(0)), long>::value;
        };

        template<typename...>
        using void_t = void;

        template<typename Type, typename = void>
        struct is_map : public std::false_type  {};

        template<typename Type>
        struct is_map<Type, void_t<typename Type::key_type, typename Type::mapped_type, typename Type::value_type>> : public std::true_type{};

    } // namespace details

    /// Default seeder for 'random' classes
    struct seeder_default {
        /// return seed sequence
        std::seed_seq& operator() ( ) {
            // MinGW issue, std::random_device returns constant value
            // Use std::seed_seq with additional seed from C++ chrono
            return seed_seq;
        }
    private:
        std::seed_seq seed_seq{ {
                static_cast<std::uintmax_t>( std::random_device{ }( ) ),
                static_cast<std::uintmax_t>( std::chrono::steady_clock::now( )
                                             .time_since_epoch( ).count( ) ),
        } };
    };

    namespace details {

        /**
        * \brief Stateless core implementation containing algorithms taking Engine& engine
        */
        template<
            typename Engine,
            typename Seeder = seeder_default,
            template<typename> class IntegerDist = std::uniform_int_distribution,
            template<typename> class RealDist = std::uniform_real_distribution,
            typename BoolDist = std::bernoulli_distribution
        >
        struct random_impl {
            using engine_type = Engine;
            using seeder_type = Seeder;
            template<typename T> using integer_dist_t = IntegerDist<T>;
            template<typename T> using real_dist_t = RealDist<T>;
            using bool_dist_t = BoolDist;
            using common = details::common;
            using weight = details::weight;

            static constexpr typename Engine::result_type (min)( ) {
                return (Engine::min)( );
            }

            static constexpr typename Engine::result_type (max)( ) {
                return (Engine::max)( );
            }

            static void discard( Engine& engine, const unsigned long long z ) {
                engine.discard( z );
            }

            static void reseed( Engine& engine ) {
                Seeder seeder;
                seed( engine, seeder( ) );
            }

            static void seed( Engine& engine, const typename Engine::result_type value = Engine::default_seed ) {
                engine.seed( value );
            }

            template<typename Sseq>
            static void seed( Engine& engine, Sseq& seq ) {
                engine.seed( seq );
            }

            static typename Engine::result_type get( Engine& engine ) {
                return engine( );
            }

            static bool is_equal( const Engine& engine, const Engine& other ) {
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
            static typename std::enable_if<details::is_uniform_int<T>::value, T>::type
            get( Engine& engine, T from = (std::numeric_limits<T>::min)( ), T to = (std::numeric_limits<T>::max)( ) ) {
                if( from < to )
                    return IntegerDist<T>{ from, to }( engine );
                return IntegerDist<T>{ to, from }( engine );
            }

            template<typename T>
            static typename std::enable_if<details::is_uniform_real<T>::value, T>::type
            get( Engine& engine, T from = (std::numeric_limits<T>::min)( ), T to = (std::numeric_limits<T>::max)( ) ) {
                if( from < to )
                    return RealDist<T>{ from, to }( engine );
                return RealDist<T>{ to, from }( engine );
            }

            template<typename T>
            static typename std::enable_if<details::is_byte<T>::value, T>::type
            get( Engine& engine, T from = (std::numeric_limits<T>::min)( ), T to = (std::numeric_limits<T>::max)( ) ) {
                using short_t = typename std::conditional<std::is_signed<T>::value, short, unsigned short>::type;
                return static_cast<T>( get<short_t>( engine, from, to ) );
            }

            template<typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
            static typename std::enable_if<
                   std::is_same<Key, common>::value
                && details::is_supported_number<A>::value
                && details::is_supported_number<B>::value
                && std::is_signed<A>::value != std::is_unsigned<B>::value
                , C>::type
            get( Engine& engine, A from = (std::numeric_limits<A>::min)( ), B to = (std::numeric_limits<B>::max)( ) ) {
                return get( engine, static_cast<C>( from ), static_cast<C>( to ) );
            }

            template<typename T>
            static typename std::enable_if<details::is_supported_character<T>::value, T>::type
            get( Engine& engine, T from = (std::numeric_limits<T>::min)(), T to = (std::numeric_limits<T>::max)() ) {
                if (from < to)
                    return static_cast<T>(IntegerDist<std::int64_t>{ static_cast<std::int64_t>(from), static_cast<std::int64_t>(to) }(engine));
                return static_cast<T>(IntegerDist<std::int64_t>{ static_cast<std::int64_t>(to), static_cast<std::int64_t>(from) }(engine));
            }

            template<typename T>
            static typename std::enable_if<std::is_same<T, bool>::value, bool>::type
            get( Engine& engine, const double probability = 0.5 ) {
                assert( 0 <= probability && 1 >= probability );
                return BoolDist{ probability }( engine );
            }

            template<typename T>
            static T get( Engine& engine, std::initializer_list<T> init_list ) {
                assert( 0u != init_list.size( ) );
                return *get( engine, init_list.begin( ), init_list.end( ) );
            }

            template<typename InputIt>
            static typename std::enable_if<details::is_iterator<InputIt>::value, InputIt>::type
            get( Engine& engine, InputIt first, InputIt last ) {
                const auto size = std::distance( first, last );
                if( 0 == size ) return last;
                using diff_t = typename std::iterator_traits<InputIt>::difference_type;
                return std::next( first, get<diff_t>( engine, 0, size - 1 ) );
            }

            template<typename Container>
            static auto get( Engine& engine, Container& container ) ->
                typename std::enable_if<details::is_iterator<decltype(std::begin(container))>::value, decltype(std::begin(container))>::type {
                return get( engine, std::begin( container ), std::end( container ) );
            }

            template<template<typename...> class Container, typename A>
            static typename std::enable_if<details::has_reserve<Container<A>>::value, Container<A>>::type
            get( Engine& engine, A from, A to, std::size_t size) {
                Container<A> container;
                container.reserve(size);
                for (std::size_t i = 0; i < size; ++i)
                    container.insert(std::end(container), get(engine, from, to));
                return container;
            }

            template<template<typename...> class Container, typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
            static typename std::enable_if<std::is_same<Key, common>::value && details::has_reserve<Container<A>>::value, Container<C>>::type
            get( Engine& engine, A start, B end, std::size_t size) {
                Container<C> container;
                container.reserve(size);
                for (std::size_t i = 0; i < size; ++i)
                    container.insert(std::end(container), get<common>(engine, start, end));
                return container;
            }

            template<template<typename...> class Container, typename A>
            static typename std::enable_if<!details::has_reserve<Container<A>>::value, Container<A>>::type
            get( Engine& engine, A start, A end, std::size_t size) {
                Container<A> container;
                for (std::size_t i = 0; i < size; ++i)
                    container.insert(std::end(container), get(engine, start, end));
                return container;
            }

            template<template<typename...> class Container, typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
            static typename std::enable_if<std::is_same<Key, common>::value && !details::has_reserve<Container<C>>::value, Container<C>>::type
            get( Engine& engine, A start, B end, std::size_t size) {
                Container<C> container;
                for (std::size_t i = 0; i < size; ++i)
                    container.insert(std::end(container), get<common>(engine, start, end));
                return container;
            }

            template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A>
            static typename std::enable_if<!details::has_insert<Container<A, N>>::value, Container<A, N>>::type
            get( Engine& engine, A start, A end) {
                Container<A, N> container = {{ 0 }};
                for (std::size_t i = 0; i < N; ++i)
                    container[i] = get(engine, start, end);
                return container;
            }

            template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
            static typename std::enable_if<std::is_same<Key, common>::value && !details::has_insert<Container<C, N>>::value, Container<C, N>>::type
            get( Engine& engine, A start, B end) {
                Container<C, N> container = {{ 0 }};
                for (std::size_t i = 0; i < N; ++i)
                    container[i] = get<common>(engine, start, end);
                return container;
            }

            template<typename T, std::size_t N>
            static T* get( Engine& engine, T( &array )[ N ] ) {
                return std::addressof( array[ get<std::size_t>( engine, 0, N - 1 ) ] );
            }

            template<typename Dist, typename... Args>
            static typename Dist::result_type get( Engine& engine, Args&&... args ) {
                return Dist{ std::forward<Args>( args )... }( engine );
            }

            template<typename Dist>
            static typename Dist::result_type get( Engine& engine, Dist& dist ) {
                return dist( engine );
            }

            template<typename Key, class MapContainer>
            static auto get(Engine& engine, const MapContainer& map_container) ->
                typename std::enable_if<
                    details::is_map<MapContainer>::value &&
                    details::is_iterator<decltype(std::begin(map_container))>::value &&
                    !std::is_signed<typename MapContainer::mapped_type>::value &&
                    std::is_same<Key, details::weight>::value,
                decltype(std::begin(map_container))>::type {
                using MappedType = typename MapContainer::mapped_type;
                using IteratorType = decltype(std::begin(map_container));
                
                MappedType total_weight = 0;
                for (IteratorType it = std::begin(map_container); it != std::end(map_container); ++it)
                    total_weight += it->second;
                if(total_weight == MappedType(0)) return std::end(map_container);

                MappedType random_weight = get(engine, MappedType(0), total_weight - 1);
                MappedType sum = 0;
                for(IteratorType it = std::begin(map_container); it != std::end(map_container); ++it) {
                    sum += it->second;
                    if(sum > random_weight) return it;
                }
                return std::end(map_container);
            }

            template<typename Key, class MapContainer>
            static auto get(Engine& engine, const MapContainer& map_container) ->
                typename std::enable_if<
                    details::is_map<MapContainer>::value &&
                    details::is_iterator<decltype(std::begin(map_container))>::value &&
                    details::is_uniform_real<typename MapContainer::mapped_type>::value &&
                    std::is_same<Key, details::weight>::value,
                decltype(std::begin(map_container))>::type {
                using MappedType = typename MapContainer::mapped_type;
                using IteratorType = decltype(std::begin(map_container));
                
                MappedType total_weight = 0;
                for (IteratorType it = std::begin(map_container); it != std::end(map_container); ++it) {
                    assert(it->second >= MappedType(0));
                    total_weight += it->second;
                }
                if(total_weight == MappedType(0)) return std::end(map_container);

                MappedType random_weight = get(engine, MappedType(0), std::nextafter(total_weight, (std::numeric_limits<MappedType>::min)( )));
                MappedType sum = 0;
                for(IteratorType it = std::begin(map_container); it != std::end(map_container); ++it) {
                    sum += it->second;
                    if(sum > random_weight) return it;
                }
                return std::end(map_container);
            }

            template<typename RandomIt>
            static void shuffle( Engine& engine, RandomIt first, RandomIt last ) {
                std::shuffle( first, last, engine );
            }

            template<typename Container>
            static void shuffle( Engine& engine, Container& container ) {
                shuffle( engine, std::begin( container ), std::end( container ) );
            }
        };

    } // namespace details

    /**
    * \brief Base template class for random with static API and static internal member storage
    */
    template<
        typename Derived,
        typename Engine,
        typename Seeder = seeder_default,
        template<typename> class IntegerDist = std::uniform_int_distribution,
        template<typename> class RealDist = std::uniform_real_distribution,
        typename BoolDist = std::bernoulli_distribution
    >
    class basic_random_base {
        using impl = details::random_impl<Engine, Seeder, IntegerDist, RealDist, BoolDist>;
    public:
        using engine_type = Engine;
        using seeder_type = Seeder;
        template<typename T> using integer_dist_t = IntegerDist<T>;
        template<typename T> using real_dist_t = RealDist<T>;
        using bool_dist_t = BoolDist;
        using common = details::common;
        using weight = details::weight;

        constexpr basic_random_base( ) = default;

        static constexpr typename Engine::result_type (min)( ) { return impl::min( ); }
        static constexpr typename Engine::result_type (max)( ) { return impl::max( ); }
        static void discard( const unsigned long long z ) { impl::discard( engine_instance( ), z ); }
        static void reseed( ) { impl::reseed( engine_instance( ) ); }
        static void seed( const typename Engine::result_type value = Engine::default_seed ) { impl::seed( engine_instance( ), value ); }
        template<typename Sseq> static void seed( Sseq& seq ) { impl::seed( engine_instance( ), seq ); }
        static typename Engine::result_type get( ) { return impl::get( engine_instance( ) ); }
        static bool is_equal( const Engine& other ) { return impl::is_equal( engine_instance( ), other ); }
        template<typename CharT, typename Traits> static void serialize( std::basic_ostream<CharT, Traits>& ost ) { impl::serialize( engine_instance( ), ost ); }
        template<typename CharT, typename Traits> static void deserialize( std::basic_istream<CharT, Traits>& ist ) { impl::deserialize( engine_instance( ), ist ); }

        template<typename T>
        static typename std::enable_if<details::is_uniform_int<T>::value, T>::type
        get( T from = (std::numeric_limits<T>::min)( ), T to = (std::numeric_limits<T>::max)( ) ) {
            return impl::template get<T>( engine_instance( ), from, to );
        }

        template<typename T>
        static typename std::enable_if<details::is_uniform_real<T>::value, T>::type
        get( T from = (std::numeric_limits<T>::min)( ), T to = (std::numeric_limits<T>::max)( ) ) {
            return impl::template get<T>( engine_instance( ), from, to );
        }

        template<typename T>
        static typename std::enable_if<details::is_byte<T>::value, T>::type
        get( T from = (std::numeric_limits<T>::min)( ), T to = (std::numeric_limits<T>::max)( ) ) {
            return impl::template get<T>( engine_instance( ), from, to );
        }

        template<typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<std::is_same<Key, common>::value && details::is_supported_number<A>::value && details::is_supported_number<B>::value && std::is_signed<A>::value != std::is_unsigned<B>::value, C>::type
        get( A from = (std::numeric_limits<A>::min)( ), B to = (std::numeric_limits<B>::max)( ) ) {
            return impl::template get<Key, A, B, C>( engine_instance( ), from, to );
        }

        template<typename T>
        static typename std::enable_if<details::is_supported_character<T>::value, T>::type
        get(T from = (std::numeric_limits<T>::min)(), T to = (std::numeric_limits<T>::max)()) {
            return impl::template get<T>( engine_instance( ), from, to );
        }

        template<typename T>
        static typename std::enable_if<std::is_same<T, bool>::value, bool>::type
        get( const double probability = 0.5 ) {
            return impl::template get<T>( engine_instance( ), probability );
        }

        template<typename T>
        static T get( std::initializer_list<T> init_list ) {
            return impl::template get<T>( engine_instance( ), init_list );
        }

        template<typename InputIt>
        static typename std::enable_if<details::is_iterator<InputIt>::value, InputIt>::type
        get( InputIt first, InputIt last ) {
            return impl::get( engine_instance( ), first, last );
        }

        template<typename Container>
        static auto get( Container& container ) ->
            typename std::enable_if<details::is_iterator<decltype(std::begin(container))>::value, decltype(std::begin(container))>::type {
            return impl::get( engine_instance( ), container );
        }

        template<template<typename...> class Container, typename A>
        static typename std::enable_if<details::has_reserve<Container<A>>::value, Container<A>>::type
        get(A from, A to, std::size_t size) {
            return impl::template get<Container, A>( engine_instance( ), from, to, size );
        }

        template<template<typename...> class Container, typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<std::is_same<Key, common>::value && details::has_reserve<Container<A>>::value, Container<C>>::type
        get(A start, B end, std::size_t size) {
            return impl::template get<Container, Key, A, B, C>( engine_instance( ), start, end, size );
        }

        template<template<typename...> class Container, typename A>
        static typename std::enable_if<!details::has_reserve<Container<A>>::value, Container<A>>::type
        get(A start, A end, std::size_t size) {
            return impl::template get<Container, A>( engine_instance( ), start, end, size );
        }

        template<template<typename...> class Container, typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<std::is_same<Key, common>::value && !details::has_reserve<Container<C>>::value, Container<C>>::type
        get(A start, B end, std::size_t size) {
            return impl::template get<Container, Key, A, B, C>( engine_instance( ), start, end, size );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A>
        static typename std::enable_if<!details::has_insert<Container<A, N>>::value, Container<A, N>>::type
        get(A start, A end) {
            return impl::template get<Container, N, A>( engine_instance( ), start, end );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<std::is_same<Key, common>::value && !details::has_insert<Container<C, N>>::value, Container<C, N>>::type
        get(A start, B end) {
            return impl::template get<Container, N, Key, A, B, C>( engine_instance( ), start, end );
        }

        template<typename T, std::size_t N>
        static T* get( T( &array )[ N ] ) {
            return impl::get( engine_instance( ), array );
        }

        template<typename Dist, typename... Args>
        static typename Dist::result_type get( Args&&... args ) {
            return impl::template get<Dist>( engine_instance( ), std::forward<Args>( args )... );
        }

        template<typename Dist>
        static typename Dist::result_type get( Dist& dist ) {
            return impl::get( engine_instance( ), dist );
        }

        template<typename Key, class MapContainer>
        static auto get(const MapContainer& map_container) -> typename std::enable_if<
            details::is_map<MapContainer>::value &&
            details::is_iterator<decltype(std::begin(map_container))>::value &&
            !std::is_signed<typename MapContainer::mapped_type>::value &&
            std::is_same<Key, details::weight>::value,
        decltype(std::begin(map_container))>::type {
            return impl::template get<Key>( engine_instance( ), map_container );
        }

        template<typename Key, class MapContainer>
        static auto get(const MapContainer& map_container) -> typename std::enable_if<
            details::is_map<MapContainer>::value &&
            details::is_iterator<decltype(std::begin(map_container))>::value &&
            details::is_uniform_real<typename MapContainer::mapped_type>::value &&
            std::is_same<Key, details::weight>::value,
        decltype(std::begin(map_container))>::type {
            return impl::template get<Key>( engine_instance( ), map_container );
        }

        template<typename RandomIt>
        static void shuffle( RandomIt first, RandomIt last ) { impl::shuffle( engine_instance( ), first, last ); }

        template<typename Container>
        static void shuffle( Container& container ) { impl::shuffle( engine_instance( ), container ); }

        template<typename... Args>
        auto operator()( Args&&... args ) const -> decltype( get( std::forward<Args>( args )... ) ) {
            return get( std::forward<Args>( args )... );
        }

    private:
        static Engine& engine_instance( ) { return Derived::engine(); }
    };

    /**
    * \brief Random with static API and static internal storage
    */
    template<
        typename Engine,
        typename Seeder = seeder_default,
        template<typename> class IntegerDist = std::uniform_int_distribution,
        template<typename> class RealDist = std::uniform_real_distribution,
        typename BoolDist = std::bernoulli_distribution
    >
    class basic_random_static : public basic_random_base<basic_random_static<Engine, Seeder,
                                        IntegerDist, RealDist, BoolDist>,
                                        Engine, Seeder, IntegerDist, RealDist, BoolDist> {
    public:
        using seeder_type = Seeder;
        template<typename T> using integer_dist_t = IntegerDist<T>;
        template<typename T> using real_dist_t = RealDist<T>;
        using bool_dist_t = BoolDist;
        using common = details::common;
        using weight = details::weight;

        basic_random_static( ) = default;

        static Engine get_engine( ) { return engine_instance( ); }
        static Engine& engine() { return engine_instance(); }
    private:
        static Engine& engine_instance( ) {
            static Engine engine{ Seeder{ }( ) };
            return engine;
        }
    };

    /**
    * \brief Random with thread_local API and thread_local internal storage
    */
    template<
        typename Engine,
        typename Seeder = seeder_default,
        template<typename> class IntegerDist = std::uniform_int_distribution,
        template<typename> class RealDist = std::uniform_real_distribution,
        typename BoolDist = std::bernoulli_distribution
    >
    class basic_random_thread_local : public basic_random_base<basic_random_thread_local<Engine, Seeder,
                                             IntegerDist, RealDist, BoolDist>,
                                             Engine, Seeder, IntegerDist, RealDist, BoolDist> {
    public:
        using seeder_type = Seeder;
        template<typename T> using integer_dist_t = IntegerDist<T>;
        template<typename T> using real_dist_t = RealDist<T>;
        using bool_dist_t = BoolDist;
        using common = details::common;
        using weight = details::weight;

        basic_random_thread_local( ) = default;

        static Engine get_engine( ) { return engine_instance( ); }
        static Engine& engine() { return engine_instance(); }
    private:
        static Engine& engine_instance( ) {
            thread_local Engine engine{ Seeder{ }( ) };
            return engine;
        }
    };

    /**
    * \brief Random with local API and local member storage
    */
    template<
        typename Engine,
        typename Seeder = seeder_default,
        template<typename> class IntegerDist = std::uniform_int_distribution,
        template<typename> class RealDist = std::uniform_real_distribution,
        typename BoolDist = std::bernoulli_distribution
    >
    class basic_random_local {
        using impl = details::random_impl<Engine, Seeder, IntegerDist, RealDist, BoolDist>;
    public:
        using engine_type = Engine;
        using seeder_type = Seeder;
        template<typename T> using integer_dist_t = IntegerDist<T>;
        template<typename T> using real_dist_t = RealDist<T>;
        using bool_dist_t = BoolDist;
        using common = details::common;
        using weight = details::weight;

        basic_random_local( ) = default;

        static constexpr typename Engine::result_type (min)( ) { return impl::min( ); }
        static constexpr typename Engine::result_type (max)( ) { return impl::max( ); }
        void discard( const unsigned long long z ) { impl::discard( m_engine, z ); }
        void reseed( ) { impl::reseed( m_engine ); }
        void seed( const typename Engine::result_type value = Engine::default_seed ) { impl::seed( m_engine, value ); }
        template<typename Sseq> void seed( Sseq& seq ) { impl::seed( m_engine, seq ); }
        typename Engine::result_type get( ) { return impl::get( m_engine ); }
        bool is_equal( const Engine& other ) const { return impl::is_equal( m_engine, other ); }
        template<typename CharT, typename Traits> void serialize( std::basic_ostream<CharT, Traits>& ost ) const { impl::serialize( m_engine, ost ); }
        template<typename CharT, typename Traits> void deserialize( std::basic_istream<CharT, Traits>& ist ) { impl::deserialize( m_engine, ist ); }

        template<typename T>
        typename std::enable_if<details::is_uniform_int<T>::value, T>::type
        get( T from = (std::numeric_limits<T>::min)( ), T to = (std::numeric_limits<T>::max)( ) ) {
            return impl::template get<T>( m_engine, from, to );
        }

        template<typename T>
        typename std::enable_if<details::is_uniform_real<T>::value, T>::type
        get( T from = (std::numeric_limits<T>::min)( ), T to = (std::numeric_limits<T>::max)( ) ) {
            return impl::template get<T>( m_engine, from, to );
        }

        template<typename T>
        typename std::enable_if<details::is_byte<T>::value, T>::type
        get( T from = (std::numeric_limits<T>::min)( ), T to = (std::numeric_limits<T>::max)( ) ) {
            return impl::template get<T>( m_engine, from, to );
        }

        template<typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        typename std::enable_if<std::is_same<Key, common>::value && details::is_supported_number<A>::value && details::is_supported_number<B>::value && std::is_signed<A>::value != std::is_unsigned<B>::value, C>::type
        get( A from = (std::numeric_limits<A>::min)( ), B to = (std::numeric_limits<B>::max)( ) ) {
            return impl::template get<Key, A, B, C>( m_engine, from, to );
        }

        template<typename T>
        typename std::enable_if<details::is_supported_character<T>::value, T>::type
        get(T from = (std::numeric_limits<T>::min)(), T to = (std::numeric_limits<T>::max)()) {
            return impl::template get<T>( m_engine, from, to );
        }

        template<typename T>
        typename std::enable_if<std::is_same<T, bool>::value, bool>::type
        get( const double probability = 0.5 ) {
            return impl::template get<T>( m_engine, probability );
        }

        template<typename T>
        T get( std::initializer_list<T> init_list ) {
            return impl::template get<T>( m_engine, init_list );
        }

        template<typename InputIt>
        typename std::enable_if<details::is_iterator<InputIt>::value, InputIt>::type
        get( InputIt first, InputIt last ) {
            return impl::get( m_engine, first, last );
        }

        template<typename Container>
        auto get( Container& container ) ->
            typename std::enable_if<details::is_iterator<decltype(std::begin(container))>::value, decltype(std::begin(container))>::type {
            return impl::get( m_engine, container );
        }

        template<template<typename...> class Container, typename A>
        typename std::enable_if<details::has_reserve<Container<A>>::value, Container<A>>::type
        get(A from, A to, std::size_t size) {
            return impl::template get<Container, A>( m_engine, from, to, size );
        }

        template<template<typename...> class Container, typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        typename std::enable_if<std::is_same<Key, common>::value && details::has_reserve<Container<A>>::value, Container<C>>::type
        get(A start, B end, std::size_t size) {
            return impl::template get<Container, Key, A, B, C>( m_engine, start, end, size );
        }

        template<template<typename...> class Container, typename A>
        typename std::enable_if<!details::has_reserve<Container<A>>::value, Container<A>>::type
        get(A start, A end, std::size_t size) {
            return impl::template get<Container, A>( m_engine, start, end, size );
        }

        template<template<typename...> class Container, typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        typename std::enable_if<std::is_same<Key, common>::value && !details::has_reserve<Container<C>>::value, Container<C>>::type
        get(A start, B end, std::size_t size) {
            return impl::template get<Container, Key, A, B, C>( m_engine, start, end, size );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A>
        typename std::enable_if<!details::has_insert<Container<A, N>>::value, Container<A, N>>::type
        get(A start, A end) {
            return impl::template get<Container, N, A>( m_engine, start, end );
        }

        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename Key, typename A, typename B, typename C = typename std::common_type<A, B>::type>
        typename std::enable_if<std::is_same<Key, common>::value && !details::has_insert<Container<C, N>>::value, Container<C, N>>::type
        get(A start, B end) {
            return impl::template get<Container, N, Key, A, B, C>( m_engine, start, end );
        }

        template<typename T, std::size_t N>
        T* get( T( &array )[ N ] ) {
            return impl::get( m_engine, array );
        }

        template<typename Dist, typename... Args>
        typename Dist::result_type get( Args&&... args ) {
            return impl::template get<Dist>( m_engine, std::forward<Args>( args )... );
        }

        template<typename Dist>
        typename Dist::result_type get( Dist& dist ) {
            return impl::get( m_engine, dist );
        }

        template<typename Key, class MapContainer>
        auto get(const MapContainer& map_container) -> typename std::enable_if<
            details::is_map<MapContainer>::value &&
            details::is_iterator<decltype(std::begin(map_container))>::value &&
            !std::is_signed<typename MapContainer::mapped_type>::value &&
            std::is_same<Key, details::weight>::value,
        decltype(std::begin(map_container))>::type {
            return impl::template get<Key>( m_engine, map_container );
        }

        template<typename Key, class MapContainer>
        auto get(const MapContainer& map_container) -> typename std::enable_if<
            details::is_map<MapContainer>::value &&
            details::is_iterator<decltype(std::begin(map_container))>::value &&
            details::is_uniform_real<typename MapContainer::mapped_type>::value &&
            std::is_same<Key, details::weight>::value,
        decltype(std::begin(map_container))>::type {
            return impl::template get<Key>( m_engine, map_container );
        }

        template<typename RandomIt>
        void shuffle( RandomIt first, RandomIt last ) { impl::shuffle( m_engine, first, last ); }

        template<typename Container>
        void shuffle( Container& container ) { impl::shuffle( m_engine, container ); }

        template<typename... Args>
        auto operator()( Args&&... args ) -> decltype( this->get( std::forward<Args>( args )... ) ) {
            return this->get( std::forward<Args>( args )... );
        }

        Engine get_engine( ) const { return m_engine; }
        Engine& engine() { return m_engine; }
    private:
        static Engine make_seeded_engine( ) { return Engine{ Seeder{ }( ) }; }
        Engine m_engine{ make_seeded_engine( ) };
    };

    /** 
    * \brief The basic static random alias based on a std::mt19937
    */
    using random_static = basic_random_static<std::mt19937>;

    /**
    * \brief The basic thread_local random alias based on a std::mt19937
    */
    using random_thread_local = basic_random_thread_local<std::mt19937>;

    /**
    * \brief The basic local random alias based on a std::mt19937
    */
    using random_local = basic_random_local<std::mt19937>;

} // namespace effolkronium

#endif // #ifndef EFFOLKRONIUM_RANDOM_HPP
