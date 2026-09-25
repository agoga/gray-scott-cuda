#include "gray_scott/runner.hpp"

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace {

/* pybind11 maps this thin wrapper to Python. The actual run is shared with
    the CLI, and releasing the GIL lets other Python threads run while CUDA works. */
void simulate(const SimulationConfig& config, Backend backend, const std::string& gif_path,
                  int fps, bool include_step_number) {
     run_simulation(config, backend, gif_path, fps, include_step_number);
}

} // namespace

PYBIND11_MODULE(_gray_scott, module) {
    module.doc() = "CUDA and CPU Gray-Scott reaction-diffusion simulation.";

    py::enum_<Backend>(module, "Backend")
        .value("cpu", Backend::Cpu)
        .value("cuda", Backend::Cuda);

    py::enum_<InitialCondition>(module, "InitialCondition")
        .value("center_square", InitialCondition::CenterSquare)
        .value("two_circles", InitialCondition::TwoCircles)
        .value("random_spots", InitialCondition::RandomSpots)
        .value("concentric_rings", InitialCondition::ConcentricRings)
        .value("radial_spokes", InitialCondition::RadialSpokes);

    py::class_<SimulationConfig>(module, "Config")
        .def(py::init<>())
        .def_readwrite("width", &SimulationConfig::width)
        .def_readwrite("height", &SimulationConfig::height)
        .def_readwrite("steps", &SimulationConfig::steps)
        .def_readwrite("frame_interval", &SimulationConfig::frame_interval)
        .def_readwrite("diffusion_u", &SimulationConfig::diffusion_u)
        .def_readwrite("diffusion_v", &SimulationConfig::diffusion_v)
        .def_readwrite("feed", &SimulationConfig::feed)
        .def_readwrite("kill", &SimulationConfig::kill)
        .def_readwrite("timestep", &SimulationConfig::timestep)
        .def_readwrite("seed", &SimulationConfig::seed)
        .def_readwrite("initial_condition", &SimulationConfig::initial_condition);

    module.def("simulate", &simulate,
               py::arg("config"),
               py::arg("backend") = Backend::Cuda,
               py::arg("gif_path") = "",
               py::arg("fps") = 10,
               py::arg("include_step_number") = false,
               py::call_guard<py::gil_scoped_release>());
}
