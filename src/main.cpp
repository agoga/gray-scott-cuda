#include "gray_scott/runner.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

/* This executable is a native check of the same library used by Python. It is
    useful when debugging the CUDA build without a Python interpreter. */

struct CommandLine {
    SimulationConfig config;
    Backend backend = Backend::Cuda;
    std::string output_path;
};

void print_usage() {
    std::cout << "gray_scott_cli [options]\n"
              << "  --cpu | --cuda\n"
              << "  --width N --height N --steps N --frame-interval N\n"
              << "  --feed F --kill K\n"
              << "  --condition center-square|two-circles|random-spots|rings|spokes\n"
              << "  --gif PATH\n";
}

InitialCondition read_condition(const std::string& value) {
    if (value == "center-square") return InitialCondition::CenterSquare;
    if (value == "two-circles") return InitialCondition::TwoCircles;
    if (value == "random-spots") return InitialCondition::RandomSpots;
    if (value == "rings") return InitialCondition::ConcentricRings;
    if (value == "spokes") return InitialCondition::RadialSpokes;
    throw std::invalid_argument("Unknown initial condition: " + value);
}

CommandLine parse_arguments(int argc, char** argv) {
    CommandLine command_line;
    for (int index = 1; index < argc; ++index) {
        const std::string option = argv[index];
        if (option == "--help") {
            print_usage();
            std::exit(0);
        } else if (option == "--cpu") {
            command_line.backend = Backend::Cpu;
            continue;
        } else if (option == "--cuda") {
            command_line.backend = Backend::Cuda;
            continue;
        }
        if (index + 1 >= argc) {
            throw std::invalid_argument("Missing value for " + option);
        }
        if (option == "--gif") {
            command_line.output_path = argv[++index];
        } else if (option == "--condition") {
            command_line.config.initial_condition = read_condition(argv[++index]);
        } else if (option == "--width") {
            command_line.config.width = std::stoi(argv[++index]);
        } else if (option == "--height") {
            command_line.config.height = std::stoi(argv[++index]);
        } else if (option == "--steps") {
            command_line.config.steps = std::stoi(argv[++index]);
        } else if (option == "--frame-interval") {
            command_line.config.frame_interval = std::stoi(argv[++index]);
        } else if (option == "--feed") {
            command_line.config.feed = std::stof(argv[++index]);
        } else if (option == "--kill") {
            command_line.config.kill = std::stof(argv[++index]);
        } else {
            throw std::invalid_argument("Unknown option: " + option);
        }
    }
    return command_line;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const CommandLine command_line = parse_arguments(argc, argv);
        run_simulation(command_line.config, command_line.backend,
                       command_line.output_path, 10, false);
        std::cout << "Simulation complete using "
                  << (command_line.backend == Backend::Cuda ? "CUDA" : "CPU") << ".\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
