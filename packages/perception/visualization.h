#pragma once

#define GLM_FORCE_RADIANS

#include <memory>
#include <string>

#include "Eigen/Eigen"
#include "GL/glew.h"
#include "GL/glew.h"
#include "GL/glu.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "glog/logging.h"
#include "sophus/se3.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "packages/perception/grid.h"

#define checkGLErrorState()                                                                                                                \
    {                                                                                                                                      \
        auto error = glGetError();                                                                                                         \
        if (error != GL_NO_ERROR) {                                                                                                        \
            LOG(FATAL) << error;                                                                                                           \
        }                                                                                                                                  \
    }

namespace perception {
namespace visualization {

    GLuint loadShaders(const std::string& vertexFilePath, const std::string& fragmentFilePath);

    struct gl_point_t {
        float x, y, z;
    };

    struct gl_color_t {
        GLfloat r, g, b, a;
    };

    static constexpr int kTotalLineSegmentsPerCube = 24;

    typedef struct {
        float x, y, z;
        std::vector<gl_point_t> m_lines;
    } gl_cube_t;

    inline void createCube(gl_cube_t* cube, float centre_x, float centre_y, float centre_z, float w, float h, float l) {
        CHECK_NOTNULL(cube);
        cube->m_lines.clear();
        cube->m_lines.reserve(kTotalLineSegmentsPerCube);
        cube->m_lines.push_back(gl_point_t{ -w, -h, -l });
        cube->m_lines.push_back(gl_point_t{ -w, h, -l });
        cube->m_lines.push_back(gl_point_t{ -w, h, -l });
        cube->m_lines.push_back(gl_point_t{ w, h, -l });
        cube->m_lines.push_back(gl_point_t{ w, h, -l });
        cube->m_lines.push_back(gl_point_t{ w, -h, -l });
        cube->m_lines.push_back(gl_point_t{ w, -h, -l });
        cube->m_lines.push_back(gl_point_t{ -w, -h, -l });
        // Front
        cube->m_lines.push_back(gl_point_t{ -w, -h, l });
        cube->m_lines.push_back(gl_point_t{ -w, h, l });
        cube->m_lines.push_back(gl_point_t{ -w, h, l });
        cube->m_lines.push_back(gl_point_t{ w, h, l });
        cube->m_lines.push_back(gl_point_t{ w, h, l });
        cube->m_lines.push_back(gl_point_t{ w, -h, l });
        cube->m_lines.push_back(gl_point_t{ w, -h, l });
        cube->m_lines.push_back(gl_point_t{ -w, -h, l });
        // Sides
        cube->m_lines.push_back(gl_point_t{ -w, -h, -l });
        cube->m_lines.push_back(gl_point_t{ -w, -h, l });
        cube->m_lines.push_back(gl_point_t{ -w, h, -l });
        cube->m_lines.push_back(gl_point_t{ -w, h, l });
        cube->m_lines.push_back(gl_point_t{ w, h, -l });
        cube->m_lines.push_back(gl_point_t{ w, h, l });
        cube->m_lines.push_back(gl_point_t{ w, -h, -l });
        cube->m_lines.push_back(gl_point_t{ w, -h, l });
        CHECK(kTotalLineSegmentsPerCube == cube->m_lines.size());
        for (auto& entry : cube->m_lines) {
            entry.x += centre_x;
            entry.y += centre_y;
            entry.z += centre_z;
        }
    }

    class Primitive {
    public:
        Primitive(const std::string& vertex, const std::string& fragment)
            : m_init(false) {
            m_programID = perception::visualization::loadShaders(vertex, fragment);
            CHECK(m_programID > 0);
            checkGLErrorState();
            m_transform << 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0;
        }

        virtual void draw() = 0;

        virtual void update(const Sophus::SE3d& transform) { m_transform = transform.matrix().cast<float>(); }

        virtual void update(const glm::mat4& MVP) {
            glUseProgram(m_programID);
            checkGLErrorState();
            Eigen::Matrix4f T = Eigen::Map<const Eigen::Matrix4f>(glm::value_ptr(MVP)) * m_transform;
            glUniformMatrix4fv(glGetUniformLocation(m_programID, "MVP"), 1, GL_FALSE, T.data());
            checkGLErrorState();
            glUseProgram(0);
        }

    protected:
        bool m_init;
        GLuint m_VBO;
        GLuint m_programID;
        Eigen::Matrix4f m_transform;
    };

    class CoordinateSystem : public Primitive {
    public:
        CoordinateSystem(float scale = 1.0)
            : Primitive("packages/perception/bin/shaders/identity.vertexshader", "packages/perception/bin/shaders/identity.fragmentshader")
            , m_scale(scale) {

            constexpr float kDefaultOpacity = 0.75;

            gl_point_t origin;
            origin.x = 0.0f;
            origin.y = 0.0f;
            origin.z = 0.0f;

            gl_point_t unit_x;
            unit_x.x = m_scale;
            unit_x.y = 0.0f;
            unit_x.z = 0.0f;
            m_vertices.push_back(origin);
            m_colors.push_back({ 1.0, 0.0, 0.0, kDefaultOpacity });
            m_vertices.push_back(unit_x);
            m_colors.push_back({ 1.0, 0.0, 0.0, kDefaultOpacity });

            gl_point_t unit_y;
            unit_y.x = 0.0f;
            unit_y.y = m_scale;
            unit_y.z = 0.0f;
            m_vertices.push_back(origin);
            m_colors.push_back({ 0.0, 1.0, 0.0, kDefaultOpacity });
            m_vertices.push_back(unit_y);
            m_colors.push_back({ 0.0, 1.0, 0.0, kDefaultOpacity });

            gl_point_t unit_z;
            unit_z.x = 0.0f;
            unit_z.y = 0.0f;
            unit_z.z = m_scale;
            m_vertices.push_back(origin);
            m_colors.push_back({ 0.0, 0.0, 1.0, kDefaultOpacity });
            m_vertices.push_back(unit_z);
            m_colors.push_back({ 0.0, 0.0, 1.0, kDefaultOpacity });
        }

        void draw() {
            glUseProgram(m_programID);
            glEnable(GL_BLEND);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_MULTISAMPLE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkGLErrorState();
            glEnableClientState(GL_VERTEX_ARRAY);
            CHECK(m_colors.size() == m_vertices.size());
            glVertexPointer(3, GL_FLOAT, sizeof(gl_point_t), m_vertices.data());
            checkGLErrorState();
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4, GL_FLOAT, sizeof(gl_color_t), m_colors.data());
            checkGLErrorState();
            glDrawArrays(GL_LINES, 0, m_vertices.size());
            checkGLErrorState();
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisable(GL_MULTISAMPLE);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_BLEND);
            glUseProgram(0);
        }

    private:
        float m_scale;
        std::vector<gl_point_t> m_vertices;
        std::vector<gl_color_t> m_colors;
    };

    class Grid : public Primitive {
    public:
        Grid()
            : Primitive("packages/perception/bin/shaders/identity.vertexshader", "packages/perception/bin/shaders/grid.fragmentshader") {
            const auto limit = 2 * 5.0;

            gl_point_t vertex;
            for (int i = -10; i <= 10; i++) {
                vertex.x = i;
                vertex.y = -limit;
                vertex.z = 0.0;
                m_vertices.push_back(vertex);
                vertex.x = i;
                vertex.y = limit;
                vertex.z = 0.0f;
                m_vertices.push_back(vertex);
            }
            for (int j = -10; j <= 10; j++) {
                vertex.x = -limit;
                vertex.y = j;
                vertex.z = 0.0f;
                m_vertices.push_back(vertex);
                vertex.x = limit;
                vertex.y = j;
                vertex.z = 0.0f;
                m_vertices.push_back(vertex);
            }
        }

        void draw() {
            glUseProgram(m_programID);
            checkGLErrorState();
            glEnableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glLineWidth(1.0);
            glVertexPointer(3, GL_FLOAT, 0, m_vertices.data());
            checkGLErrorState();
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDrawArrays(GL_LINES, 0, m_vertices.size());
            checkGLErrorState();
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
            glDisableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
            glUseProgram(0);
        }

    private:
        std::vector<gl_point_t> m_vertices;
    };

    class PointCloud : public Primitive {
    public:
        PointCloud()
            : Primitive("packages/perception/bin/shaders/points.vertexshader", "packages/perception/bin/shaders/points.fragmentshader") {}

        void initialise() {
            glGenBuffers(1, &m_VBO);
            checkGLErrorState();
            m_init = true;
        }

        void update(const perception::PointCloudXYZ& cloud) {
            if (!m_init) {
                return;
            }
            if (cloud.xyz.cols() == 0) {
                return;
            }
            CHECK(3 == cloud.xyz.rows()) << cloud.xyz.rows() << " " << cloud.xyz.cols();
            m_points.clear();
            m_points.reserve(cloud.xyz.cols());
            for (int i = 0; i < cloud.xyz.cols(); ++i) {
                m_points.push_back({ cloud.xyz(0, i), cloud.xyz(1, i), cloud.xyz(2, i) });
            }
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(gl_point_t) * m_points.size(), m_points.data(), GL_DYNAMIC_DRAW);
            checkGLErrorState();
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        void drawPoints() {
            glUseProgram(m_programID);
            checkGLErrorState();

            glEnable(GL_BLEND);
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_POINT_SMOOTH);
            glEnable(GL_PROGRAM_POINT_SIZE);
            glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

            glColor4f(0.0, 1.0, 0.0, 0.4);
            glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            checkGLErrorState();
            glEnableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glVertexPointer(3, GL_FLOAT, sizeof(gl_point_t), (void*)(sizeof(float) * 0));
            checkGLErrorState();
            glDrawArrays(GL_POINTS, 0, m_points.size());
            checkGLErrorState();
            glDisableClientState(GL_VERTEX_ARRAY);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            checkGLErrorState();

            glDisable(GL_MULTISAMPLE);
            glDisable(GL_BLEND);
            glDisable(GL_POINT_SMOOTH);
            glDisable(GL_PROGRAM_POINT_SIZE);

            glUseProgram(0);
        }

        void draw() {
            if (!m_init) {
                initialise();
            }
            drawPoints();
        }

    private:
        std::vector<gl_point_t> m_points;
    };

    class Voxels : public Primitive {
    public:
        Voxels()
            : Primitive(
                  "packages/perception/bin/shaders/identity.vertexshader", "packages/perception/bin/shaders/identity.fragmentshader") {
            m_init = false;
            checkGLErrorState();
        }

        void update(const Sophus::SE3d& transform) override { Primitive::update(transform); }

        void update(const perception::VoxelGridProto& grid) {
            m_voxelGridOptions = grid.options();
            auto res_x = m_voxelGridOptions.grid_options().res_x();
            auto res_y = m_voxelGridOptions.grid_options().res_y();
            auto res_z = m_voxelGridOptions.grid_options().res_z();
            if (res_x <= 0 || res_y <= 0 || res_z <= 0) {
                return;
            }
            m_init = true;

            m_voxelOutlines.clear();

            std::vector<perception::Voxel> filledVoxels;
            deserialize(grid, filledVoxels);

            for (const auto voxel : filledVoxels) {
                if (voxel.valid) {
                    gl_cube_t cube;
                    createCube(&cube, voxel.centre[0], voxel.centre[1], voxel.centre[2], res_x / 2, res_y / 2, res_z / 2);
                    float angle = std::numeric_limits<float>::quiet_NaN();
                    perception::angleToVertical(voxel, angle);
                    constexpr float kDefaultVerticalTolerance = 0.1f;
                    bool traversible = (std::abs(angle) < kDefaultVerticalTolerance);
                    m_voxelOutlines.emplace_back(std::make_pair(traversible, cube));
                }
            }
            checkGLErrorState();
        }

        void drawOutlines() {
            if (!m_init) {
                return;
            }
            auto x = m_voxelGridOptions.grid_options().dim_x() / 2;
            auto y = m_voxelGridOptions.grid_options().dim_y() / 2;
            auto z = m_voxelGridOptions.grid_options().dim_z() / 2;

            gl_cube_t outline;
            createCube(&outline, 0, 0, 0, x, y, z);

            glEnable(GL_BLEND);
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glEnableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glVertexPointer(3, GL_FLOAT, sizeof(gl_point_t), outline.m_lines.data());
            checkGLErrorState();
            glDrawArrays(GL_LINES, 0, outline.m_lines.size());
            checkGLErrorState();
            glDisableClientState(GL_VERTEX_ARRAY);

            glEnableClientState(GL_VERTEX_ARRAY);
            for (const auto& voxel : m_voxelOutlines) {
                const auto& traversible = std::get<0>(voxel);
                const auto& cube = std::get<1>(voxel);

                glVertexPointer(3, GL_FLOAT, sizeof(gl_point_t), cube.m_lines.data());
                checkGLErrorState();
                if (traversible) {
                    glColor4f(0.0f, 1.0f, 0.0f, 0.4f);
                } else {
                    glColor4f(0.5f, 0.5f, 0.5f, 0.2f);
                }
                checkGLErrorState();
                glDrawArrays(GL_LINES, 0, cube.m_lines.size());
                checkGLErrorState();
            }
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
        }

        void draw() {
            glUseProgram(m_programID);
            drawOutlines();
            glUseProgram(0);
        }

    private:
        VoxelGridOptions m_voxelGridOptions;
        std::vector<std::pair<bool, gl_cube_t> > m_voxelOutlines;
    };

    class Controls {
    public:
        virtual void update(const SDL_Event& event) = 0;
        virtual glm::mat4 getViewMatrix() = 0;
        virtual glm::mat4 getProjectionMatrix() = 0;
    };

    class FPSControls : public Controls {
    public:
        FPSControls();

        void update(const SDL_Event& event) override;
        glm::mat4 getViewMatrix() override;
        glm::mat4 getProjectionMatrix() override;

    private:
        class FPSControlsImpl;
        std::shared_ptr<FPSControlsImpl> m_impl;
    };

} // visualization
} // perception
