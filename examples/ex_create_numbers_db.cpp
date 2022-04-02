#include <sstream>
#include <iomanip>
#include <iostream>
#include <cmath>

void create(int digits=2) {
    using uint = unsigned long;
    uint numbers_count = std::pow(10, digits);
    std::stringstream ss;
    for (int ix = 0; ix < numbers_count; ++ix) {
        ss << std::setw(digits) << std::setfill('0') << ix;
    }
//    ss << std::setw(digits) << std::setfill('0') << 5;
    std::string s = ss.str();
    std::cout << s << std::endl;

}

int main() {
    create(2);
    create(3);
}

