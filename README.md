# Random for modern C++ with convenient API
[![CI](https://github.com/ilqvya/random/actions/workflows/ci.yml/badge.svg)](https://github.com/ilqvya/random/actions/workflows/ci.yml)
[![Coverage Status](https://coveralls.io/repos/github/ilqvya/random/badge.svg?branch=master&unused=0)](https://coveralls.io/github/ilqvya/random?branch=master&unused=0)
<a href="https://scan.coverity.com/projects/ilqvya-random"><img alt="Coverity Scan Build Status" src="https://scan.coverity.com/projects/31956/badge.svg"/></a>
- [Design goals](#design-goals)
- [Supported compilers](#supported-compilers)
- [Integration](#integration)
- [Five-minute tutorial](#five-minute-tutorial)
  - [Number range](#number-range)
  - [Common type number range](#common-type-number-range)
  - [Character range](#character-range)
  - [Bool](#bool)
  - [Random value from std::initializer_list](#random-value-from-stdinitializer_list)
  - [Random iterator](#random-iterator)
  - [Random element from array](#random-element-from-array)
  - [Container of random values](#container-of-random-values)
  - [Weighted random values](#weighted-random-values)
  - [Shuffle](#shuffle)
  - [Custom distribution](#custom-distribution)
  - [Custom Seeder](#custom-seeder)
  - [Thread local random](#thread-local-random)
  - [Local random](#local-random)
  - [engine](#engine)
  - [Get engine](#get-engine)
  - [Seeding](#seeding)
  - [min-value](#min-value)
  - [max-value](#max-value)
  - ['get' without arguments](#get-without-arguments)
  - [Discard](#discard)
  - [Is equal](#is-equal)
  - [Serialize](#serialize)
  - [Deserialize](#deserialize)
## Design goals
There are a few ways to work with random in C++:
- **C style**
```cpp
  srand( time(NULL) ); // seed with time since epoch
  auto random_number = (rand() % 9) + 1; // get a pseudo-random integer between 1 and 9
```
* Problems:
  * Must specify a seed manually
  * Must write your own distribution algorithm
  * [There are no guarantees as to the quality of the random sequence produced.](http://en.cppreference.com/w/cpp/numeric/random/rand#Notes)
- **C++11 style**
```cpp
  std::random_device random_device; // create object for seeding
  std::mt19937 engine{random_device()}; // create engine and seed it
  std::uniform_int_distribution<> dist(1,9); // create distribution for integers with [1; 9] range
  auto random_number = dist(engine); // finally get a pseudo-random integer number
```
* Problems:
  * Must specify a seed manually
  * Must choose, create, and manage a chain of various objects like engines and distributions
  * [mt19937](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine) uses ~5000 bytes of internal state for each instance (which hurts performance if created repeatedly)
  * Cumbersome and not intuitive to use
- **ilqvya random style**
```cpp
  // auto seeded
  auto random_number = Random::get(1, 9); // invoke 'get' method to generate a pseudo-random integer in [1; 9] range
  // yep, that's all.
```
* Advantages:
  * **Intuitive syntax**. You can do almost everything with random via a simple 'get' method: generating numbers, bools, random elements from a given container or set, or using custom distributions.
  * **Trivial integration**. All code consists of a single header file [`random.hpp`](https://github.com/ilqvya/random/blob/master/include/ilqvya/random.hpp). That's it. No compiled library, no subproject, no external dependencies, no complex build system. Written in vanilla C++11 and compatible with modern C++.
  * **Usability**. Available generator types: 
    * *random* (or *random_thread_local*): static methods and [thread_local](http://en.cppreference.com/w/cpp/keyword/thread_local) internal state. Fully thread-safe default!
    * *random_static*: static methods and static internal state. Fast, but not thread-safe across concurrent threads.
    * *random_local*: non-static member methods and local member state. Can be created on the stack at local scope.
  * **Next-Gen Performance**. Uses `std::philox4x64` as the default engine in C++26 (when available), delivering fast counter-based random number generation with minimal 32-byte state and full 256-bit entropy seeding, while seamlessly falling back to `std::mt19937` in C++11–C++23.
## Supported compilers
* GCC 4.9+ (and later)
* Clang 3.7+ (and later)
* Microsoft Visual C++ 2015+ (and later)
## Integration
#### CMake
* As subproject
```cmake
add_subdirectory(random) # path to the 'random' library root
... # create target
target_link_libraries(${TARGET} ilqvya::random)
```
* As external project

First, build and install the project:
```cmd
cd "path_to_root_of_the_library"
mkdir build
cd build
cmake ..
cmake --build . --target install --config Release
ctest -C Release
```
Then, find the package via CMake:
```cmake
find_package(ilqvya_random REQUIRED)
... # create target
target_link_libraries(${TARGET} ilqvya::random)
```
#### Manually
The single required source file, [`random.hpp`](https://github.com/ilqvya/random/blob/master/include/ilqvya/random.hpp), is located in the [`include/ilqvya`](https://github.com/ilqvya/random/tree/master/include/ilqvya) directory.
#### Then
All you need to do is add
```cpp
#include "ilqvya/random.hpp"

// default thread-local random alias (thread-safe, auto-seeded)
using Random = ilqvya::random;
```
to files where you want to use the random class. That's it! Ensure C++11 or newer is enabled in your compiler settings (e.g., `-std=c++11` for GCC and Clang).
## Five-minute tutorial
### Number range
Returns a pseudo-random number in a [first; second] range.
```cpp
auto val = Random::get(-1, 1); // decltype(val) is int
```
```cpp
// specify explicit type
auto val = Random::get<uint8_t>(-1, 1); // decltype(val) is uint8_t
```
```cpp
// You can also specify range from greater to lower
auto val = Random::get(1.l, -1.l); // decltype(val) is long double
```
```cpp
// std::byte support (C++17+)
auto b = Random::get<std::byte>(); // random byte in [0, 255]
auto bounded_b = Random::get(std::byte{0}, std::byte{128});
```
```cpp
auto val = Random::get(1.f, -1); // Error: implicit conversions are not allowed here.
```
### Common type number range
Choose common type of two range arguments by std::common_type.
```cpp
auto val = Random::get<Random::common>(1, 0.f); // decltype(val) is float
```
```cpp
auto val = Random::get<Random::common>(0ul, 1ull); // decltype(val) is unsigned long long
```
```cpp
auto val = Random::get<Random::common>(1.2l, 1.5f); // decltype(val) is long double
```
```cpp
auto val = Random::get<Random::common>(1u, -1); // Error: prevents conversion between signed and unsigned without common type promotion.
```
### Character range
Returns a pseudo-random character in a [first; second] range.
```cpp
auto val = Random::get('a', 'z');
```
```cpp
auto val = Random::get(L'㋐', L'㋾');
```
```cpp
auto val = Random::get<wchar_t>();
```
```cpp
auto val = Random::get(u'a', u'z'); // char16_t
auto val = Random::get(U'a', U'z'); // char32_t
auto val = Random::get<char32_t>(); // default limits
```
### Bool
Generates true with a probability in [0.0, 1.0].
```cpp
auto val = Random::get<bool>(0.7); // true with 70% probability
```
```cpp
auto val = Random::get<bool>(); // true with 50% probability by default
```
```cpp
auto val = Random::get<bool>(-1); // Throws std::invalid_argument (out of [0.0, 1.0] range)
```
### Random value from std::initializer_list
Returns a random value from values in a std::initializer_list.
Throws `std::invalid_argument` if the list is empty.
```cpp
auto val = Random::get({1, 2, 3}); // val = 1 or 2 or 3
```
### Random iterator
Returns a random iterator from an iterator range or container.
Iterator must satisfy at least [ForwardIterator](http://en.cppreference.com/w/cpp/named_req/ForwardIterator).
If std::distance(first, last) == 0, returns the 'last' iterator.
If the container is empty, returns the [std::end](http://en.cppreference.com/w/cpp/iterator/end)(container) iterator.
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
### Random element from array
Returns a pointer to a random element in a built-in array.
```cpp
int array [] = {1, 2, 3};
auto randomPtr = Random::get( array );
```
### Container of random values
Returns a container filled with random numbers.
Any container with "begin", "end", and "insert" methods is supported:
```cpp
auto vec = Random::get<std::vector>(1, 9, 5); // decltype(vec) is std::vector<int> with size = 5
// Note: "reserve" method is invoked automatically if available for performance

auto deq = Random::get<std::deque>(1, 9, 5); // decltype(deq) is std::deque<int> with size = 5

auto lst = Random::get<std::list>(1.0, 9.9, 10); // decltype(lst) is std::list<double> with size = 10

auto mset = Random::get<std::multiset>(1.0, 9.9, 10); // decltype(mset) is std::multiset<double> with size = 10

auto arr = Random::get<std::array, 5>('0', '9'); // decltype(arr) is std::array<char, 5>
// Warning: Returning arrays with large size could be inefficient

auto vec = Random::get<std::vector>(1l, 9ll, 5); // decltype(vec) is std::vector<long long> with size = 5

template<typename T>
class MyContainer
{
    iterator begin() {...}
    iterator end() {...}
    void insert(iterator after, T value) {...}
};

auto vec = Random::get<MyContainer>(1, 9, 5); // decltype(vec) is std::MyContainer<int> with size = 5

```
### Weighted random values
* Inline weighted selection (returns chosen value directly):
```cpp
    // Variadic syntax
    auto val = Random::get<int>({1, 3}, {4, 4}, {10, 5}); // returns chosen int based on weights 3, 4, 5
    auto f   = Random::get<float>({1.1f, 1}, {1.2f, 2});

    // Initializer list syntax
    auto str = Random::get<std::string>({ {"Apple", 10}, {"Banana", 20}, {"Orange", 30} });
```
* Containers of pairs or map-like containers (returns iterator):
```cpp
    std::unordered_map<std::string, unsigned long> nonzero_ulong_umap = {{"Orange", 1ul}, {"Apple", 2ul}, {"Banana", 3ul}};
    std::unordered_map<std::string, unsigned> nonzero_uint_umap = {{"Orange", 1u}, {"Apple", 2u}, {"Banana", 3u}};
    
    std::map<std::string, float> nonzero_float_map = {{"Orange", 1.0f}, {"Apple", 2.0f}, {"Banana", 3.0f}};
    std::map<std::string, double> nonzero_double_map = {{"Orange", 1.0}, {"Apple", 2.0}, {"Banana", 3.0}};

    std::vector<std::pair<std::string, int>> nonzero_vec = {{"Orange", 1}, {"Apple", 2}, {"Banana", 3}};

    Random::get<Random_t::weight>(nonzero_ulong_umap);
    Random::get<Random_t::weight>(nonzero_uint_umap);
    Random::get<Random_t::weight>(nonzero_float_map);
    Random::get<Random_t::weight>(nonzero_double_map);
    Random::get<Random_t::weight>(nonzero_vec);
```
### Enum random values (C++26 Reflection)
Returns a random enumerator from any `enum` or `enum class` (requires C++26 with reflection support, e.g. `-std=c++26 -freflection`):
```cpp
enum class Color { Red, Green, Blue };
auto color = Random::get<Color>(); // Returns randomly chosen Color

enum Status { Disconnected = 0, Connecting = 10, Connected = 100 };
auto status = Random::get<Status>(); // Returns randomly chosen Status
```
### Shuffle
Reorders the elements in a given range or container [ref](http://en.cppreference.com/w/cpp/algorithm/random_shuffle):
```cpp
std::array<int, 3> array{ {1, 2, 3} };
```
* Iterator range
```cpp
Random::shuffle( array.begin( ), array.end( ) );
```
* Container
```cpp
Random::shuffle( array );
```
### Custom distribution
Returns the result from operator() of a distribution using the internal engine:
* Template argument
```cpp
// 1.f and 2.f will be forwarded to std::gamma_distribution constructor
auto result = Random::get<std::gamma_distribution<>>( 1.f, 2.f );
```
* Argument by reference or temporary
```cpp
std::gamma_distribution<> gamma{ 1.f, 2.f };
auto result = Random::get( gamma ); // returns result of gamma.operator()( engine )

// Or pass temporary distribution directly:
auto inline_result = Random::get( std::uniform_int_distribution<int>{ 1, 6 } );
```
### Custom Seeder
Specify a seed with which the random engine will be initialized at construction time:
* Number
```cpp
struct MySeeder {
    unsigned operator() () {
        return 42u;
    }
};
    
// Seeded by 42
using Random = ilqvya::basic_random_static<std::mt19937, MySeeder>;
```
* Seed sequence

Because `std::seed_seq` is non-copyable, the 'random' library destroys the seeder instance after engine seeding, so returning the seed sequence by reference is safe:
```cpp
struct MySeeder {
    // std::seed_seq isn't copyable
    std::seed_seq& operator() () {
        return seed_seq_;
    }
    std::seed_seq seed_seq_{ { 1, 2, 3, 4, 5 } };
};
    
// Seeded by seed_seq_ from MySeeder
using Random = ilqvya::basic_random_static<std::mt19937, MySeeder>;
```
* Reseed

Seeds the internal random engine using a newly created Seeder instance:
```cpp
Random::reseed( );
```
### Thread local random
Uses static method API with [thread_local](http://en.cppreference.com/w/cpp/keyword/thread_local) storage, which is fully **thread-safe** (and available as default alias `ilqvya::random`):
```cpp
using Random = ilqvya::random; // or ilqvya::random_thread_local

// use in the same way as random_static. Thread safe
std::thread first{ [ ] { Random::get( ); } };
std::thread second{ [ ] { Random::get( ); } };
```
### Local random
Uses member method API with local member storage, created on the stack at local scope:
```cpp
#include "ilqvya/random.hpp"

using Random_t = ilqvya::random_local;

int main( ) {
  Random_t localRandom{ }; // Construct on the stack
  
  // access via dot operator
  auto val = localRandom.get(-10, 10);
  
} // Destroy localRandom and free stack memory
```
### Seeding
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/seed)

Sets a new seed for the internal random engine:
```cpp
Random::seed( 10 ); // 10 is new seed number

std::seed_seq sseq{ 1, 2, 3 };
Random::seed( sseq ); // use seed sequence here
```
### Min value
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/min)

Returns the minimum value potentially generated by the internal random-number engine:
```cpp
auto minVal = Random::min( );
```
### Max value
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/max)

Returns the maximum value potentially generated by the internal random-number engine:
```cpp
auto maxVal = Random::max( );
```
### 'get' without arguments
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/operator())

Returns the random number in [ Random::min( ), Random::max( ) ] range:
```cpp
auto val = Random::get( );
// val is random number in [ Random::min( ), Random::max( ) ] range
```
### engine
Returns a reference to the internal engine:
```cpp
auto& engine = Random::engine( );
```
```cpp
std::sample(itBeg, itEnd, std::back_inserter(out), 5, Random::engine( ));
```
### Get engine
Returns a copy of the internal engine:
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
### Is equal
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/operator_cmp)

Compares the internal pseudo-random number engine with another engine:
```cpp
Random::engine_type otherEngine;
bool isSame = Random::is_equal( otherEngine );
```
### Serialize
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/operator_ltltgtgt)

Serializes the internal state of the pseudo-random number engine as a sequence of decimal numbers separated by spaces, and inserts it into the output stream. The fill character and formatting flags of the stream are ignored and unaffected.
```cpp
std::stringstream strStream;
Random::serialize( strStream ); // strStream now contains the internal state of the Random engine
```
### Deserialize
[ref](http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/operator_ltltgtgt)

Restores the internal state of the pseudo-random number engine from a serialized representation created by an earlier call to 'serialize' using a stream with the same imbued locale, CharT, and Traits. If the input cannot be deserialized, the internal engine is left unchanged and failbit is set on the input stream.
```cpp
std::stringstream strStream;
Random::serialize( strStream );

// ...

Random::deserialize( strStream ); // Restore internal state of the Random engine
```
