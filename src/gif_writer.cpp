#include "gray_scott/gif_writer.hpp"

#define GIF_H_IMPLEMENTATION
#include "gif.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

/* The simulator produces normalized V values in the range 0..1. This class
    converts those values to grayscale RGBA pixels and hands each sampled frame
    to the GIF encoder. It is deliberately separate from the simulation math. */
struct AnimationGifWriter::State {
    ::GifWriter gif{};
    int width = 0;
    int height = 0;
    int delay_cs = 1;
    std::vector<uint8_t> pixels;
    bool closed = false;
};

AnimationGifWriter::AnimationGifWriter(const std::string& path, int width, int height, int frame_delay_cs)
    : state_(std::make_unique<State>()) {
    state_->width = width;
    state_->height = height;
    state_->delay_cs = std::max(1, frame_delay_cs);
    state_->pixels.resize(static_cast<std::size_t>(width) * height * 4);
    if (!GifBegin(&state_->gif, path.c_str(), width, height, state_->delay_cs)) {
        state_.reset();
        throw std::runtime_error("Could not create the GIF file.");
    }
}

AnimationGifWriter::~AnimationGifWriter() {
    close();
}

void AnimationGifWriter::add_frame(const Frame& frame, int step, bool include_step_number) {
    if (state_ == nullptr || state_->closed) {
        throw std::runtime_error("Cannot add a frame to a closed GIF.");
    }
    if (frame.width != state_->width || frame.height != state_->height) {
        throw std::invalid_argument("GIF frame dimensions do not match the output.");
    }

    for (std::size_t i = 0; i < frame.value.size(); ++i) {
        const auto brightness = static_cast<uint8_t>(
            std::clamp(frame.value[i], 0.0f, 1.0f) * 255.0f);
        state_->pixels[i * 4 + 0] = brightness;
        state_->pixels[i * 4 + 1] = brightness;
        state_->pixels[i * 4 + 2] = brightness;
        state_->pixels[i * 4 + 3] = 255;
    }

    if (include_step_number) {
        /* A tiny built-in digit font avoids pulling a text-rendering system into
           the simulation just to label saved frames. */
        const uint8_t digits[10][5] = {
            {0b111, 0b101, 0b101, 0b101, 0b111},
            {0b010, 0b110, 0b010, 0b010, 0b111},
            {0b111, 0b001, 0b111, 0b100, 0b111},
            {0b111, 0b001, 0b111, 0b001, 0b111},
            {0b101, 0b101, 0b111, 0b001, 0b001},
            {0b111, 0b100, 0b111, 0b001, 0b111},
            {0b111, 0b100, 0b111, 0b101, 0b111},
            {0b111, 0b001, 0b010, 0b010, 0b010},
            {0b111, 0b101, 0b111, 0b101, 0b111},
            {0b111, 0b101, 0b111, 0b001, 0b111},
        };
        const std::string number = std::to_string(std::max(0, step));
        const int scale = std::max(1, std::min(state_->width, state_->height) / 256);
        const int digit_width = 3 * scale;
        const int gap = scale;
        const int box_width = 6 + static_cast<int>(number.size()) * (digit_width + gap);
        const int box_height = 6 + 5 * scale;

        for (int y = 2; y < std::min(state_->height, box_height); ++y) {
            for (int x = 2; x < std::min(state_->width, box_width); ++x) {
                const std::size_t pixel = (static_cast<std::size_t>(y) * state_->width + x) * 4;
                state_->pixels[pixel + 0] = 0;
                state_->pixels[pixel + 1] = 0;
                state_->pixels[pixel + 2] = 0;
            }
        }

        int x_origin = 5;
        for (const char character : number) {
            const auto& glyph = digits[character - '0'];
            for (int row = 0; row < 5; ++row) {
                for (int column = 0; column < 3; ++column) {
                    if ((glyph[row] & (1 << (2 - column))) == 0) {
                        continue;
                    }
                    for (int dy = 0; dy < scale; ++dy) {
                        for (int dx = 0; dx < scale; ++dx) {
                            const int x = x_origin + column * scale + dx;
                            const int y = 5 + row * scale + dy;
                            if (x >= state_->width || y >= state_->height) {
                                continue;
                            }
                            const std::size_t pixel = (static_cast<std::size_t>(y) * state_->width + x) * 4;
                            state_->pixels[pixel + 0] = 255;
                            state_->pixels[pixel + 1] = 255;
                            state_->pixels[pixel + 2] = 255;
                        }
                    }
                }
            }
            x_origin += digit_width + gap;
        }
    }

    GifWriteFrame(&state_->gif, state_->pixels.data(), state_->width,
                  state_->height, state_->delay_cs);
}

void AnimationGifWriter::close() {
    if (state_ == nullptr) {
        return;
    }
    if (!state_->closed) {
        GifEnd(&state_->gif);
        state_->closed = true;
    }
    state_.reset();
}
