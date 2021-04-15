
#include <chrono>
#include <iostream>

namespace hal {

class Timer {
public:
    Timer()
        : m_begin(m_clock::now()) {}
    void reset() { m_begin = m_clock::now(); }
    double elapsed() const { return std::chrono::duration_cast<m_second>(m_clock::now() - m_begin).count(); }

private:
    typedef std::chrono::high_resolution_clock m_clock;
    typedef std::chrono::duration<double, std::ratio<1> > m_second;
    std::chrono::time_point<m_clock> m_begin;
};
}