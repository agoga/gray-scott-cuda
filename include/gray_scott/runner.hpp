#pragma once

#include "gray_scott/simulation.hpp"

#include <string>

// Run the configured simulation and optionally write sampled frames as a GIF.
void run_simulation(const SimulationConfig& config,
                    Backend backend,
                    const std::string& gif_path = {},
                    int fps = 10,
                    bool include_step_number = false);
