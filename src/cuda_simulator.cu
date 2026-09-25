#include "gray_scott/simulation.hpp"
#include "gray_scott/initial_conditions.hpp"

#include <cuda_runtime.h>

#include <stdexcept>
#include <string>
#include <vector>

/*
   Gray-Scott models two chemicals, U and V, on a grid.

       dU/dt = Du * laplace(U) - U*V*V + F*(1-U)
       dV/dt = Dv * laplace(V) + U*V*V - (F+K)*V

   U and V diffuse to nearby cells, while U*V*V is the local reaction.
   F feeds U into the system and K removes V. The update below uses explicit
   Euler integration: new_value = old_value + timestep * derivative.
*/

namespace {

void check_cuda(cudaError_t result, const char* operation) {
    if (result != cudaSuccess) {
        throw std::runtime_error(std::string(operation) + ": " + cudaGetErrorString(result));
    }
}

__global__ void update_fields(
    const float* __restrict__ u,
    const float* __restrict__ v,
    float* next_u,
    float* next_v,
    int width,
    int height,
    float diffusion_u,
    float diffusion_v,
    float feed,
    float kill,
    float timestep) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) {
        return;
    }

    // Periodic boundaries wrap edges; branches are cheaper than four modulo operations per cell.
    const int left_x = x == 0 ? width - 1 : x - 1;
    const int right_x = x == width - 1 ? 0 : x + 1;
    const int top_y = y == 0 ? height - 1 : y - 1;
    const int bottom_y = y == height - 1 ? 0 : y + 1;
    const int center = y * width + x;
    const int left = y * width + left_x;
    const int right = y * width + right_x;
    const int top = top_y * width + x;
    const int bottom = bottom_y * width + x;
    const float current_u = u[center];
    const float current_v = v[center];
    const float laplace_u = u[left] + u[right] + u[top] + u[bottom] - 4.0f * current_u;
    const float laplace_v = v[left] + v[right] + v[top] + v[bottom] - 4.0f * current_v;
    const float reaction = current_u * current_v * current_v;

    next_u[center] = current_u + timestep *
        (diffusion_u * laplace_u - reaction + feed * (1.0f - current_u));
    next_v[center] = current_v + timestep *
        (diffusion_v * laplace_v + reaction - (feed + kill) * current_v);
}

class CudaSimulator final : public Simulator {
public:
    ~CudaSimulator() override {
        cudaFree(u_);
        cudaFree(v_);
        cudaFree(next_u_);
        cudaFree(next_v_);
    }

    void reset(const SimulationConfig& config) override {
        config_ = config;
        const std::size_t count = static_cast<std::size_t>(config.width) * config.height;
        cudaFree(u_);
        cudaFree(v_);
        cudaFree(next_u_);
        cudaFree(next_v_);
        check_cuda(cudaMalloc(&u_, count * sizeof(float)), "cudaMalloc U");
        check_cuda(cudaMalloc(&v_, count * sizeof(float)), "cudaMalloc V");
        check_cuda(cudaMalloc(&next_u_, count * sizeof(float)), "cudaMalloc next U");
        check_cuda(cudaMalloc(&next_v_, count * sizeof(float)), "cudaMalloc next V");

        std::vector<float> u(count, 1.0f);
        std::vector<float> v(count, 0.0f);
        // Build the initial field on the CPU and copy it once; the GPU handles every later step.
        initialize_fields(config, u, v);
        check_cuda(cudaMemcpy(u_, u.data(), count * sizeof(float), cudaMemcpyHostToDevice),
               "copy initial U");
        check_cuda(cudaMemcpy(v_, v.data(), count * sizeof(float), cudaMemcpyHostToDevice),
               "copy initial V");
    }

    void step() override {
        // One thread owns one cell; double buffers prevent reads from seeing partially updated neighbors.
        const dim3 block(16, 16);
        const dim3 grid((config_.width + block.x - 1) / block.x,
                        (config_.height + block.y - 1) / block.y);
        update_fields<<<grid, block>>>(u_, v_, next_u_, next_v_, config_.width, config_.height,
                                       config_.diffusion_u, config_.diffusion_v,
                                       config_.feed, config_.kill, config_.timestep);
        check_cuda(cudaGetLastError(), "launch update kernel");
        std::swap(u_, next_u_);
        std::swap(v_, next_v_);

    }

    void snapshot(Frame& frame) override {
        frame.width = config_.width;
        frame.height = config_.height;
        frame.value.resize(static_cast<std::size_t>(config_.width) * config_.height);
        check_cuda(cudaMemcpy(frame.value.data(), v_,
                              frame.value.size() * sizeof(float), cudaMemcpyDeviceToHost),
                   "copy frame to host");
    }

    void finish() override {
        check_cuda(cudaDeviceSynchronize(), "finish CUDA simulation");
    }

private:
    SimulationConfig config_;
    float* u_ = nullptr;
    float* v_ = nullptr;
    float* next_u_ = nullptr;
    float* next_v_ = nullptr;
};

} // namespace

std::unique_ptr<Simulator> make_cuda_simulator() {
    return std::make_unique<CudaSimulator>();
}
