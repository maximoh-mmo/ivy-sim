#include "ivy_random_tests.h"
#include "ivy_wall_tests.h"
#include "ivy_pixel_sampler_tests.h"

int main() {
    IvyRandomTests random_tests;
    IvyWallTests wall_tests;
    IvyPixelSamplerTests pixel_sampler_tests;
    const bool random_passed = random_tests.run_all();
    const bool wall_passed = wall_tests.run_all();
    const bool pixel_sampler_passed = pixel_sampler_tests.run_all();

    return random_passed && wall_passed && pixel_sampler_passed ? 0 : 1;
}