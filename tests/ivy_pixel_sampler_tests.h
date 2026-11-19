#pragma once

class IvyPixelSamplerTests {    
public:
    bool run_all();

private:
    bool run_test(const char* name, bool (IvyPixelSamplerTests::*test)());
    bool expect(bool condition, const char* message);

    bool test_initial_state();
    bool test_set_pixel_data();
    bool test_reject_invalid_dimensions();
    bool test_reject_incorrect_data_size();
    bool test_failed_update_preserves_data();
    bool test_sample_invalid_returns_zero();
    bool test_sample_bilinear();
    bool test_sample_uv_wrapping();


    int m_passed = 0;
    int m_failed = 0;
};
