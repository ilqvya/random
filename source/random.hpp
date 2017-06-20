#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>
#include <type_traits>

namespace effolkronium {
	/**
	* \brief Base template class for random
	* \param Engine A random engine with interface like in the std::mt19937
	*/
    template<typename Engine>
    class basic_random {
    public:
		/// Type of used random number engine
        using engine_type = Engine;

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

        /**
        * \brief Generate a random integer number in a [from, to] range
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random integer number in a [from, to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
		* \note Prevent implicit type conversion
        */
        template<typename A>
        static typename std::enable_if<
               is_uniform_int<A>::value
            && is_uniform_int<A>::value
            , A>::type get( A from, A to ) noexcept {
            if( from < to ) // Allow range from higher to lower
                return std::uniform_int_distribution<A>{ from, to }( engine );
            return std::uniform_int_distribution<A>{ to, from }( engine );
        }

        /**
        * \brief Generate a random real number in a [from, to] range
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random real number in a [from, to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
		* \note Prevent implicit type conversion
        */
        template<typename A>
        static typename std::enable_if<
               is_uniform_real<A>::value
            && is_uniform_real<A>::value
            , A>::type get( A from, A to ) noexcept {
            if( from < to ) // Allow range from higher to lower
                return std::uniform_real_distribution<A>{ from, to }( engine );
            return std::uniform_real_distribution<A>{ to, from }( engine );
        }
    private:
        /// The random number engine
        static Engine engine;
    };

    template<typename Engine>
    Engine basic_random<Engine>::engine( std::random_device{ }( ) );

    /// The basic random alias based on a std::mt19937 random number engine
    using random = basic_random<std::mt19937>;

} // namespace effolkronium

#endif // #ifndef RANDOM_HPP