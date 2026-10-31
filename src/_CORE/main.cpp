/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/

#include "engine.hpp"

#include <filesystem>
#include <iostream>

namespace {
void printUsage(const char* program) {
    std::cerr << "Usage:\n"
              << "  " << program << " build <BUILD.sbl>\n"
              << "  " << program << " hash <file>\n";
}
}

int main(int argc, char** argv) {
    if (argc == 3 && std::string(argv[1]) == "hash") {
        std::string error;
        const std::string digest = engine::Hashing::hashFile(argv[2], error);
        if (digest.empty()) {
            std::cerr << "nexe_engine: " << error << '\n';
            return 1;
        }
        std::cout << digest << "  " << argv[2] << '\n';
        return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "build") {
        engine::Engine buildEngine;
        std::string error;
        if (!buildEngine.loadManifest(std::filesystem::path(argv[2]), error) ||
            !buildEngine.build(error)) {
            std::cerr << "nexe_engine: " << error << '\n';
            return 1;
        }
        return 0;
    }
    printUsage(argv[0]);
    return 2;
}
