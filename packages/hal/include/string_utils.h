
#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace hal {

inline std::string& ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

inline std::string& rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

inline std::string& trim(std::string& s) { return ltrim(rtrim(s)); }

template <typename TARGET_T> inline TARGET_T lexicalCast(const std::string& str) {
    std::istringstream ss(str);

    TARGET_T target;
    ss >> target;

    return target;
}
}
