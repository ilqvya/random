# EasyRandom
[![Build Status](https://travis-ci.org/effolkronium/EasyRandom.svg?branch=develop)](https://travis-ci.org/effolkronium/EasyRandom)
[![Build status](https://ci.appveyor.com/api/projects/status/xv0aq60p91j1jnjr/branch/develop?svg=true)](https://ci.appveyor.com/project/effolkronium/easyrandom/branch/develop)
[![Coverage Status](https://coveralls.io/repos/github/effolkronium/EasyRandom/badge.svg?branch=develop)](https://coveralls.io/github/effolkronium/EasyRandom?branch=develop)
<a href="https://scan.coverity.com/projects/effolkronium-easyrandom">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/12707/badge.svg"/>
</a>

Random without extra actions for modern C++
- [Design goals](#design-goals)
- [Integration](#integration)
- [Examples](#examples)

## Design goals

There are a few ways to get working with random in C++. EasyRandom class had these design goals:

- **Intuitive syntax**. You can do almost everything with random by simple 'get' method

- **Trivial integration**. All code consists of a single header file [`EasyRandom.hpp`](https://github.com/effolkronium/EasyRandom/blob/develop/source/EasyRandom.hpp). That's it. No library, no subproject, no dependencies, no complex build system. The class is written in vanilla C++11. All in all, everything should require no adjustment of your compiler flags or project settings.
## Example
```cpp
Random::get();
```
