#ifndef _DATE_HELPER_H_
#define _DATE_HELPER_H_

#include <string>
#include <sstream>
#include <chrono>

#include <date/date.h>

namespace utils {

using minute_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;

minute_point parse(const std::string& str) {
    std::istringstream in(str);
    minute_point       tp;
    in >> date::parse("%H:%M", tp);
    return tp;
}

}  // namespace utils

#endif  // _DATE_HELPER_H_
