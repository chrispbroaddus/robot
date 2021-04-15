
#pragma once

#include "sample.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace filter_graph {

class Container {
public:
    typedef std::unordered_map<std::string, std::shared_ptr<Sample> > map_t;

    Container() = default;
    Container(const std::string& streamId, std::shared_ptr<Sample> sample) { add(streamId, sample); }
    ~Container() = default;

    ///
    /// Add a sample into the container.
    ///
    void add(const std::string& streamId, std::shared_ptr<Sample> sample) {
        map_t::iterator it = m_map.find(streamId);
        if (it == m_map.end()) {
            m_map.insert(it, std::make_pair(streamId, sample));
        } else {
            throw std::runtime_error("streamId already exists");
        }
    }

    ///
    /// Get a sample from a streamId.
    ///
    std::shared_ptr<Sample> get(const std::string& streamId) {
        map_t::iterator it = m_map.find(streamId);
        if (it == m_map.end()) {
            return std::shared_ptr<Sample>();
        } else {
            return it->second;
        }
    }
    std::shared_ptr<const Sample> get(const std::string& streamId) const {
        map_t::const_iterator it = m_map.find(streamId);
        if (it == m_map.end()) {
            return std::shared_ptr<const Sample>();
        } else {
            return it->second;
        }
    }

    ///
    /// Erase a sample from the container.
    ///
    bool erase(const std::string& streamId) {
        map_t::iterator it = m_map.find(streamId);
        if (it == m_map.end()) {
            return false;
        } else {
            m_map.erase(it);
            return true;
        }
    }

    ///
    /// Size of the map
    ///
    inline size_t size() const { return m_map.size(); }

private:
    map_t m_map;
};
}
