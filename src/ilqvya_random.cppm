module;

#include "ilqvya/random.hpp"

export module ilqvya.random;

export namespace ilqvya {
    using ilqvya::random;
    using ilqvya::Random;
    using ilqvya::random_thread_local;
    using ilqvya::random_static;
    using ilqvya::random_local;

    using ilqvya::default_engine;
    using ilqvya::seeder_default;

    using ilqvya::basic_random_base;
    using ilqvya::basic_random_static;
    using ilqvya::basic_random_local;
    using ilqvya::basic_random_thread_local;

    namespace details {
        using details::weight;
        using details::common;
        using details::weighted_result;
    }
}

export namespace effolkronium = ilqvya;
