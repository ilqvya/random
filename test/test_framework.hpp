#ifndef ILQVYA_TEST_FRAMEWORK_HPP
#define ILQVYA_TEST_FRAMEWORK_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace test_framework {

struct TestCase {
    const char* name;
    void (*func)();
};

inline std::vector<TestCase>& get_test_registry() {
    static std::vector<TestCase> registry;
    return registry;
}

inline void register_test(const char* name, void (*func)()) {
    get_test_registry().push_back(TestCase{ name, func });
}

struct TestRegistrar {
    TestRegistrar(const char* name, void (*func)()) {
        register_test(name, func);
    }
    TestRegistrar(const char* name, const char*, void (*func)()) {
        register_test(name, func);
    }
};

struct TestFailureException : std::exception {
    const char* what() const noexcept override {
        return "Test assertion failed";
    }
};

inline const char*& get_current_test_name() {
    static const char* name = nullptr;
    return name;
}

inline const char*& get_current_section() {
    static const char* sec = nullptr;
    return sec;
}

inline bool& get_current_test_failed() {
    static bool failed = false;
    return failed;
}

inline std::size_t& get_total_assertions() {
    static std::size_t total = 0;
    return total;
}

inline std::size_t& get_failed_assertions() {
    static std::size_t failed = 0;
    return failed;
}

inline void record_assertion(bool success, const char* expr, const char* file, int line, bool is_require) {
    ++get_total_assertions();
    if (!success) {
        ++get_failed_assertions();
        get_current_test_failed() = true;
        std::cerr << "  [FAILED] " << (get_current_test_name() ? get_current_test_name() : "unnamed test");
        if (get_current_section()) {
            std::cerr << " -> " << get_current_section();
        }
        std::cerr << "\n    at " << file << ":" << line << "\n";
        std::cerr << "    Assertion: " << expr << "\n\n";

        if (is_require) {
            throw TestFailureException();
        }
    }
}

struct SectionGuard {
    const char* prev_section;

    explicit SectionGuard(const char* name)
        : prev_section(get_current_section()) {
        get_current_section() = name;
    }

    ~SectionGuard() {
        get_current_section() = prev_section;
    }

    explicit operator bool() const noexcept {
        return true;
    }
};

inline int run_all_tests() {
    const auto& tests = get_test_registry();
    std::size_t passed_tests = 0;
    std::size_t failed_tests = 0;

    std::cout << "Running " << tests.size() << " test cases...\n";
    const auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& test : tests) {
        get_current_test_name() = test.name;
        get_current_section() = nullptr;
        get_current_test_failed() = false;

        try {
            test.func();
        } catch (const TestFailureException&) {
            get_current_test_failed() = true;
        } catch (const std::exception& e) {
            std::cerr << "  [EXCEPTION] " << test.name << " threw: " << e.what() << "\n\n";
            get_current_test_failed() = true;
            ++get_failed_assertions();
        } catch (...) {
            std::cerr << "  [EXCEPTION] " << test.name << " threw an unknown exception\n\n";
            get_current_test_failed() = true;
            ++get_failed_assertions();
        }

        if (get_current_test_failed()) {
            ++failed_tests;
        } else {
            ++passed_tests;
        }
    }

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    const std::size_t total_assertions = get_total_assertions();
    const std::size_t failed_assertions = get_failed_assertions();
    const std::size_t passed_assertions = total_assertions - failed_assertions;

    std::cout << "==================================================\n";
    std::cout << "Tests:      " << passed_tests << " passed";
    if (failed_tests > 0) {
        std::cout << ", " << failed_tests << " failed";
    }
    std::cout << " (" << tests.size() << " total)\n";

    std::cout << "Assertions: " << passed_assertions << " passed";
    if (failed_assertions > 0) {
        std::cout << ", " << failed_assertions << " failed";
    }
    std::cout << " (" << total_assertions << " total)\n";
    std::cout << "Duration:   " << elapsed_ms << " ms\n";
    std::cout << "==================================================\n";

    if (failed_tests == 0 && failed_assertions == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return 0;
    } else {
        std::cerr << "TESTS FAILED\n";
        return 1;
    }
}

} // namespace test_framework

#if defined(__GNUC__) || defined(__clang__)
#    define ILQVYA_TEST_UNUSED __attribute__((unused))
#else
#    define ILQVYA_TEST_UNUSED
#endif

#define ILQVYA_TEST_CONCAT_INNER( a, b ) a##b
#define ILQVYA_TEST_CONCAT( a, b ) ILQVYA_TEST_CONCAT_INNER( a, b )

#define TEST_CASE( ... ) \
    static void ILQVYA_TEST_CONCAT( ilqvya_test_case_func_, __LINE__ )(); \
    static const ::test_framework::TestRegistrar ILQVYA_TEST_CONCAT( ilqvya_test_case_reg_, __LINE__ ) \
        ILQVYA_TEST_UNUSED( \
            __VA_ARGS__, \
            &ILQVYA_TEST_CONCAT( ilqvya_test_case_func_, __LINE__ ) \
        ); \
    static void ILQVYA_TEST_CONCAT( ilqvya_test_case_func_, __LINE__ )()

#define SECTION( name ) \
    if ( const ::test_framework::SectionGuard ILQVYA_TEST_CONCAT( _ilqvya_sec_, __LINE__ ) ILQVYA_TEST_UNUSED{ name } )

#define REQUIRE( expr ) \
    ::test_framework::record_assertion( static_cast<bool>( expr ), #expr, __FILE__, __LINE__, true )

#define CHECK( expr ) \
    ::test_framework::record_assertion( static_cast<bool>( expr ), #expr, __FILE__, __LINE__, false )

#define REQUIRE_FALSE( expr ) \
    ::test_framework::record_assertion( !static_cast<bool>( expr ), "!" #expr, __FILE__, __LINE__, true )

#define CHECK_FALSE( expr ) \
    ::test_framework::record_assertion( !static_cast<bool>( expr ), "!" #expr, __FILE__, __LINE__, false )

#ifndef ILQVYA_NO_TEST_MAIN
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return ::test_framework::run_all_tests();
}
#endif

#endif // ILQVYA_TEST_FRAMEWORK_HPP
