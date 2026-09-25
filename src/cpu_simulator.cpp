#include "gray_scott/simulation.hpp"
#include "gray_scott/initial_conditions.hpp"

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
        const int width = config_.width;
        const int height = config_.height;
        for (int y = 0; y < config_.height; ++y) {
            const std::size_t row = static_cast<std::size_t>(y) * width;
            const std::size_t top_row = static_cast<std::size_t>(y == 0 ? height - 1 : y - 1) * width;
            const std::size_t bottom_row = static_cast<std::size_t>(y == height - 1 ? 0 : y + 1) * width;
            for (int x = 0; x < config_.width; ++x) {
                const std::size_t index = row + x;
                const std::size_t left = row + (x == 0 ? width - 1 : x - 1);
                const std::size_t right = row + (x == width - 1 ? 0 : x + 1);
                const std::size_t top = top_row + x;
                const std::size_t bottom = bottom_row + x;
                const float u = u_[index];
                const float v = v_[index];
                const float laplace_u = u_[left] + u_[right] + u_[top] + u_[bottom] - 4.0f * u;
                const float laplace_v = v_[left] + v_[right] + v_[top] + v_[bottom] - 4.0f * v;
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
        frame.value = v_;
    }

    void finish() override {
    }

private:
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

