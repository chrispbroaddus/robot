#pragma once

#include "packages/perception/visualization.h"
#include "packages/planning/utils.h"

#include "sophus/se3.hpp"

namespace estimation {
namespace visualization {
    using perception::visualization::CoordinateSystem;

    class PoseVisualizer : public perception::visualization::Primitive {
    public:
        PoseVisualizer()
            : Primitive(
                  "packages/perception/bin/shaders/identity.vertexshader", "packages/perception/bin/shaders/identity.fragmentshader") {
            Sophus::SE3d opengl;
            Sophus::SE3d axesConversion;
            planning::zippyToOpenGL(opengl, axesConversion);
            m_axesConversion = axesConversion.matrix();
        }

        void updateStates(const std::deque<estimation::State>& states) { m_states = states; }

        void update(const glm::mat4& MVP) override {
            Primitive::update(MVP);
            m_MVP = MVP;
        }

        void draw() override {
            for (const auto& state : m_states) {
                CoordinateSystem c;
                auto transform = Eigen::Map<Eigen::Matrix4f>(glm::value_ptr(m_MVP)).cast<double>();
                Eigen::Matrix4d result = transform * m_axesConversion * state.m_pose.matrix();
                c.update(glm::make_mat4(result.data()));
                c.draw();
            }
        }

    private:
        Eigen::Matrix4d m_axesConversion;
        std::deque<estimation::State> m_states;
        glm::mat4 m_MVP;
    };

} // visualization
} // estimation
