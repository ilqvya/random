#ifndef EFFOLKRONIUM_RANDOM_HPP
#define EFFOLKRONIUM_RANDOM_HPP

#include <random>
#include <type_traits>
#include <cassert>
#include <initializer_list>
#include <utility> // std::declval
#include <algorithm> // std::shuffle
#include <iterator> // std::begin, std::end

namespace effolkronium {
    namespace details {
        /// Key type for getting common type numbers or objects
        struct common{ }; 

        /// True if type T is applicable by the std::uniform_int_distribution
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

    } // namespace details

    /**
    * \brief The default seeder for random classes
    */
    struct seeder_default final {
        /// return seed number
        std::random_device::result_type operator() ( ) const noexcept {
            return std::random_device{ }( );
        }
    };

    /**
    * \brief Base template class for random 
    *        with static API and static internal member storage
    * \param Engine A random engine with interface like in the std::mt19937
    * \param Seeder A seeder for random class, 
    *        from which will be seeded internal random_engine
    */
    template<typename Engine, typename Seeder = seeder_default>
    class basic_random_static final {
    public:
        /// Type of used random number engine
        using engine_type = Engine;

        /// Key type for getting common type numbers or objects
        using common = details::common;

        /**
        * \brief Generate a random integer number in a [from; to] range
        *        by std::uniform_int_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random integer number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
		* \note Prevent implicit type conversion
        */
        template<typename A>
        static typename std::enable_if<details::is_uniform_int<A>::value
            , A>::type get( A from, A to ) noexcept {
            if( from < to ) // Allow range from higher to lower
                return std::uniform_int_distribution<A>{ from, to }( engine );
            return std::uniform_int_distribution<A>{ to, from }( engine );
        }

        /**
        * \brief Generate a random real number in a [from; to] range
        *        by std::uniform_real_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random real number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename A>
        static typename std::enable_if<details::is_uniform_real<A>::value
            , A>::type get( A from, A to ) noexcept {
            if( from < to ) // Allow range from higher to lower
                return std::uniform_real_distribution<A>{ from, to }( engine );
            return std::uniform_real_distribution<A>{ to, from }( engine );
        }

        /**
        * \brief Generate a random byte number in a [from; to] range
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random byte number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename A>
        static typename std::enable_if<details::is_byte<A>::value
            , A>::type get( A from, A to ) noexcept {
            // Choose between short and unsigned short for byte conversion
            using short_t = typename std::conditional<std::is_signed<A>::value,
                short, unsigned short>::type;

            return static_cast<A>( get<short_t>( from, to ) );
        }

        /**
        * \brief Generate a random common_type number in a [from; to] range
        * \param Key The Key type for this version of 'get' method
        *        Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random common_type number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Allow implicit type conversion
        * \note Prevent implicit type conversion from singed to unsigned types
        *       Why? std::common_type<Unsigned, Signed> chooses unsigned value,
        *                 then Signed value will be converted to Unsigned value
        *                       which gives us a wrong range for random values.
        *                           https://stackoverflow.com/a/5416498/5734836
        */
        template<
            typename Key,
            typename A,
            typename B, 
            typename C = typename std::common_type<A, B>::type
        >
        static typename std::enable_if<
               std::is_same<Key, common>::value
            && details::is_supported_number<A>::value
            && details::is_supported_number<B>::value
            // Prevent implicit type conversion from singed to unsigned types
            && std::is_signed<A>::value != std::is_unsigned<B>::value
            , C>::type get( A from, B to ) noexcept {
            return get( static_cast<C>( from ), static_cast<C>( to ) );
        }

        /**
        * \brief Generate a bool value with specific probability
        *                         by std::bernoulli_distribution
        * \param probability The probability of generating true in [0; 1] range
        *        0 means always false, 1 means always true
        * \return 'true' with 'probability' probability ('false' otherwise)
        */
        template<typename T>
        static typename std::enable_if<std::is_same<T, bool>::value
            , bool>::type get( const double probability = 0.5 ) noexcept {
            assert( 0 <= probability && 1 >= probability ); // out of [0; 1] range
            return std::bernoulli_distribution{ probability }( engine );
        }

        /**
        * \brief Return random value from initilizer_list
        * \param init_list initilizer_list with values
        * \return Random value from initilizer_list
        * \note Should be 1 or more elements in initilizer_list
        * \note Warning! Elements in initilizer_list can't be moved:
        *               https://stackoverflow.com/a/8193157/5734836
        */
        template<typename T>
        static T get( std::initializer_list<T> init_list ) 
                noexcept( noexcept( T{ std::declval<const T>( ) } ) ) {
            assert( 0 != init_list.size( ) );

            return *( init_list.begin( ) +
                      get<typename std::initializer_list<T>::size_type>(
                          0, init_list.size( ) - 1 ) );
        }

        /**
        * \brief Reorders the elements in the given range [first, last)
        *        such that each possible permutation of those elements
        *        has equal probability of appearance.
        * \param first, last - the range of elements to shuffle randomly       
        */
        template<class RandomIt>
        static void shuffle( RandomIt first, RandomIt last ) noexcept {
            std::shuffle( first, last, engine );
        }

        /**
        * \brief Reorders the elements in the given container
        *        such that each possible permutation of those elements
        *        has equal probability of appearance.
        * \param container - the container with elements to shuffle randomly
        */
        template<class Container>
        static void shuffle( Container& container ) noexcept {
            shuffle( std::begin( container ), std::end( container ) );
        }
    private:
        /// The random number engine
        static Engine engine;
    };

    /// Seed random number engine by Seeder
    template<typename Engine, typename Seeder>
    Engine basic_random_static<Engine, Seeder>::engine( [ & ] {
        Seeder seeder{ };
        return static_cast<typename Engine::result_type>( 
            static_cast<Seeder>(seeder)( )
            );
    }( ) );

    /** 
    * \brief The basic static random alias based on a std::default_random_engine
    * \note It uses static methods API and data with static storage
    * \note Not thread safe but more prefomance
    */
    using random_static = basic_random_static<std::default_random_engine>;

} // namespace effolkronium

#endif // #ifndef EFFOLKRONIUM_RANDOM_HPP