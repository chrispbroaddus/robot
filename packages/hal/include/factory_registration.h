
#pragma once

namespace hal {

class FactoryRegistration {
public:
    static FactoryRegistration& instance() {
        static FactoryRegistration s_instance;
        return s_instance;
    }

    void registerFactories();

private:
    FactoryRegistration() = default;
    ~FactoryRegistration() = default;
};

inline void Initialize() { FactoryRegistration::instance().registerFactories(); }
}