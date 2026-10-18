/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/

#ifndef NEXE_ENGINE_HPP
#define NEXE_ENGINE_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace engine {

// A task maps to a node in BUILD.sbl. The engine also accepts the older
// [[task]] TOML format for compatibility.
struct Task {
    std::string name;
    std::string command;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<std::string> dependencies;
};

class Hashing {
public:
    static std::string hashFile(const std::filesystem::path& path, std::string& error);
    static std::string taskFingerprint(const Task& task, const std::filesystem::path& root,
                                       std::string& error);
};

class Engine {
public:
    bool loadManifest(const std::filesystem::path& manifest, std::string& error);
    bool build(std::string& error);
    const std::vector<Task>& tasks() const { return tasks_; }

private:
    std::vector<Task> tasks_;
    std::filesystem::path root_;

    bool executionPlan(std::vector<std::size_t>& plan, std::string& error) const;
    bool outputsExist(const Task& task) const;
    std::filesystem::path statePath(const Task& task) const;
};

} // namespace engine

#endif // NEXE_ENGINE_HPP
