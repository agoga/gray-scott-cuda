#include "gray_scott/runner.hpp"

#include "gray_scott/gif_writer.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

/* Keeping this loop here prevents the CLI and Python binding from growing
    separate versions of the same simulation and GIF-writing workflow. */
void run_simulation(const SimulationConfig& config,
                    Backend backend,
                    const std::string& gif_path,
                    int fps,
                    bool include_step_number) {
    if (config.width < 3 || config.height < 3 || config.steps < 1 ||
        config.frame_interval < 1 || fps < 1) {
        throw std::invalid_argument(
            "width and height must be at least 3; steps, frame_interval, and fps must be positive.");
    }
    for (float value : {config.diffusion_u, config.diffusion_v, config.feed, config.kill}) {
        if (!std::isfinite(value) || value < 0.0f) {
            throw std::invalid_argument("Diffusion, feed, and kill must be finite and nonnegative.");
        }
    }
    if (!std::isfinite(config.timestep) || config.timestep <= 0.0f) {
        throw std::invalid_argument("timestep must be finite and positive.");
    }
    // The four-neighbor Euler diffusion update requires D * dt <= 1/4.
    // This checks diffusion only; the reaction can still become unstable.
    if (static_cast<double>(config.timestep) * std::max(config.diffusion_u, config.diffusion_v) > 0.25) {
        throw std::invalid_argument("timestep is too large for the diffusion rates.");
    }

    std::unique_ptr<Simulator> simulator = backend == Backend::Cuda
        ? make_cuda_simulator()
        : make_cpu_simulator();
    simulator->reset(config);

    std::unique_ptr<AnimationGifWriter> gif;
    if (!gif_path.empty()) {
        gif = std::make_unique<AnimationGifWriter>(gif_path, config.width, config.height,
                                                   std::max(1, 100 / fps));
    }

    Frame frame;
    for (int step = 1; step <= config.steps; ++step) {
        simulator->step();
        if (gif && (step % config.frame_interval == 0 || step == config.steps)) {
            simulator->snapshot(frame);
            gif->add_frame(frame, step, include_step_number);
        }
    }

    // A run without GIF output still needs one final synchronization so async
    // CUDA errors are reported instead of being hidden at process exit.
    simulator->finish();
}
