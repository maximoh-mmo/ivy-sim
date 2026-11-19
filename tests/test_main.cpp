#include "ivy_random_tests.h"

int main() {
    IvyRandomTests tests;

    return tests.run_all() ? 0 : 1;
}