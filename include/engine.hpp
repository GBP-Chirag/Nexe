/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#if defined(__has_include)
#  if __has_include(<blake3.h>)
#    include <blake3.h>
#    define HAVE_BLAKE3 1
#  else
#    define HAVE_BLAKE3 0
#  endif
#else
#  define HAVE_BLAKE3 0
#endif

#if defined(__has_include)
#  if __has_include(<CLI/CLI.hpp>)
#    include <CLI/CLI.hpp>
#    define HAVE_CLI11 1
#  else
#    define HAVE_CLI11 0
#  endif
#else
#  define HAVE_CLI11 0
#endif

#if !HAVE_CLI11
namespace CLI {
class Option {
public:
    Option& required() { return *this; }
};

class App {
public:
    explicit App(const std::string&) {}
    template <typename T>
    Option* add_option(const std::string&, T&) { static Option option; return &option; }
    template <typename T>
    Option* add_option(const std::string&, T&, const std::string&) { static Option option; return &option; }
};
}

#ifndef CLI11_PARSE
#define CLI11_PARSE(app, argc, argv) do { (void)(app); (void)(argc); (void)(argv); } while (0)
#endif
#endif

struct Task {
    std::string name;
    std::string command;
    std::vector<std::string> dependencies;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

    Task() = default;
    Task(const std::string& task_name, const std::vector<std::string>& task_dependencies)
        : name(task_name), dependencies(task_dependencies) {}
};

class DAG {
public:
    DAG() = default;
    explicit DAG(int n) : adj_(n) {}

    void addEdge(int u, int v) {
        if (u >= 0 && u < static_cast<int>(adj_.size())) {
            adj_[u].push_back(v);
        }
    }

    std::vector<int> topoSort() const {
        int n = static_cast<int>(adj_.size());
        std::vector<int> indegree(n, 0);
        std::queue<int> q;
        std::vector<int> order;

        for (int i = 0; i < n; ++i) {
            for (int v : adj_[i]) {
                ++indegree[v];
            }
        }

        for (int i = 0; i < n; ++i) {
            if (indegree[i] == 0) {
                q.push(i);
            }
        }

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order.push_back(u);
            for (int v : adj_[u]) {
                if (--indegree[v] == 0) {
                    q.push(v);
                }
            }
        }

        if (static_cast<int>(order.size()) != n) {
            return {};
        }
        return order;
    }

    void process() const {
        auto order = topoSort();
        if (order.empty()) {
            std::cout << "Cycle detected in DAG\n";
            return;
        }
        for (std::size_t i = 0; i < order.size(); ++i) {
            if (i) {
                std::cout << ' ';
            }
            std::cout << order[i];
        }
        std::cout << '\n';
    }

private:
    std::vector<std::vector<int>> adj_;
};

class Engine {
public:
    Engine() = default;
    Engine(const std::string& input_file, const std::string& output_file)
        : input_file_(input_file), output_file_(output_file) {}

    void run() {
        auto plan = buildExecutionPlan();
        processDAG(plan);
        processHashing();
    }

    std::vector<std::string> buildExecutionPlan() const;
    void executePlan(const std::vector<std::string>& sortedPlan);

private:
    void processDAG(std::vector<std::string>& plan);
    void processHashing();
    std::string hashData(const std::string& data);
    std::vector<std::string> splitData(const std::string& data);

    std::unordered_map<std::string, std::string> dataStore_;
    std::string input_file_;
    std::string output_file_;
};

void task();

#endif // ENGINE_HPP