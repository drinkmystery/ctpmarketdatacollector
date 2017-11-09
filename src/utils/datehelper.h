#ifndef _DATE_HELPER_H_
#define _DATE_HELPER_H_

#include <string>
#include <sstream>
#include <chrono>

#include <date/date.h>

namespace utils {

std::chrono::minutes parse(const std::string& str) {
    std::istringstream   in(str);
    std::chrono::minutes tp;
    in >> date::parse("%H:%M", tp);
    return tp;
}

}  // namespace utils

#endif  // _DATE_HELPER_H_
 