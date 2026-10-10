# Project Review & Improvement Roadmap (`effolkronium/random`)

This document summarizes the comprehensive architectural, algorithmic, build-system, and testing review of the `effolkronium/random` library, along with actionable steps to resolve bugs, enhance reliability, and modernize the codebase.

---

## Executive Summary

| Category | Severity | Key Highlights |
| :--- | :--- | :--- |
| **Correctness & Logic** | **High** | `numeric_limits::min()` float range bug, broken weighted float sampling with `nextafter`, `{{ 0 }}` compile failure on non-scalar types, signed weights rejected. |
| **Architecture & Code Health** | **Medium-High** | ~500 lines of copy-pasted code between static and local classes; awkward API divergence requiring macro hacks (`DOT`) in tests. |
| **Entropy & Seeding Quality** | **Medium-High** | `std::mt19937` (19,937 bits of internal state) is seeded with only 2 words, leaving the vast majority of generator states unreachable. |
| **Modern C++ Evolution** | **Medium** | Locked to C++11; missing `[[nodiscard]]`, `noexcept`, `char8_t`, C++17 `if constexpr`, and C++20 Ranges. |
| **CMake & Packaging** | **Medium** | Deprecated `export(PACKAGE)`, missing namespaced `ALIAS` target (`effolkronium::random`), non-standard install paths. |
| **Testing & CI** | **Medium** | Bundled Catch v1 (2017), infinite loop hazards in tests, `REQUIRE(true == true)` dummy assertions, lack of GitHub Actions CI. |

---

## 1. Correctness Bugs & Behavioral Flaws

### 1.1 `std::numeric_limits<float>::min()` Is Positive, Not Lowest
* **Location:** `include/effolkronium/random.hpp` (around line 346)
* **Code:**
  ```cpp
  template<typename T>
  static typename std::enable_if<details::is_uniform_real<T>::value, T>::type 
  get( T from = (std::numeric_limits<T>::min)( ),
       T to = (std::numeric_limits<T>::max)( ) )
  ```
* **Problem:** In C++, `std::numeric_limits<T>::min()` for floating-point types (`float`, `double`) returns the **smallest positive normalized number** ($\approx 1.17549 \times 10^{-38}$ for `float`), **not** the lowest negative number.
* **Impact:** 
  1. `Random::get<float>()` will never generate `0.0` or any negative value; its range is restricted to $[10^{-38}, 10^{38}]$.
  2. If the user expects the entire negative-to-positive float range, the correct function is `std::numeric_limits<T>::lowest()`.
  3. Furthermore, callers invoking `Random::get<float>()` without arguments in math, gaming, or simulation applications overwhelmingly expect a normalized distribution $[0.0, 1.0)$, not astronomical numbers up to $3.4 \times 10^{38}$.
* **Remediation:** 
  - Change the default range for real types without arguments to $[0.0, 1.0)$.
  - If a full-range overload is explicitly desired, use `std::numeric_limits<T>::lowest()`.

---

### 1.2 `Container<A, N> container = {{ 0 }};` Compilation Failure
* **Location:** `include/effolkronium/random.hpp` (lines 587 and 615)
* **Code:**
  ```cpp
  Container<A, N> container = {{ 0 }};
  for (std::size_t i = 0; i < N; ++i)
      container[i] = get(start, end);
  ```
* **Problem:** If `Container` is `std::array<std::string, N>` or `std::array<CustomStruct, N>` where elements cannot be constructed from integer literal `0`, this triggers a **hard compilation failure**.
* **Remediation:** Use default or value initialization: `Container<A, N> container{};`. The initial zeroing is redundant because every element index $0 \dots N-1$ is immediately overwritten inside the loop.

---

### 1.3 Weighted Random: Signed Integers (`int`) Fail to Compile
* **Location:** `include/effolkronium/random.hpp` (lines 676 and 716)
* **Code:**
  ```cpp
  !std::is_signed<typename MapContainer::mapped_type>::value &&  // Overload 1 (unsigned int)
  details::is_uniform_real<typename MapContainer::mapped_type>::value // Overload 2 (float)
  ```
* **Problem:** If a user passes a standard `std::map<std::string, int>`, both SFINAE overloads fail because `int` is signed and is not `uniform_real`. Users are unexpectedly forced to use `unsigned int` or `unsigned long`.
* **Remediation:** Support signed integer types as weights, with an assertion or validation that weights are non-negative ($\ge 0$).

---

### 1.4 Weighted Random: `nextafter(..., min())` Flaw & Silent `end()` Return
* **Location:** `include/effolkronium/random.hpp` (lines 729 and 1418)
* **Code:**
  ```cpp
  MappedType random_weight = get(MappedType(0), std::nextafter(total_weight, (std::numeric_limits<MappedType>::min)( )));
  ```
* **Problems:**
  1. `(std::numeric_limits<MappedType>::min)()` is positive. If `total_weight` is subnormal (smaller than `min()`), `nextafter` steps **upward** towards `min()`, resulting in a number greater than `total_weight`.
  2. Because floating-point addition accumulates precision error, `sum > random_weight` can evaluate to false for all elements if `random_weight` was sampled close to `total_weight`. When that happens, the function falls through and returns `std::end(map_container)`. Callers dereferencing the returned iterator encounter **undefined behavior / segmentation fault**.
  3. The current implementation runs in $O(N)$ on every single call because it traverses the map twice (once to sum weights, once to pick the item).
* **Remediation:** 
  - Ensure fallback safety: if loop finishes without returning, return the last non-zero weight element rather than `end()`.
  - Alternatively, leverage standard distributions designed for weighted sampling, such as `std::discrete_distribution` or `std::piecewise_constant_distribution`.

---

### 1.5 Mismatched Trait Check in `common_type` Container Generation
* **Location:** `include/effolkronium/random.hpp` (lines 519–520)
* **Code:**
  ```cpp
  template<
      template<typename...> class Container,
      typename Key,
      typename A,
      typename B,
      typename C = typename std::common_type<A, B>::type>
  static typename std::enable_if<
             std::is_same<Key, common>::value
          && details::has_reserve<Container<A>>::value // <--- Checks Container<A>!
      , Container<C>>::type get(A start, B end, std::size_t size)
  ```
* **Problem:** The return type is `Container<C>`, but the reserve trait check checks `Container<A>`. In line 566, it checks `!details::has_reserve<Container<C>>::value`. Both should check `Container<C>`.

---

### 1.6 SFINAE Trait Ambiguity in `has_reserve` and `has_insert`
* **Location:** `include/effolkronium/random.hpp` (lines 123–150)
* **Code:**
  ```cpp
  template <typename C>
  static long test(decltype(&C::reserve));
  ```
* **Problem:** Taking the address `&C::reserve` or `&C::insert` fails if the member function is overloaded (e.g. `std::vector::insert` has multiple overloads). In C++, taking `&C::insert` without an explicit function pointer type can lead to ambiguous template substitutions or false negatives.
* **Remediation:** Use expression SFINAE with `declval`:
  ```cpp
  template <typename C, typename = void>
  struct has_reserve : std::false_type {};

  template <typename C>
  struct has_reserve<C, details::void_t<decltype(std::declval<C&>().reserve(std::declval<std::size_t>()))>> : std::true_type {};
  ```

---

## 2. Architecture & Code Duplication

### 2.1 500+ Lines Duplicated Between Static and Local Classes
* **Problem:** 
  - Lines 195–765 define `basic_random_base` (570 lines of static methods calling `Derived::engine()`).
  - Lines 901–1470 define `basic_random_local` (570 lines of identical methods implemented as non-static member functions).
  - Even stranger, `basic_random_local` derives from `basic_random_base`, but completely re-declares every single function.
* **Impact:** Any bug fix or feature addition must be manually copied and kept in sync across both halves of the header file.
* **Remediation:** Decouple algorithms from storage strategy:
  1. Create a stateless core namespace or helper class (e.g., `random_impl`) containing static methods taking `Engine& engine` as their first argument.
  2. Implement `basic_random_static`, `basic_random_thread_local`, and `basic_random_local` as thin wrappers that forward calls with their respective engine instance (`static`, `thread_local`, or `member`).
  3. This eliminates ~500 lines of duplicate code (~40% of the entire file) and guarantees consistency.

### 2.2 The `Random DOT get` Macro Hack in Tests
* **Problem:** In `test/current_random.hpp`:
  ```cpp
  #ifndef RANDOM_LOCAL
  #define DOT ::
  #else
  #define DOT .
  #endif
  ```
* **Impact:** Because `random_static` only exposes static functions (`::get`) and `random_local` only exposes member functions (`.get`), generic code cannot easily be written to accept any random generator without macros.
* **Remediation:** Provide instance member methods (`.get(...)`) and `operator()` on all generator types, enabling uniform syntax across all storage policies.

---

## 3. RNG Quality & State Space

### 3.1 Severe Under-seeding of `std::mt19937`
* **Location:** `include/effolkronium/random.hpp` (lines 171–176)
* **Code:**
  ```cpp
  std::seed_seq seed_seq{ {
      static_cast<std::uintmax_t>( std::random_device{ }( ) ),
      static_cast<std::uintmax_t>( std::chrono::steady_clock::now( ).time_since_epoch( ).count( ) ),
  } };
  ```
* **Problem:** `std::mt19937` has an internal state of **19,937 bits (624 32-bit words)**.
* Supplying only 2 words ($\approx 64$ bits of entropy) restricts the generator so that only $2^{64}$ out of $2^{19937}$ states can ever be reached.
* **Remediation:** Initialize `std::seed_seq` with an array of 624 32-bit values drawn from `std::random_device` (or high-resolution entropy sources), or allow user configuration of full-state entropy initialization.

---

## 4. Modern C++ Evolution (C++14 / C++17 / C++20 / C++23)

1. **`[[nodiscard]]` Attribute:**
   - Functions like `get(...)`, `min()`, `max()`, and `is_equal()` should be marked `[[nodiscard]]` to prevent bugs where callers forget to capture or use the generated random value.
2. **`noexcept` Specification:**
   - Mark `min()`, `max()`, `discard()`, moving, and basic accessors as `noexcept`.
3. **C++17 `if constexpr`:**
   - Replace complex SFINAE enable_if hierarchies (e.g., reserve vs. non-reserve containers, signed vs. unsigned types) with clean `if constexpr` branches.
4. **C++20 Character Types:**
   - Add support for `char8_t` in `is_supported_character`.
5. **C++20 Ranges & Concepts:**
   - Support `std::ranges::range`, `std::span`, and concept-based constraints (`std::integral`, `std::floating_point`) when compiled in C++20 mode.

---

## 5. CMake & Build System Modernization

1. **Add Namespaced Alias Target:**
   - Add modern CMake alias so consumers can use standard namespacing:
     ```cmake
     add_library(effolkronium::random ALIAS effolkronium_random)
     ```
2. **Declare Interface Compile Features:**
   - Currently, consumers don't automatically inherit the C++ standard flag:
     ```cmake
     target_compile_features(effolkronium_random INTERFACE cxx_std_11)
     ```
3. **Remove Deprecated `export(PACKAGE)`:**
   - `export(PACKAGE ...)` writes into `~/.cmake/packages/` during build and has been deprecated since CMake 3.15 due to unexpected cross-project interference.
4. **Standardize Installation Destinations:**
   - Installing CMake config files to `${CMAKE_INSTALL_PREFIX}/cmake` is non-standard. Use `lib/cmake/effolkronium_random` or `share/cmake/effolkronium_random`.
5. **Conditional Test Building for Subprojects:**
   - Prevent building unit tests when included as a subproject via `add_subdirectory`:
     ```cmake
     if(Random_BuildTests AND (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME))
         enable_testing()
         add_subdirectory(test)
     endif()
     ```

---

## 6. Test Suite & CI/CD Improvements

1. **Replace Bundled Catch v1.9.3 (2017):**
   - `test/catch.hpp` is a 13,000-line Catch1 single header from 2017. Catch1 is unmaintained and causes warnings on modern compilers.
   - Upgrade to **Catch2 v3**, **doctest**, or **GoogleTest** via `FetchContent` or package manager.
2. **Eliminate Infinite Loop Hazards in Tests:**
   - In `test/random_test.cpp`:
     ```cpp
     while( Random DOT get<bool>( ) == Random DOT get<bool>( ) );
     ```
     ```cpp
     do { Random DOT shuffle( arr ); } while( arr_copy == arr );
     ```
   - If an engine produces deterministic or repeating output, these loops hang the test runner indefinitely. Use bounded iterations (e.g. `for (int i = 0; i < 100; ++i)`).
3. **Remove Meaningless Assertions:**
   - Replace dummy assertions like `REQUIRE( true == true );` with assertions verifying the actual test outcomes or counts.
4. **Fix `std::set` Size Test:**
   - `REQUIRE(4 == set.size());` assumes generated values never collide. If duplicate numbers are drawn, `std::set` drops duplicates and fails the test. The test should account for possible duplicate elimination.
5. **Add GitHub Actions CI:**
   - AppVeyor is currently the only CI. Add `.github/workflows/ci.yml` matrix testing Linux (GCC, Clang), macOS (AppleClang), and Windows (MSVC) with multiple C++ standards (C++11, C++14, C++17, C++20).
6. **Fix `appveyor_build_win.bat` Redundancy:**
   - Lines 4–7 contain duplicated conditional blocks for VS 2019 and VS 2022 that conflict with `%GEN%` and `%ARCH%`. Clean up and simplify the batch script.

---

## 7. Action Plan / Priority Roadmap

- [x] **Phase 1: Correctness & Bug Fixes**
  - [x] Fix `numeric_limits::min()` to `lowest()` / change default float range to $[0.0, 1.0)$.
  - [x] Fix `Container<A, N> container = {{ 0 }};` compile error on non-scalar types.
  - [x] Allow signed integer types in weighted random selection.
  - [x] Fix `nextafter` and fallback behavior in weighted float selection.
  - [x] Fix container `common_type` reserve trait check (`Container<C>`).
  - [x] Fix expression SFINAE on `has_reserve` and `has_insert`.
- [x] **Phase 2: Refactoring & Architecture**
  - [x] Decouple core algorithms into stateless functions taking `Engine&`.
  - [x] Remove ~500 lines of duplicate code between `basic_random_base` and `basic_random_local`.
  - [x] Provide uniform member and static calling syntax.
- [x] **Phase 3: Quality & Entropy**
  - [x] Expand default seeder entropy to fully initialize MT19937 state (624 words).
  - [x] Add `[[nodiscard]]` (`EFFOLKRONIUM_NODISCARD`) and `noexcept` specifications.
  - [x] Conditionally support `char8_t` in `is_supported_character`.
- [x] **Phase 4: Modern CMake & CI/CD**
  - [x] Add `effolkronium::random` ALIAS target and `cxx_std_11` compile features.
  - [x] Remove `export(PACKAGE)` and standardize install destination.
  - [x] Make Catch compatible with modern C++ (C++17/C++20) while preserving pure C++11 compatibility.
  - [x] Eliminate infinite loop hazards and dummy assertions in test suite.
  - [x] Fix `appveyor_build_win.bat` VS 2019/2022 generator & arch setup.
  - [x] Add GitHub Actions multi-platform matrix CI workflow.
