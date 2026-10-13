#include "engine.hpp"

std::vector<std::string> Engine::buildExecutionPlan() const {
    DAG dag(3);
    dag.addEdge(0, 1);
    dag.addEdge(0, 2);
    dag.addEdge(1, 2);

    auto order = dag.topoSort();
    if (order.empty()) {
        return {};
    }

    const std::vector<std::string> steps = {
        "src/main.cpp",
        "src/dag.cpp",
        "src/hasher.cpp"
    };

    std::vector<std::string> plan;
    for (int node : order) {
        if (node >= 0 && node < static_cast<int>(steps.size())) {
            plan.push_back(steps[node]);
        }
    }
    return plan;
}

void Engine::processDAG(std::vector<std::string>& plan) {
    auto sortedPlan = buildExecutionPlan();
    if (sortedPlan.empty()) {
        std::cerr << "No valid DAG execution plan could be generated.\n";
        return;
    }

    std::cout << "Sorted DAG execution plan:\n";
    for (const auto& step : sortedPlan) {
        std::cout << "  - " << step << '\n';
    }

    plan = std::move(sortedPlan);
    executePlan(plan);
}

void Engine::executePlan(const std::vector<std::string>& sortedPlan) {
    std::cout << "Setting compiler environment variables...\n";
    setenv("CC", "gcc", 1);
    setenv("CXX", "g++", 1);
    setenv("CPPFLAGS", "-Iinclude", 1);
    setenv("CFLAGS", "-O2 -Wall", 1);
    setenv("CXXFLAGS", "-O2 -Wall -std=c++17", 1);

    const char* path = std::getenv("PATH");
    if (path != nullptr) {
        setenv("PATH", path, 1);
    }

    std::cout << "Executing local compiler commands via system()...\n";
    for (const auto& step : sortedPlan) {
        const std::string lower = step;
        if (lower.find(".hpp") != std::string::npos || lower.find(".h") != std::string::npos) {
            std::cout << "Skipping header-only dependency: " << step << '\n';
            continue;
        }

        std::string compiler = "gcc";
        const std::string compiler_check = "command -v clang >/dev/null 2>&1";
        if (std::system(compiler_check.c_str()) == 0) {
            compiler = "clang";
        }

        std::string command = compiler + " -Iinclude -c " + step + " -o " + step + ".o";
        std::cout << "  $ " << command << '\n';
        const int exit_code = std::system(command.c_str());
        if (exit_code != 0) {
            std::cerr << "Compiler invocation failed for '" << step << "' with exit code " << exit_code << '\n';
        }
    }
}

void task() {
    Task t("build", {"clean", "compile"});
    t.command = "make all";
    t.inputs = {"src/main.cpp", "src/dag.cpp"};
    t.outputs = {"build/postake_engine"};

    std::cout << "Task Name: " << t.name << "\n";
    std::cout << "Command: " << t.command << "\n";
    std::cout << "Dependencies: ";
    for (std::size_t i = 0; i < t.dependencies.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << t.dependencies[i];
    }
    std::cout << "\nInputs: ";
    for (std::size_t i = 0; i < t.inputs.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << t.inputs[i];
    }
    std::cout << "\nOutputs: ";
    for (std::size_t i = 0; i < t.outputs.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << t.outputs[i];
    }
    std::cout << std::endl;
}
