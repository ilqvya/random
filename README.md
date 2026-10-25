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
  - [Bool & Coin Flip](#bool--coin-flip)
  - [Random value from std::initializer_list](#random-value-from-stdinitializer_list)
  - [Random iterator](#random-iterator)
  - [Random element from array](#random-element-from-array)
  - [Container of random values](#container-of-random-values)
  - [Multi-roll and batch numbers (structured bindings)](#multi-roll-and-batch-numbers-structured-bindings)
  - [In-place fill](#in-place-fill)
  - [Weighted random values](#weighted-random-values)
  - [Sample without replacement](#sample-without-replacement)
  - [Choice with replacement](#choice-with-replacement)
  - [Enum random values (C++26 Reflection)](#enum-random-values-c26-reflection)
  - [Shuffle & std::span views](#shuffle--stdspan-views)
  - [Random string](#random-string)
  - [Hex & UUID / GUID Generator](#hex--uuid--guid-generator)
  - [Common Distributions](#common-distributions)
  - [Call operator (functor)](#call-operator-functor)
  - [Custom distribution](#custom-distribution)
  - [Custom Seeder](#custom-seeder)
  - [Thread local random](#thread-local-random)
  - [Local random](#local-random)
  - [Stream operators](#stream-operators)
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
  * **C++20 Modules Support**. Can be consumed either as a single header file (`ilqvya/random.hpp`) for C++11–C++26 or imported as a native C++20 module (`import ilqvya.random;`) for zero macro leakage, clean encapsulation, and faster compilation times.
## Supported compilers
* GCC 4.9+ (and later)
* Clang 3.7+ (and later)
* Microsoft Visual C++ 2015+ (and later)
## Integration
#### CMake
* **FetchContent** (recommended for CMake 3.11+):
```cmake
include(FetchContent)
FetchContent_Declare(
    random
    GIT_REPOSITORY https://github.com/ilqvya/random.git
    GIT_TAG        v2.0.0
)
FetchContent_MakeAvailable(random)

target_link_libraries(${TARGET} random::random) # or ilqvya::random
```
> Unit tests (`Random_BuildTests`) and install rules (`Random_Install`) are automatically disabled when included via `FetchContent` or `add_subdirectory`.

* **External package** (installed via CMake / package manager):
```cmake
find_package(ilqvya_random REQUIRED)
target_link_libraries(${TARGET} random::random) # or ilqvya::random
```

* **C++20 Module** (`import ilqvya.random;`):
Enable `-DRandom_BuildModules=ON` during CMake configuration (requires CMake 3.28+ and Ninja or Visual Studio generator):
```cmake
find_package(ilqvya_random REQUIRED)
target_link_libraries(${TARGET} ilqvya::random_module)
```

#### Manually
The single required source file, [`random.hpp`](https://github.com/ilqvya/random/blob/master/include/ilqvya/random.hpp), is located in the [`include/ilqvya`](https://github.com/ilqvya/random/tree/master/include/ilqvya) directory. The module interface [`ilqvya_random.cppm`](https://github.com/ilqvya/random/blob/master/src/ilqvya_random.cppm) is in `src/`.
#### Usage
* **Header-only (C++11 through C++26)**:
Add the header to your source file:
```cpp
#include "ilqvya/random.hpp"

// Both ilqvya::random and ilqvya::Random work directly
auto val = ilqvya::Random::get(1, 6);
```

* **C++20 Module (`import ilqvya.random;`)**:
If your toolchain supports C++20 modules, you can import the module directly:
```cpp
import ilqvya.random;

int main() {
    auto dice = ilqvya::random::get(1, 6);
    auto token = ilqvya::random::get_string(16);
    auto sampled = ilqvya::random::sample(population, 3);
}
```
Ensure C++11 or newer is enabled in your compiler settings (e.g., `-std=c++11` for GCC and Clang, or `-std=c++20` when using modules).
## Five-minute tutorial

> [!NOTE]
> All tutorial examples below work identically with either `#include <ilqvya/random.hpp>` or C++20 `import ilqvya.random;`. Both `ilqvya::Random` and `ilqvya::random` are available out of the box in both modes!

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
### Bool & Coin Flip
Generates a random boolean with specified probability in [0.0, 1.0], or convenient Bernoulli aliases:
```cpp
auto val = Random::get<bool>(0.7); // true with 70% probability
auto def = Random::get<bool>();    // true with 50% probability by default

// Fast Bernoulli probability alias & coin flip:
bool heads = Random::flip();               // 50% coin flip (alias for flip(0.5))
bool crit  = Random::chance(0.15);         // true with 15% probability in [0.0, 1.0]
bool lucky = Random::chance_pct(25.0);     // true with 25% percentage in [0.0, 100.0]

// Out of range probabilities throw std::invalid_argument:
// Random::chance(-0.1);
// Random::chance_pct(101.0);
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
### Multi-roll and batch numbers (structured bindings)
Generate a fixed-size `std::array` of random numbers with compile-time size `N`, perfect for unpacking with C++17 structured bindings:
```cpp
// RGB color triple:
auto [r, g, b] = Random::get<std::array, 3>(0, 255);

// 3D coordinate:
auto [x, y, z] = Random::get_array<3>(-1.0f, 1.0f);

// Multi-roll dice:
auto dice = Random::get_array<6>(1, 6); // std::array<int, 6>

// Default limits:
auto arr = Random::get_array<4, int>(); // std::array<int, 4> with default limits
```

### In-place fill
Fill an existing container, iterator range, or built-in array in-place with random values, avoiding reallocations:
```cpp
// Container in-place fill:
std::vector<int> buffer(1024);
Random::fill(buffer, 0, 100);

// Iterator range fill:
Random::fill(buffer.begin(), buffer.begin() + 10, -5, 5);

// Built-in C-style array fill:
int raw[8];
Random::fill(raw, 1, 6);
Random::fill(raw); // default limits
```
### Weighted random values
Selects a random element based on relative weights:

* **Initializer list of weighted values**:
  Returns the selected value directly based on weights using direct zero-allocation selection:
```cpp
    // Explicit return type with initializer list of {value, weight}
    auto val = Random::get<int>( { { 1, 10 }, { 2, 20 }, { 3, 70 } } ); // 10%, 20%, 70% probability
    auto str = Random::get<std::string>( { { "rare", 1.5 }, { "common", 98.5 } } );

    // Explicit weighted_result type
    using WR = Random_t::weighted_result<int, double>;
    auto chosen = Random::get( { WR{ 10, 0.25 }, WR{ 20, 0.75 } } );
```
  * Supports both integer weights (`int`, `unsigned`, `size_t`) and floating-point weights (`float`, `double`).
  * Throws `std::invalid_argument` if the list is empty or if any weight is negative.
  * If all weights are 0, safely returns the first element.
  * `weighted_result` provides `constexpr operator==` and `operator!=`.

* **Variadic syntax** (convenient for 2 to 10 weighted items):
```cpp
    auto val = Random::get<int>( { 1, 3 }, { 4, 4 }, { 10, 5 } ); // returns chosen int based on weights 3, 4, 5
    auto f   = Random::get<float>( { 1.1f, 1 }, { 1.2f, 2 } );
```

* **Containers of pairs or map-like containers** (returns iterator):
```cpp
    std::unordered_map<std::string, unsigned long> nonzero_ulong_umap = { { "Orange", 1ul }, { "Apple", 2ul }, { "Banana", 3ul } };
    std::map<std::string, double> nonzero_double_map = { { "Orange", 1.0 }, { "Apple", 2.0 }, { "Banana", 3.0 } };
    std::vector<std::pair<std::string, int>> nonzero_vec = { { "Orange", 1 }, { "Apple", 2 }, { "Banana", 3 } };

    auto it1 = Random::get<Random_t::weight>( nonzero_ulong_umap );
    auto it2 = Random::get<Random_t::weight>( nonzero_double_map );
    auto it3 = Random::get<Random_t::weight>( nonzero_vec );
```
  * Returns mutable iterator for non-const containers, and const iterator for const containers.
  * Passing temporary rvalue containers is disabled at compile time (`= delete`) to prevent dangling iterators.

### Sample without replacement
Samples $N$ unique elements from a population without replacement (guaranteed no duplicates from population). Available across all C++ standards (C++11 through C++26):

* **Containers and Ranges** (returns container with sampled elements):
```cpp
std::vector<int> population = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

// Returns std::vector<int> containing 4 uniquely sampled elements
auto sample_vec = Random::sample( population, 4 );

// Specify destination container type (e.g. std::deque, std::set, std::unordered_set)
auto sample_deq  = Random::sample<std::deque>( population, 3 );
auto sample_set  = Random::sample<std::set>( population, 4 );
auto sample_uset = Random::sample<std::unordered_set>( population, 4 );
```

* **Built-in Arrays**:
```cpp
int arr[] = { 10, 20, 30, 40, 50 };
auto sampled = Random::sample( arr, 3 ); // std::vector<int> with 3 unique elements
```

* **String Literals and Random String Sampling**:
```cpp
// Samples unique characters excluding the null terminator '\0', returns std::string
auto chars = Random::sample( "abcdefghij", 4 );

// sample_string helper for character ranges or string literals
std::string text = "ABCDEFGHIJ";
auto sampled_str = Random::sample_string( text, 3 );
```

* **Iterator Ranges**:
```cpp
std::vector<int> out;
Random::sample( population.begin( ), population.end( ), std::back_inserter( out ), 4 );
```
If `n <= 0`, returns an empty container or leaves output unmodified.

### Choice with replacement
Selects $N$ random elements from a population with replacement (elements can be selected multiple times):
```cpp
std::vector<int> population = { 1, 2, 3, 4, 5 };

// Returns std::vector<int> containing 10 elements chosen with replacement:
auto chosen = Random::choice( population, 10 );

// Specify destination container type:
auto chosen_deq = Random::choice<std::deque>( population, 5 );

// String literals and choice_string:
auto seq = Random::choice( "ACGT", 16 ); // std::string of 16 DNA bases
auto str = Random::choice_string( std::string{ "0123456789" }, 6 );

// Built-in arrays:
int arr[] = { 10, 20, 30 };
auto choices = Random::choice( arr, 5 );

// Iterator ranges:
std::vector<int> out;
Random::choice( population.begin(), population.end(), std::back_inserter( out ), 4 );
```

### Enum random values (C++26 Reflection)
Returns a random enumerator from any `enum` or `enum class` (requires C++26 with reflection support, e.g. `-std=c++26 -freflection`):
```cpp
enum class Color { Red, Green, Blue };
auto color = Random::get<Color>(); // Returns randomly chosen Color

enum Status { Disconnected = 0, Connecting = 10, Connected = 100 };
auto status = Random::get<Status>(); // Returns randomly chosen Status
```

### Shuffle & std::span views
Reorders the elements in a given range, container, or built-in array [ref](http://en.cppreference.com/w/cpp/algorithm/random_shuffle):
* Iterator range:
```cpp
std::vector<int> vec{ 1, 2, 3, 4, 5 };
Random::shuffle( vec.begin( ), vec.end( ) );
```
* Container & C++20 `std::span` / range views:
```cpp
Random::shuffle( vec );

// C++20 std::span and subranges are supported directly:
std::span<int> sp{ vec };
Random::shuffle( sp );
```
* Built-in array:
```cpp
int array[] = { 1, 2, 3, 4, 5 };
Random::shuffle( array ); // supported for C-style arrays (rvalue arrays deleted for safety)
```

### Random string
Generates a random `std::string` of specified length:
* Default character range (printable characters `' '` to `'~'`, avoiding embedded null bytes and non-printable control characters):
```cpp
auto s  = Random::get_string( 16 );          // 16 random printable ASCII characters
auto ws = Random::get_string<wchar_t>( 16 ); // 16 random printable wide characters
```
* From a character range `[from, to]`:
```cpp
auto token  = Random::get_string( 16, 'a', 'z' ); // 16 random lowercase letters
auto digits = Random::get_string( 6, '0', '9' );  // 6 random digits
```
* From a custom charset (string literal, `std::string`, or container of characters):
```cpp
auto hex = Random::get_string( 32, "0123456789abcdef" ); // 32 random hex characters (excludes '\0')
auto key = Random::get_string( 16, std::string{ "ABCDEFGHJKLMNPQRSTUVWXYZ23456789" } );
```
Throws `std::invalid_argument` if charset container is empty or range is invalid. Empty string literals fail at compile time.

### Hex & UUID / GUID Generator
Generate hexadecimal strings and standard UUIDs / GUIDs or MongoDB ObjectIDs:

> [!WARNING]
> **Not Cryptographically Secure:** Generated values (strings, hex tokens, UUIDs) are driven by pseudo-random engines (`std::mt19937` or `std::philox4x64`) and are **not** cryptographically secure (not a CSPRNG). mt19937 state is predictable from past output. Do not use them for cryptographic secrets, session IDs, authentication tokens, or password reset links.

```cpp
// Hex strings:
auto hex32 = Random::get_hex(32);        // e.g. "a3f1c8..." (lowercase by default)
auto hex16 = Random::get_hex(16, true);  // e.g. "A3F1C8..." (uppercase)

// UUIDs (default is UUIDv4 - RFC 4122 random):
std::string uuid4 = Random::get_uuid(); 
// e.g. "123e4567-e89b-42d3-a456-426614174000"

// Specific UUID versions via ilqvya::uuid_version (RFC 4122 / RFC 9562 / MongoDB):
using ilqvya::uuid_version;
auto v4  = Random::get_uuid(uuid_version::v4);               // RFC 4122 random (default)
auto v1  = Random::get_uuid(uuid_version::v1);               // Gregorian time-based (RFC 4122)
auto v6  = Random::get_uuid(uuid_version::v6);               // Reordered Gregorian time-based (RFC 9562)
auto v7  = Random::get_uuid(uuid_version::v7);               // Unix epoch millisecond time-ordered (RFC 9562, monotonic)
auto nil = Random::get_uuid(uuid_version::nil);              // "00000000-0000-0000-0000-000000000000"
auto max = Random::get_uuid(uuid_version::max);              // "ffffffff-ffff-ffff-ffff-ffffffffffff"
auto oid = Random::get_uuid(uuid_version::mongo_object_id);  // 24 hex characters MongoDB ObjectID (timestamp + process random + counter)
```

### Common Distributions
Direct helpers for non-uniform standard distributions:
```cpp
// Normal (Gaussian) distribution:
auto norm = Random::normal(0.0, 1.0);       // mean = 0.0, stddev = 1.0 (default 0.0, 1.0)
auto nval = Random::normal();               // defaults to mean 0.0, stddev 1.0

// Exponential distribution:
auto exp_val = Random::exponential(1.5);    // lambda rate = 1.5 (default 1.0)

// Poisson distribution:
auto p_val = Random::poisson(4.0);          // mean = 4.0 (default 1.0)
```

### Call operator (functor)
Random generator instances and static classes can be invoked directly using the function call operator `operator()`:
```cpp
auto num = Random( 1, 100 ); // same as Random::get( 1, 100 )

ilqvya::random_local rng;
auto val = rng( 1.0, 5.0 );  // same as rng.get( 1.0, 5.0 )
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
// Or when using C++20 modules:
// import ilqvya.random;

using Random_t = ilqvya::random_local;

int main( ) {
  Random_t localRandom{ }; // Construct on the stack (auto-seeded)
  
  // Custom initialization:
  Random_t seeded{ 42u };                 // Seeded with unsigned integer
  std::seed_seq sseq{ 1, 2, 3, 4, 5 };
  Random_t from_seq{ sseq };              // Seeded with std::seed_seq
  std::mt19937 custom_eng{ 12345 };
  Random_t from_eng{ custom_eng };        // Initialized from an existing engine

  // Access via dot operator or call operator
  auto val = localRandom.get( -10, 10 );
  auto roll = localRandom( 1, 6 );

  // Generators can be compared for identical engine state
  if ( seeded == from_seq ) { /* ... */ }
  
} // Destroy localRandom and free stack memory
```
### Stream operators
In addition to `serialize` and `deserialize`, generators support standard stream insertion (`<<`) and extraction (`>>`) operators:
```cpp
std::stringstream ss;
ss << Random; // Serializes internal engine state into stream

ss >> Random; // Restores internal engine state from stream
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
const auto& const_engine = Random::engine( );
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

Compares the internal pseudo-random number engine with another engine or generator:
```cpp
Random::engine_type otherEngine;
bool isSame = Random::is_equal( otherEngine );

ilqvya::random_local rng1, rng2;
if ( rng1 == rng2 ) { /* ... */ }
if ( rng1 != rng2 ) { /* ... */ }
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
