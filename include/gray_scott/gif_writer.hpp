#pragma once

#include "gray_scott/simulation.hpp"

#include <memory>
#include <string>

class AnimationGifWriter {
public:
    AnimationGifWriter(const std::string& path, int width, int height, int frame_delay_cs);
    ~AnimationGifWriter();

    AnimationGifWriter(const AnimationGifWriter&) = delete;
    AnimationGifWriter& operator=(const AnimationGifWriter&) = delete;

    void add_frame(const Frame& frame, int step, bool include_step_number);
    void close();

private:
    struct State;
    std::unique_ptr<State> state_;
};
