# Random for modern C++ with convenient API
[![Build Status](https://travis-ci.org/effolkronium/random.svg?branch=develop)](https://travis-ci.org/effolkronium/random)
[![Build status](https://ci.appveyor.com/api/projects/status/vq1kodqqxwx16rfv/branch/develop?svg=true)](https://ci.appveyor.com/project/effolkronium/random/branch/develop)
[![Coverage Status](https://coveralls.io/repos/github/effolkronium/random/badge.svg?branch=develop)](https://coveralls.io/github/effolkronium/random?branch=develop)
<a href="https://scan.coverity.com/projects/effolkronium-random">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/12707/badge.svg"/>
</a>
- [Design goals](#design-goals)
- [Integration](#integration)
- [Examples](#examples)
  - [Number range](#number-range)
  - [Common type number range](#common-type-number-range)
  - [Bool](#bool)
  - [Random value from std::initilizer_list](#random-value-from std::initilizer_list)
## Design goals
There are few ways to get working with random in C++:
- **C style**
```cpp
  srand( time(NULL) ); // seed with time since epoch
  auto random_number = rand() % (9 - 1)) + 1; // get a pseudo-random integer between 1 and 9
```
* Problems
  * should specify seed before using rand() function
  * should write your own distribution algorihtm
  * [There are no guarantees as to the quality of the random sequence produced.](http://en.cppreference.com/w/cpp/numeric/random/rand#Notes)
- **C++11 style**
```cpp
  std::random_device random_device; // create object for seeding
  std::mt19937 engine{random_device()}; // create engine and seed it
  std::uniform_int_distribution<> dist(1,9); // create distribution for integers with [1, 9] range
  auto random_number = dist(engine); // finally get a random number
```
* Problems
  * should specify seed
  * should choose, create and use a chain of various objects like engines and distributions
  * mt19937 use 5000 bytes of memory for each creation
  * Uncomfortable and not intuitively clear
- **effolkronium random style**
```cpp
  // auto seeded
  auto random_number = Random::get(1, 9); // invoke 'get' method to generate  a pseudo-random integer between 1 and 9
  // yep, that's all.
```
* Advantages
  * **Intuitive syntax**. You can do almost everything with random by simple 'get' method, like getting simple numbers, bools, random object from given set or using custom distribution.
  * **Trivial integration**. All code consists of a single header file [`random.hpp`](https://github.com/effolkronium/random/blob/develop/source/random.hpp). Tahat's it. No library, no subproject, no dependencies, no complex build system. The class is written in vanilla C++11. All in all, everything should require no adjustment of your compiler flags or project settings.
  * **Usability**. There are 3 versions of random: 
    * *random_static* which has static methods and static internal state. It's not thread safe but more efficient
    * *random_thread_local* which has static methods and [thread_local](http://en.cppreference.com/w/cpp/keyword/thread_local) internal state. It's thread safe but less efficient
    * *random_local* which has non static methods and local internal state. It can be created at local scope
## Integration
The single required source, file `random.hpp` is in the `source` directory.
All you need to do is add
```cpp
#include "random.hpp"

// get base random alias which is auto seeded and has static API and internal state
using Random = effolkronium::random_static;
```
to the files you want to use effolkronium random class. That's it. Do not forget to set the necessary switches to enable C++11 (e.g., `-std=c++11` for GCC and Clang).
## Examples
### Number range
Returns a random number between first and second argument.
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
auto val = Random::get<Random::common>(1, 1.f) // decltype(val) is float
```
```cpp
auto val = Random::get<Random::common>(10ul, 10ull) // decltype(val) is unsigned long long
```
```cpp
auto val = Random::get<Random::common>(10.l, 10.f) // decltype(val) is long double
```
```cpp
auto val = Random::get<Random::common>(10u, 10) // Error: prevent conversion from signed to unsigned
```
### Bool
Generate bool with [0; 1] probability
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
```cpp
auto val = Random::get({1, 2, 3}) // val = 1 or 2 or 3
```
