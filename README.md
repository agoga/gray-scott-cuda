# Gray-Scott CUDA

A Gray-Scott reaction-diffusion simulator written in C++ and CUDA, with a small Python interface.


Gray-Scott is a reaction-diffusion model for two interacting chemicals, usually called `U` and `V`. Each chemical spreads through the grid, while the reaction changes their local concentrations.

The model used here is:

```text
∂U/∂t = Du * laplace(U) - U*V² + F*(1-U)
∂V/∂t = Dv * laplace(V) + U*V² - (F+K)*V
```

`Du` and `Dv` control diffusion. `F` is the feed rate and `K` is the kill rate. Small changes to `F` and `K` can produce spots, waves, rings, stripes, and other patterns.

The simulation uses an explicit Euler update on a 2D grid with wrapping boundaries. Each cell reads its four neighbors, calculates the reaction and diffusion terms, and writes the next state to a separate buffer.

## Project

This is mainly a CUDA practice project. The CPU implementation provides a simple comparison point, while the CUDA implementation updates one grid cell per GPU thread. Python is used to set parameters and start runs. C++ handles the simulation and GIF encoding.


A very basic result is shown here:

![Gray-Scott concentric-ring result](gray_scott_python.gif)

The GIF was generated using concentric rings as the initial condition.

## Requirements

- Windows
- NVIDIA GPU with CUDA support
- CUDA Toolkit 13.4
- Visual Studio 2022 Build Tools with MSVC
- CMake 3.25 or newer
- Anaconda Python 3.11

CMake downloads `pybind11` and `gif-h` during configuration. The Python notebook environment is listed in `environment.yml`.

## Build

Create the Python environment if needed:

```powershell
conda env create -f environment.yml
conda activate gray-scott
```

Configure and build from a Visual Studio developer environment or a shell where MSVC is available:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --parallel 4
```

The build produces the native test program and Python extension here:

```text
build/Debug/gray_scott_cli.exe
build/python/gray_scott/_gray_scott.cp311-win_amd64.pyd
```

Run the native test program:

```powershell
.\build\Debug\gray_scott_cli.exe --cuda --width 512 --height 512 --steps 100
```

## Python

Point Python at the build output:

```powershell
$env:PYTHONPATH = "$PWD\build\python"
```

Run a simulation without saving output:

```python
import gray_scott

config = gray_scott.Config()
config.width = 512
config.height = 512
config.steps = 1000
config.frame_interval = 20
config.initial_condition = gray_scott.InitialCondition.concentric_rings

gray_scott.simulate(config, backend=gray_scott.Backend.cuda)
```

Write a GIF from C++:

```python
gray_scott.simulate(
    config,
    backend=gray_scott.Backend.cuda,
    gif_path="pattern.gif",
    fps=10,
    include_step_number=True,
)
```

Available initial conditions are `center_square`, `two_circles`, `random_spots`, `concentric_rings`, and `radial_spokes`.

## Notebook

Open `gray_scott_experiments.ipynb` after building and select the Python 3.11 environment from `environment.yml`.

## License

This project is licensed under CC BY 4.0. See [LICENSE](LICENSE).
