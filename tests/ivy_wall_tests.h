#pragma once

class IvyWallTests {
public:
    bool run_all();

private:
    bool test_local_to_world();
    bool test_world_to_local();
    bool test_round_trip();
    bool test_rotated_wall();
    bool test_get_normal();
    bool test_contains_local_point();
    bool test_local_to_uv();
	bool test_uv_scale_offset();
	bool test_is_on_surface();

	bool expect(bool condition, const char* message);

    int m_passed = 0;
    int m_failed = 0;
};
