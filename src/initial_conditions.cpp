#include "gray_scott/initial_conditions.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

/* All backends start from the same host-side fields. U starts at 1 and V at 0;
    each selected pattern changes some cells to U=0.5 and V=0.25. Keeping this
    here prevents the CPU and CUDA versions from quietly starting differently. */
void initialize_fields(const SimulationConfig& config,
                       std::vector<float>& u,
                       std::vector<float>& v) {
    if (config.width < 3 || config.height < 3) {
        throw std::invalid_argument("The grid must be at least 3 by 3.");
    }

    const std::size_t count = static_cast<std::size_t>(config.width) * config.height;
    u.assign(count, 1.0f);
    v.assign(count, 0.0f);

    std::mt19937 random(config.seed);
    std::uniform_real_distribution<float> noise(-0.02f, 0.02f);
    const int center_x = config.width / 2;
    const int center_y = config.height / 2;

    // Every pattern changes the same small patch of the otherwise uniform U/V field.
    for (int y = 0; y < config.height; ++y) {
        for (int x = 0; x < config.width; ++x) {
            bool active = false;
            if (config.initial_condition == InitialCondition::CenterSquare) {
                active = std::abs(x - center_x) < config.width / 10 &&
                         std::abs(y - center_y) < config.height / 10;
            } else if (config.initial_condition == InitialCondition::TwoCircles) {
                const int radius = config.width / 10;
                const int left_x = config.width / 3;
                const int right_x = config.width * 2 / 3;
                const bool inside_left = (x - left_x) * (x - left_x) +
                    (y - center_y) * (y - center_y) < radius * radius;
                const bool inside_right = (x - right_x) * (x - right_x) +
                    (y - center_y) * (y - center_y) < radius * radius;
                active = inside_left || inside_right;
            } else if (config.initial_condition == InitialCondition::RandomSpots) {
                active = noise(random) > 0.015f;
            } else if (config.initial_condition == InitialCondition::ConcentricRings) {
                const float radius = std::sqrt(static_cast<float>((x - center_x) * (x - center_x) +
                                                                   (y - center_y) * (y - center_y)));
                const int ring = static_cast<int>(radius) / std::max(2, config.width / 16);
                active = ring % 2 == 0;
            } else {
                const int dx = std::abs(x - center_x);
                const int dy = std::abs(y - center_y);
                const int radius_squared = dx * dx + dy * dy;
                const int limit = config.width / 3;
                active = radius_squared < limit * limit &&
                         (dx < config.width / 32 || dy < config.height / 32 ||
                          std::abs(dx - dy) < config.width / 32);
            }

            if (active) {
                const std::size_t index = static_cast<std::size_t>(y) * config.width + x;
                u[index] = 0.5f;
                v[index] = 0.25f;
            }
        }
    }
}
