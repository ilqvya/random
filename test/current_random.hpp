#include "effolkronium/random.hpp"

#ifdef RANDOM_LOCAL
auto Random = effolkronium::random_local{ };
#endif

#ifdef RANDOM_STATIC
using Random = effolkronium::random_static;
#endif

#ifdef RANDOM_THREAD_LOCAL
using Random = effolkronium::random_thread_local;
#endif

#ifndef RANDOM_LOCAL
using Random_t = Random;
#define DOT ::
#else
using Random_t = effolkronium::random_local;
#define DOT .
#endif