/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/
#ifndef NEXE_DSL_AST_H
#define NEXE_DSL_AST_H

#include <string>
#include <vector>

namespace nexe::dsl {

// These nodes describe the public BUILD.sbl model. The C++ engine currently
// evaluates the built-in C/C++ rules directly; a future extension evaluator
// can use the same stable rule/toolchain boundary.
struct Toolchain {
    std::string name;
    std::string flavor; // "gnu" or "msvc"
    std::string cc;
    std::string cxx;
    std::vector<std::string> cflags;
    std::vector<std::string> cxxflags;
    std::vector<std::string> ldflags;
};

struct Rule {
    std::string kind; // cc_binary, cxx_binary, or node
    std::string name;
    std::string toolchain;
    std::vector<std::string> sources;
    std::vector<std::string> outputs;
    std::vector<std::string> dependencies;
};

} // namespace nexe::dsl

#endif // NEXE_DSL_AST_H
