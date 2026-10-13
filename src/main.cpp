#include "engine.hpp"

int main(int argc, char** argv) {
    CLI::App app{"postake_engine"};

    std::string input_file;
    app.add_option("-i,--input", input_file, "Input file")->required();

    std::string output_file;
    app.add_option("-o,--output", output_file, "Output file")->required();

    CLI11_PARSE(app, argc, argv);

    Engine engine(input_file, output_file);
    engine.run();
    return 0;
}