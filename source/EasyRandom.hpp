#include <random>

namespace EasyRandom {
    class Random {
    public:
        static int get( ) {
            std::uniform_int_distribution<> dist{ -10, 10 };
            return dist( mtEngine( ) );
        }
    private:
        static std::mt19937& mtEngine( ) {
            static std::mt19937 mt{ std::random_device{ }( ) };
            return mt;
        }
    };
}
