#pragma once

#include "gray_scott/simulation.hpp"

#include <string>

/* The backends only know how to reset, step, and provide a frame. This runner
    owns one complete run and is shared by both the CLI and Python binding. */
void run_simulation(const SimulationConfig& config,
                    Backend backend,
                    const std::string& gif_path = {},
                    int fps = 10,
                    bool include_step_number = false);
