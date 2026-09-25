#include "gray_scott/runner.hpp"

#include "gray_scott/gif_writer.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>

void run_simulation(const SimulationConfig& config,
                    Backend backend,
                    const std::string& gif_path,
                    int fps,
                    bool include_step_number) {
    if (config.width < 3 || config.height < 3 || config.steps < 1 ||
        config.frame_interval < 1 || fps < 1) {
        throw std::invalid_argument(
            "width, height, steps, frame_interval, and fps must be positive.");
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
    if (gif) {
        gif->close();
    }
}
