# Random for modern C++ with convenient API
[![Build Status](https://travis-ci.org/effolkronium/random.svg?branch=develop)](https://travis-ci.org/effolkronium/random)
[![Build status](https://ci.appveyor.com/api/projects/status/xv0aq60p91j1jnjr/branch/develop?svg=true)](https://ci.appveyor.com/project/effolkronium/random/branch/develop)
[![Coverage Status](https://coveralls.io/repos/github/effolkronium/random/badge.svg?branch=develop)](https://coveralls.io/github/effolkronium/random?branch=develop)
<a href="https://scan.coverity.com/projects/effolkronium-random">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/12707/badge.svg"/>
</a>
- [Design goals](#design-goals)
- [Integration](#integration)
- [Examples](#examples)
## Design goals
There are few ways to get working with random in C++:
- **C style**
```cpp
  srand( time(NULL) ); // seed with time since epoch
  rand() % (9 - 1)) + 1; // get a pseudo-random integer between 1 and 9
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
  dist(engine); // finally get a random number
```
* Problems
  * should specify seed
  * should choose, create and use a chain of various objects like engines and distributions
  * mt19937 use 5000 bytes of memory for each creation
  * Uncomfortable and not intuitively clear
- **effolkronium random style**
```cpp
  // auto seeded
  Random::get(1, 9); // invoke 'get' method to generate  a pseudo-random integer between 1 and 9
  // yep, that's all.
```
* Advantages
  * **Intuitive syntax**. You can do almost everything with random by simple 'get' method, like getting simple numbers, bools, random object from given set or using custom distribution.
  * **Trivial integration**. All code consists of a single header file [`random.hpp`](https://github.com/effolkronium/random/blob/develop/source/random.hpp). Tahat's it. No library, no subproject, no dependencies, no complex build system. The class is written in vanilla C++11. All in all, everything should require no adjustment of your compiler flags or project settings.
  * **Usability**. There is a 3 versions of random: 
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
### Getting simple numbers
```cpp
auto val = Random::get(-10, 10) //decltype(val) is int
```
```cpp
// you able to use range from greater to lower
auto val = Random::get(10.l, -10.l) // decltype(val) is long double
```
```cpp
auto val = Random::get(10u, -10.5) // COMPILE ERROR: Implicit conversions are not allowed here.
```
