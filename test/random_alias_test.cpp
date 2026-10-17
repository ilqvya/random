#include "ilqvya/random.hpp"
#include <cassert>

int main() {
    auto num = ilqvya::random::get(1, 10);
    assert(num >= 1 && num <= 10);
    return 0;
}
