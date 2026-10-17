#include "ilqvya/random.hpp"

int main() {
    const auto num = ilqvya::random::get(1, 10);
    if (num < 1 || num > 10) {
        return 1;
    }
    return 0;
}
