#include "gray_scott/simulation.hpp"
#include "gray_scott/initial_conditions.hpp"

#include <algorithm>

/*
    This is the plain reference version of the model. Keeping it readable is
    useful when checking that the CUDA kernel is applying the same equations.
    Each step uses the four-neighbor discrete Laplacian and explicit Euler
    integration, then swaps the old and new grids together.
*/

namespace {

class CpuSimulator final : public Simulator {
public:
    void reset(const SimulationConfig& config) override {
        config_ = config;
        const std::size_t count = static_cast<std::size_t>(config.width) * config.height;
        next_u_.resize(count);
        next_v_.resize(count);
        initialize_fields(config_, u_, v_);
    }

    void step() override {
        for (int y = 0; y < config_.height; ++y) {
            for (int x = 0; x < config_.width; ++x) {
                const std::size_t index = at(x, y);
                const float u = u_[index];
                const float v = v_[index];
                const float laplace_u = laplace(u_, x, y);
                const float laplace_v = laplace(v_, x, y);
                const float reaction = u * v * v;

                next_u_[index] = u + config_.timestep *
                    (config_.diffusion_u * laplace_u - reaction + config_.feed * (1.0f - u));
                next_v_[index] = v + config_.timestep *
                    (config_.diffusion_v * laplace_v + reaction - (config_.feed + config_.kill) * v);
            }
        }
        u_.swap(next_u_);
        v_.swap(next_v_);
    }

    void snapshot(Frame& frame) override {
        frame.width = config_.width;
        frame.height = config_.height;
        frame.value.resize(v_.size());
        std::copy(v_.begin(), v_.end(), frame.value.begin());
    }

    void finish() override {
    }

private:
    std::size_t at(int x, int y) const {
        // Wrapping makes patterns flow across an edge instead of hitting a wall.
        x = (x + config_.width) % config_.width;
        y = (y + config_.height) % config_.height;
        return static_cast<std::size_t>(y) * config_.width + x;
    }

    float laplace(const std::vector<float>& field, int x, int y) const {
        return field[at(x - 1, y)] + field[at(x + 1, y)] +
               field[at(x, y - 1)] + field[at(x, y + 1)] - 4.0f * field[at(x, y)];
    }

    SimulationConfig config_;
    std::vector<float> u_;
    std::vector<float> v_;
    std::vector<float> next_u_;
    std::vector<float> next_v_;
};

} // namespace

std::unique_ptr<Simulator> make_cpu_simulator() {
    return std::make_unique<CpuSimulator>();
}

