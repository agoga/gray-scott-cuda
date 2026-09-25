#pragma once

#include "gray_scott/simulation.hpp"

#include <vector>

// Fill matching U and V fields for the selected starting pattern.
void initialize_fields(const SimulationConfig& config,
                       std::vector<float>& u,
                       std::vector<float>& v);
