from ._gray_scott import Backend, Config, InitialCondition
from ._gray_scott import simulate as _simulate


def simulate(config=None, backend=Backend.cuda, gif_path=None, fps=10,
             include_step_number=False):
    """Run a simulation and optionally let C++ write a GIF."""
    if config is None:
        config = Config()
    _simulate(config, backend, gif_path or "", fps, include_step_number)


__all__ = ["Backend", "Config", "InitialCondition", "simulate"]
