# Random for modern C++ with convenient API
[![Build Status](https://travis-ci.org/effolkronium/random.svg?branch=develop)](https://travis-ci.org/effolkronium/random)
[![Build status](https://ci.appveyor.com/api/projects/status/vq1kodqqxwx16rfv/branch/develop?svg=true)](https://ci.appveyor.com/project/effolkronium/random/branch/develop)
[![Coverage Status](https://coveralls.io/repos/github/effolkronium/random/badge.svg?branch=develop)](https://coveralls.io/github/effolkronium/random?branch=develop)
<a href="https://scan.coverity.com/projects/effolkronium-random">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/12707/badge.svg"/>
</a>
- [Design goals](#design-goals)
- [Supported compilers](#supported-compilers)
- [Integration](#integration)
- [Five-minute tutorial](#five-minute-tutorial)
  - [Number range](#number-range)
  - [Common type number range](#common-type-number-range)
  - [Bool](#bool)
  - [Random value from std::initilizer_list](#random-value-from-stdinitilizer_list)
  - [Random iterator](#random-iterator)
  - [Shuffle](#shuffle)
  - [Custom distribution](#custom-distribution)
  - [Custom Seeder](#custom-seeder)
  - [Seeding](#seeding)
  - [min-value](#min-value)
  - [max-value](#max-value)
  - ['get' without arguments](#get-without-arguments)
  - [Get engine](#get-engine)
  - [Discard](#discard)
  - [isEqual](#isequal)
  - [Serialize](#serialize)
  - [Deserialize](#deserialize)
  - [Thread local random](#thread-local-random)
  - [Local random](#local-random)
## Design goals
There are few ways to get working with random in C++:
- **C style**
```cpp
  srand( time(NULL) ); // seed with time since epoch
  auto random_number = rand() % (9 - 1)) + 1; // get a pseudo-random integer between 1 and 9
```
* Problems
  * should specify seed
  * should write your own distribution algorihtm
  * [There are no guarantees as to the quality of the random sequence produced.](http://en.cppreference.com/w/cpp/numeric/random/rand#Notes)
- **C++11 style**
```cpp
  std::random_device random_device; // create object for seeding
  std::mt19937 engine{random_device()}; // create engine and seed it
  std::uniform_int_distribution<> dist(1,9); // create distribution for integers with [1, 9] range
  auto random_number = dist(engine); // finally get a pseudo-randomrandom integer number
```
* Problems
  * should specify seed
  * should choose, create and use a chain of various objects like engines and distributions
  * mt19937 use 5000 bytes of memory for each creation (which is bad for performance if we create it too frequently)
  * uncomfortable and not intuitively clear usage
- **effolkronium random style**
```cpp
  // auto seeded
  auto random_number = Random::get(1, 9); // invoke 'get' method to generate a pseudo-random integer between 1 and 9
  // yep, that's all.
```
* Advantages
  * **Intuitive syntax**. You can do almost everything with random by simple 'get' method, like getting simple numbers, bools, random object from given set or using custom distribution.
  * **Trivial integration**. All code consists of a single header file [`random.hpp`](https://github.com/effolkronium/random/blob/develop/include/effolkronium/random.hpp). Tahat's it. No library, no subproject, no dependencies, no complex build system. The class is written in vanilla C++11. All in all, everything should require no adjustment of your compiler flags or project settings.
  * **Usability**. There are 3 versions of random: 
    * *random_static* which has static methods and static internal state. It's not thread safe but more efficient
    * *random_thread_local* which has static methods and [thread_local](http://en.cppreference.com/w/cpp/keyword/thread_local) internal state. It's thread safe but less efficient
    * *random_local* which has non static methods and local internal state. It can be created on the stack at local scope
## Supported compilers
* GCC 4.9 - 7.0 (and possibly later)
* Clang 3.7 - 4.0 (and possibly later)
* Microsoft Visual C++ 2015
* Microsoft Visual C++ 2017
## Integration
#### CMake
TODO
#### Manually
The single required source, file [`random.hpp`](https://github.com/effolkronium/random/blob/develop/include/effolkronium/random.hpp) is in the [`include/effolkronium`](https://github.com/effolkronium/random/tree/develop/include/effolkronium) directory.
All you need to do is add
```cpp
#include "effolkronium/random.hpp"

// get base random alias which is auto seeded and has static API and internal state
using Random = effolkronium::random_static;
```
to the files you want to use effolkronium random class. That's it. Do not forget to set the necessary switches to enable C++11 (e.g., `-std=c++11` for GCC and Clang).
## Five-minute tutorial
### Number range
Returns a  pseudo-random number in a [first; second] range.
```cpp
auto val = Random::get(-1, 1) // decltype(val) is int
```
```cpp
// specify explicit type
auto val = Random::get<uint8_t>(-1, 1) // decltype(val) is uint8_t
```
```cpp
// you able to use range from greater to lower
auto val = Random::get(1.l, -1.l) // decltype(val) is long double
```
```cpp
auto val = Random::get(1.f, -1) // Error: implicit conversions are not allowed here.
```
### Common type number range
Choose common type of two range arguments by std::common_type.
```cpp
auto val = Random::get<Random::common>(1, 0.f) // decltype(val) is float
```
```cpp
auto val = Random::get<Random::common>(0ul, 1ull) // decltype(val) is unsigned long long
```
```cpp
auto val = Random::get<Random::common>(1.2l, 1.5f) // decltype(val) is long double
```
```cpp
auto val = Random::get<Random::common>(1u, -1) // Error: prevent conversion from signed to unsigned.
```
### Bool
Generate true with [0; 1] probability
```cpp
auto val = Random::get<bool>(0.7) // true with 70% probability
```
```cpp
auto val = Random::get<bool>() // true with 50% probability by default
```
```cpp
auto val = Random::get<bool>(-1) // Error: assert occurred! Out of [0; 1] range
```
### Random value from std::initilizer_list
Return random value from values in std::initilizer_list
```cpp
auto val = Random::get({1, 2, 3}) // val = 1 or 2 or 3
```
### Random iterator
Returns random iterator from iterator range or container.
Iterator must be at least [Input iterator](http://en.cppreference.com/w/cpp/concept/InputIterator).
If 'first' iterator is equal 'last' iterator, returns 'last' iterator.
If container is empty, returns std::end(container) iterator.
```cpp
std::array<int, 3> array{ {1, 2, 3} };
```
* Iterator range
```cpp
auto randomIt = Random::get( array.begin(), array.end() );
```
* Container
```cpp
auto randomIt = Random::get( array );
```
### Shuffle
Reorders the elements in a given range or in all container [ref](http://en.cppreference.com/w/cpp/algorithm/random_shuffle)
```cpp
std::array<int, 3> array{ {1, 2, 3} };
```
* Iterator range
```cpp
Random::shuffle( array.begin( ), array.end( ) )
```
* Container
```cpp
Random::shuffle( array )
```
### Custom distribution
Return result from operator() of a distribution with internal random engine of the Random class
* Template argument
```cpp
// 1.f and 2.f will be forwarded to std::gamma_distribution constructor
auto result = Random::get<std::gamma_distribution<>>( 1.f, 2.f );
```
* Argument by reference
```cpp
std::gamma_distribution<> gamma{ 1.f, 2.f };
auto result = Random DOT get( gamma ); // return result of gamma.operator()( engine_ )
```
### Custom Seeder
Specify seed by which random engine will be seeded at construction time:
* Number
```cpp
struct MySeeder {
    unsigned operator() () {
        return 42u;
    }
};
    
// Seeded by 42
using Random = effolkronium::basic_random_static<std::mt19937, MySeeder>;
```
* Seed sequence
```cpp
struct MySeeder {
    // std::seed_seq isn't copyable
    std::seed_seq& operator() () {
        return seed_seq_;
    }
    std::seed_seq seed_seq_{ { 1, 2, 3, 4, 5 } };
};
    
// Seeded by seed_seq_ from MySeeder
using Random = effolkronium::basic_random_static<std::mt19937, MySeeder>;
```
### Seeding
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/seed)

You able to reseed internal random engine.
```cpp
Random::seed( 10 ); // 10 is new seed number

std::seed_seq sseq{ 1, 2, 3 };
Random::seed( sseq ); // use seed sequence here
```
### Min value
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/min)

Returns the minimum value potentially generated by the internal random-number engine
```cpp
auto minVal = Random::min( );
```
### Max value
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/max)

Returns the maximum value potentially generated by the internal random-number engine
```cpp
auto maxVal = Random::max( );
```
### get without arguments
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/operator())

Returns the random number in [ Random::min( ), Random::max ] range
```cpp
auto val = Random::get( );
// val is rundom number in [ Random::min( ), Random::max ] range
```
### Get engine
Returns copy of internal engine.
```cpp
auto engine = Random::get_engine( );
```
### Discard
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/discard)

Advances the internal engine's state by a specified amount.
Equivalent to calling Random::get() N times and discarding the result.
```cpp
Random::discard( 500 );
```
### IsEqual
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/operator_cmp)

Compares internal pseudo-random number engine with other pseudo-random number engine.
```cpp
Random::Engine otherEngine;
bool isSame = Random::is_equal( otherEngine );
```
### Serialize
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/operator_ltltgtgt)

Serializes the internal state of the internal pseudo-random number engine as a sequence of decimal numbers separated by one or more spaces, and inserts it to the output stream. The fill character and the formatting flags of the stream are ignored and unaffected.
```cpp
std::stringstream strStream;
Random::serialize( strStream ); // the strStream now contain internal state of the Random internal engine
```
### Deserialize
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/operator_ltltgtgt)

Restores the internal state of the internal pseudo-random number engine from the serialized representation, which was created by an earlier call to 'serialize' using a stream with the same imbued locale and the same CharT and Traits. If the input cannot be deserialized, internal engine is left unchanged and failbit is raised on input stream.
```cpp
std::stringstream strStream;
Random::serialize( strStream );

// ...

Random::deserialize( strStream ); // Restore internal state of internal Random engine
```
### Thread local random
It uses static methods API and data with thread_local storage which is fully **thread safe** (but less perfomance)
```cpp
using Random = effolkronium::random_thread_local

// use in the same way as random_static. Thread safe
std::thread first{ [ ] { Random::get( ); } };
std::thread second{ [ ] { Random::get( ); } };
```
### Local random
It uses non static methods API and data with auto storage which can be created on the stack at local scope
```cpp
#include "effolkronium/random.hpp"

using Random_t = effolkronium::random_local

int main( ) {
  Random_t localRandom{ }; // Construct on the stack
  
  // access throughout dot
  auto val = localRandom.get(-10, 10);
  
} // Destroy localRandom and free stack memory
```
