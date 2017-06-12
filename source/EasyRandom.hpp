#include <random>
#include <type_traits>

namespace EasyRandom {
    template<typename Engine>
    class basic_random {
    public:
        using engine_type = Engine;

        /**
        * \brief True if type T is applicable by a std::uniform_int_distribution
        */
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

        /**
        * \brief True if type T is applicable by a std::uniform_real_distribution
        */
        template<typename T>
        struct is_uniform_real {
            static constexpr bool value =
                   std::is_same<T,       float>::value
                || std::is_same<T,      double>::value
                || std::is_same<T, long double>::value;
        };

        /**
        * \brief Generate a random integer number in a [from, to] range
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random integer number in a [from, to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        */
        template<typename A, typename B, typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<
               is_uniform_int<A>::value
            && is_uniform_int<B>::value
            && std::is_signed<A>::value != std::is_unsigned<B>::value // Prevent conversion from signed to unsigned
            , C>::type get( A from, B to )noexcept {
            if( from < to ) // Allow range from higher to lower
                return std::uniform_int_distribution<C>{ from, to }( engine );
            return std::uniform_int_distribution<C>{ to, from }( engine );
        }

        /**
        * \brief Generate a random real number in a [from, to] range
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random real number in a [from, to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        */
        template<typename A, typename B, typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<
               is_uniform_real<A>::value
            && is_uniform_real<B>::value
            , C>::type get( A from, B to )noexcept {
            if( from < to ) // Allow range from higher to lower
                return std::uniform_real_distribution<C>{ from, to }( engine );
            return std::uniform_real_distribution<C>{ to, from }( engine );
        }
    private:
        /// The random number engine
        static Engine engine;
    };

    template<typename Engine>
    Engine basic_random<Engine>::engine( std::random_device{ }( ) );

    /// The basic random alias based on a std::mt19937 random number engine
    using Random = basic_random<std::mt19937>;
}
