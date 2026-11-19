#include "ivy_random_tests.h"
#include "ivy_wall_tests.h"

int main() {
    IvyRandomTests random_tests;
    IvyWallTests wall_tests;

    const bool random_passed = random_tests.run_all();
    const bool wall_passed = wall_tests.run_all();

    return random_passed && wall_passed ? 0 : 1;
}