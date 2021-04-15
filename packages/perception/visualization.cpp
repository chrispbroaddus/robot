#include "packages/perception/visualization.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "SDL.h"
#include "glog/logging.h"

namespace perception {
namespace visualization {

    static constexpr float kSpeedScaling = 10.0;
    static constexpr float kSDLTicksToSeconds = 1000.0;

    bool readShader(const std::string& filename, std::string& shader) {
        shader.clear();
        std::ifstream shaderStream(filename);
        if (!shaderStream.good()) {
            LOG(INFO) << filename << " is not readable";
            return false;
        }
        std::string line = "";
        while (std::getline(shaderStream, line)) {
            shader += ("\n" + line);
        }
        shaderStream.close();
        return true;
    }

    GLuint loadShaders(const std::string& vertexFilePath, const std::string& fragmentFilePath) {
        GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

        std::string vertexShaderCode;
        bool addVertexShader = !vertexFilePath.empty();
        std::string fragmentShaderCode;
        bool addFragmentShader = !fragmentFilePath.empty();

        GLint result = GL_FALSE;
        int logLength = 0;

        // Vertex Shader
        if (addVertexShader) {
            CHECK(readShader(vertexFilePath, vertexShaderCode));
            char const* vertexSourcePointer = vertexShaderCode.c_str();
            glShaderSource(vertexShaderId, 1, &vertexSourcePointer, NULL);
            glCompileShader(vertexShaderId);
            glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &result);
            glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                std::vector<char> vertexShaderErrorMessage(logLength + 1);
                glGetShaderInfoLog(vertexShaderId, logLength, NULL, &vertexShaderErrorMessage[0]);
                LOG(FATAL) << &vertexShaderErrorMessage[0];
            }
            checkGLErrorState();
        }

        // Compile Fragment Shader
        if (addFragmentShader) {
            CHECK(readShader(fragmentFilePath, fragmentShaderCode));
            char const* fragmentSourcePointer = fragmentShaderCode.c_str();
            glShaderSource(fragmentShaderId, 1, &fragmentSourcePointer, NULL);
            glCompileShader(fragmentShaderId);
            glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &result);
            glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                std::vector<char> fragmentShaderErrorMessage(logLength + 1);
                glGetShaderInfoLog(fragmentShaderId, logLength, NULL, &fragmentShaderErrorMessage[0]);
                LOG(FATAL) << &fragmentShaderErrorMessage[0];
            }
            checkGLErrorState();
        }

        // Link the program
        GLuint ProgramID = glCreateProgram();
        if (addVertexShader) {
            glAttachShader(ProgramID, vertexShaderId);
            checkGLErrorState();
        }
        if (addFragmentShader) {
            glAttachShader(ProgramID, fragmentShaderId);
            checkGLErrorState();
        }
        glLinkProgram(ProgramID);

        // Check the program
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::vector<char> programErrorMessage(logLength + 1);
            glGetProgramInfoLog(ProgramID, logLength, NULL, &programErrorMessage[0]);
            LOG(FATAL) << &programErrorMessage[0];
        }

        glDetachShader(ProgramID, vertexShaderId);
        glDetachShader(ProgramID, fragmentShaderId);
        glDeleteShader(vertexShaderId);
        glDeleteShader(fragmentShaderId);
        return ProgramID;
    }
    class FPSControls::FPSControlsImpl {
    public:
        typedef enum { NONE = 0, FORWARD, BACKWARD, LEFT, RIGHT } motion_t;

        FPSControlsImpl()
            : m_init(false) {}

        glm::mat4 getViewMatrix() { return m_viewMatrix; }
        glm::mat4 getProjectionMatrix() { return m_projectionMatrix; }

        motion_t handleMotion(const SDL_Event& event) {
            std::string name;
            switch (event.type) {
            case SDL_KEYDOWN:
                name = std::string(SDL_GetKeyName(event.key.keysym.sym));
                if (name == "W") {
                    return FPSControlsImpl::FORWARD;
                }
                if (name == "S") {
                    return FPSControlsImpl::BACKWARD;
                }
                if (name == "A") {
                    return FPSControlsImpl::LEFT;
                }
                if (name == "D") {
                    return FPSControlsImpl::RIGHT;
                }
            default:
                break;
            }
            return FPSControlsImpl::NONE;
        }

        void update(const SDL_Event& event) {

            static double lastTime = SDL_GetTicks() / kSDLTicksToSeconds;
            double currentTime = SDL_GetTicks() / kSDLTicksToSeconds;
            float deltaTime = float(currentTime - lastTime);

            // Get mouse position
            int xpos, ypos;
            SDL_GetMouseState(&xpos, &ypos);

            if (previous_y < 0) {
                previous_y = ypos;
                previous_x = xpos;
            }

            auto x_delta = (xpos - previous_x) / kSpeedScaling;
            auto y_delta = (ypos - previous_y) / kSpeedScaling;

            previous_x = xpos;
            previous_y = ypos;

            horizontalAngle += mouseSpeed * float(x_delta);
            verticalAngle += mouseSpeed * float(y_delta);

            // Direction : Spherical coordinates to Cartesian coordinates conversion
            glm::vec3 direction(std::cos(verticalAngle) * std::sin(horizontalAngle), std::sin(verticalAngle),
                std::cos(verticalAngle) * std::cos(horizontalAngle));

            // Right vector
            glm::vec3 right = glm::vec3(
                std::sin(horizontalAngle - static_cast<float>(M_PI) / 2), 0, std::cos(horizontalAngle - static_cast<float>(M_PI) / 2));

            // Up vector
            glm::vec3 up = glm::cross(right, direction);

            auto motion = handleMotion(event);

            switch (motion) {
            case FPSControlsImpl::NONE:
                break;
            case FPSControlsImpl::FORWARD:
                position += direction * deltaTime * speed;
                break;
            case FPSControlsImpl::BACKWARD:
                position -= direction * deltaTime * speed;
                break;
            case FPSControlsImpl::LEFT:
                position -= right * deltaTime * speed;
                break;
            case FPSControlsImpl::RIGHT:
                position += right * deltaTime * speed;
                break;
            default:
                LOG(FATAL) << "Unknown option!";
            }

            float FoV = initialFoV;

            // Projection matrix : 45° Field of View, 4:3 ratio, display range
            // : 0.1 unit <-> 100 units
            m_projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);

            // Camera matrix
            m_viewMatrix = glm::lookAt(position, // Camera is here
                position + direction, // and looks here : at the same position, plus "direction"
                up // Head is up (set to 0,-1,0 to look upside-down)
                );

            // For the next frame, the "last time" will be "now"
            lastTime = currentTime;
        }

    protected:
        float horizontalAngle = M_PI;
        float verticalAngle = 0.0f;
        float initialFoV = 45.0f;
        float speed = 3.0f;
        float mouseSpeed = 0.005f;
        int previous_x;
        int previous_y;
        bool m_init;
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projectionMatrix;
        glm::vec3 position = glm::vec3(0, 0, 5);
    };

    FPSControls::FPSControls()
        : m_impl(new FPSControlsImpl()) {}

    void FPSControls::update(const SDL_Event& event) { m_impl->update(event); }

    glm::mat4 FPSControls::getViewMatrix() { return m_impl->getViewMatrix(); }
    glm::mat4 FPSControls::getProjectionMatrix() { return m_impl->getProjectionMatrix(); }

} // visualization
} // perception
