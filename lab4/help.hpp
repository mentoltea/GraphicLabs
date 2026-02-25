#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>

std::string read_entire_file(const std::string& filename) {
    std::ifstream file(filename);
    
    std::string result;
    char buffer[512];
    while (!file.eof()) {
        std::memset(buffer, 0, sizeof(buffer));
        file.read(buffer, sizeof(buffer)-1);
        result += buffer;
    }
    return result;
}