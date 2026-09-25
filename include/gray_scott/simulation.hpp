#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

enum class Backend {
    Cpu,
    Cuda,
};

enum class InitialCondition {
    CenterSquare,
    TwoCircles,
    RandomSpots,
    ConcentricRings,
    RadialSpokes,
};

struct SimulationConfig {
    int width = 256;
    int height = 256;
    int steps = 100;
    int frame_interval = 10;
    float diffusion_u = 0.16f;
    float diffusion_v = 0.08f;
    // This feed/kill pair tends to produce active waves and spots.
    float feed = 0.022f;
    float kill = 0.051f;
    float timestep = 1.0f;
    int seed = 1;
    InitialCondition initial_condition = InitialCondition::ConcentricRings;
};

struct Frame {
    int width = 0;
    int height = 0;
    std::vector<float> value;
};

class Simulator {
public:
    virtual ~Simulator() = default;
    virtual void reset(const SimulationConfig& config) = 0;
    virtual void step() = 0;
    virtual void snapshot(Frame& frame) = 0;
    virtual void finish() = 0;
};

std::unique_ptr<Simulator> make_cpu_simulator();
std::unique_ptr<Simulator> make_cuda_simulator();
