// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#include <librealsense/rs2.hpp>
#include <librealsense/rsutil2.hpp>
#include "example.hpp"

#include <sstream>
#include <iostream>

using namespace rs2;
using namespace std;

int main(int argc, char * argv[]) try
{
    log_to_console(RS2_LOG_SEVERITY_INFO);
    //log_to_file(log_severity::debug, "librealsense.log");

    context ctx;

    auto list = ctx.query_devices();
    if (list.size() == 0)
        throw runtime_error("No device detected. Is it plugged in?");

    auto dev = list[0];

    // Configure all supported streams to run at 30 frames per second
    util::config config;
    config.enable_all(rs2::preset::best_quality);
    auto stream = config.open(dev);
    auto syncer = dev.create_syncer();
    stream.start(syncer);

    texture_buffer buffers[RS2_STREAM_COUNT];

    // Open a GLFW window
    glfwInit();
    ostringstream ss;
    ss << "CPP Capture Example (" << dev.get_camera_info(RS2_CAMERA_INFO_DEVICE_NAME) << ")";

    auto win = glfwCreateWindow(1280, 720, ss.str().c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(win);

    while (!glfwWindowShouldClose(win))
    {
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);

        auto index = 0;
        auto frames = syncer.wait_for_frames();
        // for consistent visualization, sort frames based on stream type:
        sort(frames.begin(), frames.end(),
            [](const frame& a, const frame& b) -> bool
        {
            return a.get_stream_type() < b.get_stream_type();
        });
        auto tiles_horisontal = static_cast<int>(ceil(sqrt(frames.size())));
        auto tiles_vertical = ceil((float)frames.size() / tiles_horisontal);
        auto tile_w = static_cast<float>((float)w / tiles_horisontal);
        auto tile_h = static_cast<float>((float)h / tiles_vertical);

        for (auto&& frame : frames)
        {
            auto stream_type = frame.get_stream_type();
            buffers[stream_type].upload(frame);
        }

        // Wait for new images
        glfwPollEvents();

        // Clear the framebuffer
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the images
        glPushMatrix();
        glfwGetWindowSize(win, &w, &h);
        glOrtho(0, w, h, 0, -1, +1);

        index = 0;
        for (auto&& frame : frames)
        {
            auto stream_type = frame.get_stream_type();
            auto col_id = index / tiles_horisontal;
            auto row_id = index % tiles_horisontal;

            buffers[stream_type].show({ row_id * tile_w, static_cast<float>(col_id * tile_h), tile_w, tile_h }, 1);

            index++;
        }

        glPopMatrix();
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
catch (const error & e)
{
    cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << endl;
    return EXIT_FAILURE;
}
catch (const exception & e)
{
    cerr << e.what() << endl;
    return EXIT_FAILURE;
}
