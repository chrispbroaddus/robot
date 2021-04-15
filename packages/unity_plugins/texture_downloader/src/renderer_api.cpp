#include "renderer_api.h"
#include "glog/logging.h"
#include "platform_base.h"

using namespace unity_plugins;

/// @brief Set callback get data out of plugin
void RendererAPI::AddProcessingCallback(ProcessImageFuncPtr processingFunc) {
    if (processingFunc) {
        std::lock_guard<std::mutex> guard(m_processingFuncPtrsMutex);
        m_processingFuncPtrs.insert(processingFunc);
        CallbacksUpdated(m_processingFuncPtrs);
    }
}
void RendererAPI::RemoveProcessingCallback(ProcessImageFuncPtr processingFunc) {
    if (processingFunc) {
        std::lock_guard<std::mutex> guard(m_processingFuncPtrsMutex);
        m_processingFuncPtrs.erase(processingFunc);
        CallbacksUpdated(m_processingFuncPtrs);
    }
}

std::unordered_set<ProcessImageFuncPtr> RendererAPI::Callbacks() {
    std::lock_guard<std::mutex> guard(m_processingFuncPtrsMutex);
    return m_processingFuncPtrs;
}

std::shared_ptr<RendererAPI> CreateRendererAPI(UnityGfxRenderer apiType) {
// create api object for current platform
#if SUPPORT_OPENGL_CORE
    if (apiType == kUnityGfxRendererOpenGLCore) {
        extern std::shared_ptr<RendererAPI> CreateRendererAPI_GLCore();
        return CreateRendererAPI_GLCore();
    }
#endif
    // TODO add other platforms here. eg directX, vulkan, metal

    // Unsupported API
    return nullptr;
}
