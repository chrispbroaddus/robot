#include "Interactor.h"
#include <glm/gtc/constants.hpp> // pi
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp> // length2
#include <glm/vec3.hpp>

namespace perception {
namespace visualization {

    const glm::vec3 Interactor::X(1.f, 0.f, 0.f);
    const glm::vec3 Interactor::Y(0.f, 1.f, 0.f);
    const glm::vec3 Interactor::Z(0.f, 0.f, 1.f);

    Interactor::Interactor()
        : mCameraMotionLeftClick(ARC)
        , mCameraMotionMiddleClick(ROLL)
        , mCameraMotionRightClick(FIRSTPERSON)
        , mCameraMotionScroll(ZOOM)
        , mHeight(1)
        , mIsDragging(false)
        , mIsLeftClick(false)
        , mIsMiddleClick(false)
        , mIsRightClick(false)
        , mIsScrolling(false)
        , mPanScale(.005f)
        , mRollScale(.005f)
        , mRollSum(0.f)
        , mRotation(1.f, 0, 0, 0)
        , mRotationSum(1.f, 0, 0, 0)
        , mSpeed(1.f)
        , mWidth(1)
        , mZoomScale(.1f)
        , mZoomSum(0.f) {}

    char Interactor::clickQuadrant(float x, float y) {
        float halfw = mWidth / 2;
        float halfh = mHeight / 2;

        if (x > halfw) {
            // Opengl image coordinates origin is upperleft.
            if (y < halfh) {
                return 1;
            } else {
                return 4;
            }
        } else {
            if (y < halfh) {
                return 2;
            } else {
                return 3;
            }
        }
    }

    void Interactor::computeCameraEye(glm::vec3& eye) {
        glm::vec3 orientation = mRotationSum * Z;

        if (mZoomSum) {
            mTranslateLength += mZoomScale * mZoomSum;
            mZoomSum = 0; // Freeze zooming after applying.
        }

        eye = mTranslateLength * orientation + mCamera->getCenter();
    }

    void Interactor::computeCameraUp(glm::vec3& up) { up = glm::normalize(mRotationSum * Y); }

    void Interactor::computePan(glm::vec3& pan) {
        glm::vec2 click = mClickPoint - mPrevClickPoint;
        glm::vec3 look = mCamera->getEye() - mCamera->getCenter();
        float length = glm::length(look);
        glm::vec3 right = glm::normalize(mRotationSum * X);

        pan = (mCamera->getUp() * -click.y + right * click.x) * mPanScale * mSpeed * length;
    }

    void Interactor::computePointOnSphere(const glm::vec2& point, glm::vec3& result) {
        // https://www.opengl.org/wiki/Object_Mouse_Trackball
        float x = (2.f * point.x - mWidth) / mWidth;
        float y = (mHeight - 2.f * point.y) / mHeight;

        float length2 = x * x + y * y;

        if (2 * length2 <= 1) {
            result.z = std::sqrt(1 - length2);
        } else {
            result.z = 1 / (2 * std::sqrt(length2));
        }

        float norm = 1 / std::sqrt(length2 + result.z * result.z);

        result.x = x * norm;
        result.y = y * norm;
        result.z *= norm;
    }

    void Interactor::computeRotationBetweenVectors(const glm::vec3& u, const glm::vec3& v, glm::quat& result) {
        float cosTheta = glm::dot(u, v);
        glm::vec3 rotationAxis(glm::uninitialize);
        static const float EPSILON = 1.0e-5f;

        if (cosTheta < -1.0f + EPSILON) {
            // Parallel and opposite directions.
            rotationAxis = glm::cross(glm::vec3(0.f, 0.f, 1.f), u);

            if (glm::length2(rotationAxis) < 0.01f) {
                // Still parallel, retry.
                rotationAxis = glm::cross(glm::vec3(1.f, 0.f, 0.f), u);
            }

            rotationAxis = glm::normalize(rotationAxis);
            result = glm::angleAxis(180.0f, rotationAxis);
        } else if (cosTheta > 1.0f - EPSILON) {
            // Parallel and same direction.
            result = glm::quat(1, 0, 0, 0);
            return;
        } else {
            float theta = acos(cosTheta);
            rotationAxis = glm::cross(u, v);

            rotationAxis = glm::normalize(rotationAxis);
            result = glm::angleAxis(theta * mSpeed, rotationAxis);
        }
    }

    void Interactor::drag() {
        if (mPrevClickPoint == mClickPoint) {
            // Not moving during drag state, so skip unnecessary processing.
            return;
        }

        computePointOnSphere(mClickPoint, mStopVector);
        computeRotationBetweenVectors(mStartVector, mStopVector, mRotation);
        // Reverse so scene moves with cursor and not away due to camera model.
        mRotation = glm::inverse(mRotation);

        drag(mIsLeftClick, mCameraMotionLeftClick);
        drag(mIsMiddleClick, mCameraMotionMiddleClick);
        drag(mIsRightClick, mCameraMotionRightClick);

        // After applying drag, reset relative start state.
        mPrevClickPoint = mClickPoint;
        mStartVector = mStopVector;
    }

    void Interactor::drag(bool isClicked, CameraMotionType motion) {
        if (!isClicked) {
            return;
        }

        switch (motion) {
        case ARC:
            dragArc();
            break;
        case FIRSTPERSON:
            dragFirstPerson();
            break;
        case PAN:
            dragPan();
            break;
        case ROLL:
            rollCamera();
            break;
        case ZOOM:
            dragZoom();
            break;
        default:
            break;
        }
    }

    void Interactor::dragArc() {
        mRotationSum *= mRotation; // Accumulate quaternions.

        updateCameraEyeUp(true, true);
    }

    void Interactor::dragFirstPerson() {
        glm::vec3 pan(glm::uninitialize);
        computePan(pan);
        mCamera->setCenter(pan + mCamera->getCenter());
        mCamera->update();
        freezeTransform();
    }

    void Interactor::dragPan() {
        glm::vec3 pan(glm::uninitialize);
        computePan(pan);
        mCamera->setCenter(pan + mCamera->getCenter());
        mCamera->setEye(pan + mCamera->getEye());
        mCamera->update();
        freezeTransform();
    }

    void Interactor::dragZoom() {
        glm::vec2 dir = mClickPoint - mPrevClickPoint;
        float ax = fabs(dir.x);
        float ay = fabs(dir.y);

        if (ay >= ax) {
            setScrollDirection(dir.y <= 0);
        } else {
            setScrollDirection(dir.x <= 0);
        }

        updateCameraEyeUp(true, false);
    }

    void Interactor::freezeTransform() {
        if (mCamera) {
            // Opengl is ZYX order.
            // Flip orientation to rotate scene with sticky cursor.
            mRotationSum = glm::inverse(glm::quat(mCamera->getMatrix()));
            mTranslateLength = glm::length(mCamera->getEye() - mCamera->getCenter());
        }
    }

    Camera* Interactor::getCamera() { return mCamera; }

    Interactor::CameraMotionType Interactor::getMotionLeftClick() { return mCameraMotionLeftClick; }

    Interactor::CameraMotionType Interactor::getMotionMiddleClick() { return mCameraMotionMiddleClick; }

    Interactor::CameraMotionType Interactor::getMotionRightClick() { return mCameraMotionRightClick; }

    Interactor::CameraMotionType Interactor::getMotionScroll() { return mCameraMotionScroll; }

    void Interactor::rollCamera() {
        glm::vec2 delta = mClickPoint - mPrevClickPoint;
        char quad = clickQuadrant(mClickPoint.x, mClickPoint.y);
        switch (quad) {
        case 1:
            delta.y = -delta.y;
            delta.x = -delta.x;
            break;
        case 2:
            delta.x = -delta.x;
            break;
        case 3:
            break;
        case 4:
            delta.y = -delta.y;
        default:
            break;
        }

        glm::vec3 axis = glm::normalize(mCamera->getCenter() - mCamera->getEye());
        float angle = mRollScale * mSpeed * (delta.x + delta.y + mRollSum);
        glm::quat rot = glm::angleAxis(angle, axis);
        mCamera->setUp(rot * mCamera->getUp());
        mCamera->update();
        freezeTransform();
        mRollSum = 0;
    }

    void Interactor::scroll() {
        switch (mCameraMotionScroll) {
        case ROLL:
            rollCamera();
            break;
        case ZOOM:
            updateCameraEyeUp(true, false);
            break;
        default:
            break;
        }
    }

    void Interactor::setCamera(Camera* c) {
        mCamera = c;
        freezeTransform();
    }

    void Interactor::setClickPoint(double x, double y) {
        mPrevClickPoint = mClickPoint;
        mClickPoint.x = x;
        mClickPoint.y = y;
    }

    void Interactor::setLeftClicked(bool value) { mIsLeftClick = value; }

    void Interactor::setMiddleClicked(bool value) { mIsMiddleClick = value; }

    void Interactor::setMotionLeftClick(CameraMotionType motion) { mCameraMotionLeftClick = motion; }

    void Interactor::setMotionMiddleClick(CameraMotionType motion) { mCameraMotionMiddleClick = motion; }

    void Interactor::setMotionRightClick(CameraMotionType motion) { mCameraMotionRightClick = motion; }

    void Interactor::setMotionScroll(CameraMotionType motion) { mCameraMotionScroll = motion; }

    void Interactor::setRightClicked(bool value) { mIsRightClick = value; }

    void Interactor::setScreenSize(float width, float height) {
        if (width > 1 && height > 1) {
            mWidth = width;
            mHeight = height;
        }
    }

    void Interactor::setScrollDirection(bool up) {
        mIsScrolling = true;
        float inc = mSpeed * (up ? -1.f : 1.f);
        mZoomSum += inc;
        mRollSum += inc;
    }

    void Interactor::setSpeed(float s) { mSpeed = s; }

    void Interactor::update() {
        const bool isClick = mIsLeftClick || mIsMiddleClick || mIsRightClick;

        if (!mIsDragging) {
            if (isClick) {
                mIsDragging = true;
                computePointOnSphere(mClickPoint, mStartVector);
            } else if (mIsScrolling) {
                scroll();
                mIsScrolling = false;
            }
        } else {
            if (isClick) {
                drag();
            } else {
                mIsDragging = false;
            }
        }
    }

    void Interactor::updateCameraEyeUp(bool eye, bool up) {
        if (eye) {
            glm::vec3 eye(glm::uninitialize);
            computeCameraEye(eye);
            mCamera->setEye(eye);
        }
        if (up) {
            glm::vec3 up(glm::uninitialize);
            computeCameraUp(up);
            mCamera->setUp(up);
        }
        mCamera->update();
    }
}
}
