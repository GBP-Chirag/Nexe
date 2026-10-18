#include "engine.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <queue>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#endif

namespace engine {
namespace {
std::string trim(std::string value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return first >= last ? "" : std::string(first, last);
}
std::string unquote(const std::string& value) {
    return value.size() >= 2 && value.front() == '"' && value.back() == '"' ? value.substr(1, value.size() - 2) : "";
}
bool stringArray(const std::string& value, std::vector<std::string>& values) {
    values.clear(); const std::string input = trim(value);
    if (input.size() < 2 || input.front() != '[' || input.back() != ']') return false;
    std::size_t pos = 1;
    while (pos < input.size() - 1) {
        while (pos < input.size() - 1 && (std::isspace(static_cast<unsigned char>(input[pos])) || input[pos] == ',')) ++pos;
        if (pos == input.size() - 1) break;
        if (input[pos] != '"') return false;
        const std::size_t end = input.find('"', pos + 1); if (end == std::string::npos) return false;
        values.push_back(input.substr(pos + 1, end - pos - 1)); pos = end + 1;
        while (pos < input.size() - 1 && std::isspace(static_cast<unsigned char>(input[pos]))) ++pos;
        if (pos < input.size() - 1 && input[pos] != ',') return false;
    }
    return true;
}
bool parseField(Task& task, const std::string& key, const std::string& value, std::string& error) {
    if (key == "name" || key == "command") {
        const std::string parsed = unquote(value);
        if (parsed.empty()) { error = key + " must be a non-empty quoted string"; return false; }
        if (key == "name") task.name = parsed; else task.command = parsed;
        return true;
    }
    if (key == "inputs" || key == "outputs" || key == "dependencies" || key == "deps") {
        std::vector<std::string> parsed;
        if (!stringArray(value, parsed)) { error = key + " must be an array of quoted strings"; return false; }
        if (key == "inputs") task.inputs = std::move(parsed);
        else if (key == "outputs") task.outputs = std::move(parsed);
        else task.dependencies = std::move(parsed);
        return true;
    }
    error = "unknown task key '" + key + "'"; return false;
}
bool validTask(const Task& task, std::string& error) {
    if (task.name.empty() || task.command.empty()) { error = "every [[task]] needs both name and command"; return false; }
    return true;
}

bool expandVariables(std::string& value, const std::unordered_map<std::string, std::string>& variables, std::string& error) {
    std::size_t start = 0;
    while ((start = value.find("${", start)) != std::string::npos) {
        const std::size_t end = value.find('}', start + 2);
        if (end == std::string::npos) { error = "unterminated variable reference"; return false; }
        const std::string name = value.substr(start + 2, end - start - 2);
        const auto variable = variables.find(name);
        if (variable == variables.end()) { error = "undefined variable '" + name + "'"; return false; }
        value.replace(start, end - start + 1, variable->second);
        start += variable->second.size();
    }
    return true;
}

std::string shellQuote(const std::string& value) {
    std::string quoted = "'";
    for (const char character : value) quoted += character == '\'' ? "'\\''" : std::string(1, character);
    return quoted + "'";
}

void replaceAll(std::string& value, const std::string& needle, const std::string& replacement) {
    std::size_t position = 0;
    while ((position = value.find(needle, position)) != std::string::npos) {
        value.replace(position, needle.size(), replacement);
        position += replacement.size();
    }
}

#ifdef _WIN32
std::string base64Encode(const std::string& bytes) {
    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    for (std::size_t i = 0; i < bytes.size(); i += 3) {
        const uint32_t chunk = (static_cast<uint32_t>(static_cast<unsigned char>(bytes[i])) << 16) |
            (i + 1 < bytes.size() ? static_cast<uint32_t>(static_cast<unsigned char>(bytes[i + 1])) << 8 : 0) |
            (i + 2 < bytes.size() ? static_cast<uint32_t>(static_cast<unsigned char>(bytes[i + 2])) : 0);
        encoded += alphabet[(chunk >> 18) & 63]; encoded += alphabet[(chunk >> 12) & 63];
        encoded += i + 1 < bytes.size() ? alphabet[(chunk >> 6) & 63] : '=';
        encoded += i + 2 < bytes.size() ? alphabet[chunk & 63] : '=';
    }
    return encoded;
}

int runNativeShell(const std::string& command) {
    // PowerShell's EncodedCommand takes UTF-16LE Base64, avoiding cmd.exe
    // quoting rules and supporting paths/commands containing spaces.
    const int length = MultiByteToWideChar(CP_UTF8, 0, command.data(), static_cast<int>(command.size()), nullptr, 0);
    if (length <= 0) return -1;
    std::wstring wide(static_cast<std::size_t>(length), L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, command.data(), static_cast<int>(command.size()), wide.data(), length) <= 0) return -1;
    std::string utf16;
    utf16.reserve(wide.size() * 2);
    for (const wchar_t character : wide) { utf16 += static_cast<char>(character & 0xff); utf16 += static_cast<char>((character >> 8) & 0xff); }
    return std::system(("powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -EncodedCommand " + base64Encode(utf16)).c_str());
}
#else
int runNativeShell(const std::string& command) {
    return std::system(command.c_str());
}
#endif

bool quotedValue(const std::string& statement, const std::string& keyword, std::string& value) {
    if (statement.rfind(keyword, 0) != 0) return false;
    const std::string remainder = trim(statement.substr(keyword.size()));
    if (remainder.size() < 2 || remainder.front() != '"' || remainder.back() != '"') return false;
    value = remainder.substr(1, remainder.size() - 2);
    return true;
}

bool finishSblNode(Task& task, std::string commandTemplate,
                   const std::unordered_map<std::string, std::string>& variables, std::vector<Task>& tasks,
                   std::string& error) {
    if (task.name.empty() || commandTemplate.empty()) { error = "every node needs a name and run command"; return false; }
    if (task.inputs.empty() || task.outputs.empty()) { error = "node '" + task.name + "' needs at least one source and target"; return false; }
    if (!expandVariables(commandTemplate, variables, error)) return false;
    std::string sources;
    for (const auto& source : task.inputs) { if (!sources.empty()) sources += " "; sources += shellQuote(source); }
    replaceAll(commandTemplate, "$SOURCES", sources);
    replaceAll(commandTemplate, "$TARGET", shellQuote(task.outputs.front()));
    task.command = std::move(commandTemplate);
    tasks.push_back(std::move(task));
    return true;
}

bool parseSbl(std::istream& file, std::vector<Task>& tasks, std::string& error) {
    std::unordered_map<std::string, std::string> variables;
    Task current; std::string commandTemplate, line; bool inNode = false; std::size_t lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber; const auto comment = line.find('#'); if (comment != std::string::npos) line.erase(comment); line = trim(line);
        if (line.empty()) continue;
        if (line == "}") {
            if (!inNode) { error = "line " + std::to_string(lineNumber) + ": unexpected '}'"; return false; }
            if (!finishSblNode(current, commandTemplate, variables, tasks, error)) { error = "line " + std::to_string(lineNumber) + ": " + error; return false; }
            current = Task{}; commandTemplate.clear(); inNode = false; continue;
        }
        if (!inNode && line.rfind("node ", 0) == 0 && line.back() == '{') {
            const std::string name = trim(line.substr(5, line.size() - 6));
            if (name.empty() || name.find_first_of(" \t") != std::string::npos) { error = "line " + std::to_string(lineNumber) + ": node name must be one word"; return false; }
            current.name = name; inNode = true; continue;
        }
        if (!inNode) {
            const auto equals = line.find('=');
            if (equals == std::string::npos) { error = "line " + std::to_string(lineNumber) + ": expected NAME = \"value\" or node"; return false; }
            const std::string key = trim(line.substr(0, equals)), value = trim(line.substr(equals + 1));
            if (key.empty() || value.size() < 2 || value.front() != '"' || value.back() != '"') { error = "line " + std::to_string(lineNumber) + ": variables must use NAME = \"value\""; return false; }
            std::string expanded = value.substr(1, value.size() - 2);
            if (!expandVariables(expanded, variables, error)) { error = "line " + std::to_string(lineNumber) + ": " + error; return false; }
            variables[key] = std::move(expanded); continue;
        }
        std::string value;
        if (quotedValue(line, "source", value)) {
            if (!expandVariables(value, variables, error)) { error = "line " + std::to_string(lineNumber) + ": " + error; return false; }
            current.inputs.push_back(std::move(value)); continue;
        }
        if (quotedValue(line, "target", value)) {
            if (!expandVariables(value, variables, error)) { error = "line " + std::to_string(lineNumber) + ": " + error; return false; }
            current.outputs.push_back(std::move(value)); continue;
        }
        if (quotedValue(line, "depends", value)) { current.dependencies.push_back(std::move(value)); continue; }
        if (quotedValue(line, "run", value)) { commandTemplate = std::move(value); continue; }
        error = "line " + std::to_string(lineNumber) + ": expected source, target, depends, run, or '}'"; return false;
    }
    if (inNode) { error = "node '" + current.name + "' is missing closing '}'"; return false; }
    if (tasks.empty()) { error = "BUILD.sbl contains no nodes"; return false; }
    return true;
}
} // namespace

bool Engine::loadManifest(const std::filesystem::path& manifest, std::string& error) {
    std::ifstream file(manifest);
    if (!file) { error = "cannot open manifest '" + manifest.string() + "'"; return false; }
    tasks_.clear(); root_ = std::filesystem::absolute(manifest).parent_path();
    if (manifest.extension() == ".sbl") return parseSbl(file, tasks_, error);
    Task current; bool inTask = false; std::string line; std::size_t lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber; const auto comment = line.find('#'); if (comment != std::string::npos) line.erase(comment); line = trim(line);
        if (line.empty()) continue;
        if (line == "[[task]]") {
            if (inTask) { if (!validTask(current, error)) { error += " before line " + std::to_string(lineNumber); return false; } tasks_.push_back(std::move(current)); current = Task{}; }
            inTask = true; continue;
        }
        if (!inTask) { error = "line " + std::to_string(lineNumber) + " is outside a [[task]] table"; return false; }
        const auto equals = line.find('=');
        if (equals == std::string::npos) { error = "line " + std::to_string(lineNumber) + " must contain '='"; return false; }
        if (!parseField(current, trim(line.substr(0, equals)), trim(line.substr(equals + 1)), error)) { error = "line " + std::to_string(lineNumber) + ": " + error; return false; }
    }
    if (inTask) { if (!validTask(current, error)) return false; tasks_.push_back(std::move(current)); }
    if (tasks_.empty()) { error = "manifest contains no [[task]] tables"; return false; }
    return true;
}

bool Engine::executionPlan(std::vector<std::size_t>& plan, std::string& error) const {
    std::unordered_map<std::string, std::size_t> index; std::vector<std::size_t> degree(tasks_.size()); std::vector<std::vector<std::size_t>> outgoing(tasks_.size());
    for (std::size_t i = 0; i < tasks_.size(); ++i) if (!index.emplace(tasks_[i].name, i).second) { error = "duplicate task name '" + tasks_[i].name + "'"; return false; }
    for (std::size_t i = 0; i < tasks_.size(); ++i) for (const auto& dependency : tasks_[i].dependencies) {
        const auto found = index.find(dependency);
        if (found == index.end()) { error = "task '" + tasks_[i].name + "' depends on unknown task '" + dependency + "'"; return false; }
        outgoing[found->second].push_back(i); ++degree[i];
    }
    std::queue<std::size_t> ready; for (std::size_t i = 0; i < tasks_.size(); ++i) if (degree[i] == 0) ready.push(i);
    plan.clear(); while (!ready.empty()) { const auto current = ready.front(); ready.pop(); plan.push_back(current); for (const auto next : outgoing[current]) if (--degree[next] == 0) ready.push(next); }
    if (plan.size() != tasks_.size()) { error = "task dependency graph contains a cycle"; return false; }
    return true;
}

std::filesystem::path Engine::statePath(const Task& task) const {
    std::string safe; for (const unsigned char c : task.name) safe += std::isalnum(c) || c == '-' || c == '_' ? static_cast<char>(c) : '_';
    return root_ / ".nexe" / "state" / (safe + ".hash");
}
bool Engine::outputsExist(const Task& task) const {
    return !task.outputs.empty() && std::all_of(task.outputs.begin(), task.outputs.end(), [this](const std::string& output) { return std::filesystem::exists(root_ / output); });
}
bool Engine::build(std::string& error) {
    std::vector<std::size_t> plan; if (!executionPlan(plan, error)) return false;
    for (const auto index : plan) {
        const Task& task = tasks_[index]; const auto state = statePath(task); const std::string fingerprint = Hashing::taskFingerprint(task, root_, error); if (fingerprint.empty()) return false;
        std::string previous; { std::ifstream input(state); std::getline(input, previous); }
        if (previous == fingerprint && outputsExist(task)) { std::cout << "[skip] " << task.name << " (inputs unchanged)\n"; continue; }
        std::cout << "[build] " << task.name << "\n  $ " << task.command << '\n';
        std::error_code directoryError;
        const auto originalDirectory = std::filesystem::current_path(directoryError);
        if (directoryError) { error = "cannot read current directory: " + directoryError.message(); return false; }
        std::filesystem::current_path(root_, directoryError);
        if (directoryError) { error = "cannot enter build directory: " + directoryError.message(); return false; }
        const int commandStatus = runNativeShell(task.command);
        std::error_code restoreError;
        std::filesystem::current_path(originalDirectory, restoreError);
        if (restoreError) { error = "cannot restore current directory: " + restoreError.message(); return false; }
        if (commandStatus != 0) { error = "task '" + task.name + "' failed"; return false; }
        if (!outputsExist(task)) { error = "task '" + task.name + "' completed but did not create every declared output"; return false; }
        std::error_code createError; std::filesystem::create_directories(state.parent_path(), createError); if (createError) { error = "cannot create state directory: " + createError.message(); return false; }
        const auto temporary = state.string() + ".tmp"; { std::ofstream output(temporary); if (!output) { error = "cannot write state for '" + task.name + "'"; return false; } output << fingerprint << '\n'; }
        std::error_code renameError; std::filesystem::rename(temporary, state, renameError);
        if (renameError) { std::filesystem::remove(state, renameError); renameError.clear(); std::filesystem::rename(temporary, state, renameError); }
        if (renameError) { error = "cannot store state for '" + task.name + "': " + renameError.message(); return false; }
    }
    return true;
}
} // namespace engine
