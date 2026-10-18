#include "engine.hpp"
#include "blake3.h"

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace engine {
namespace {
std::string hexDigest(const uint8_t digest[BLAKE3_OUT_LEN]) {
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < BLAKE3_OUT_LEN; ++i) stream << std::setw(2) << static_cast<unsigned>(digest[i]);
    return stream.str();
}
void updateString(blake3_hasher& hasher, const std::string& value) {
    const uint64_t length = value.size();
    blake3_hasher_update(&hasher, &length, sizeof(length));
    blake3_hasher_update(&hasher, value.data(), value.size());
}
} // namespace

std::string Hashing::hashFile(const std::filesystem::path& path, std::string& error) {
    std::ifstream input(path, std::ios::binary);
    if (!input) { error = "cannot read input '" + path.string() + "'"; return {}; }
    blake3_hasher hasher; blake3_hasher_init(&hasher);
    std::array<char, 64 * 1024> buffer{};
    while (input.read(buffer.data(), buffer.size()) || input.gcount() > 0) {
        blake3_hasher_update(&hasher, buffer.data(), static_cast<std::size_t>(input.gcount()));
    }
    if (!input.eof()) { error = "failed while reading '" + path.string() + "'"; return {}; }
    uint8_t digest[BLAKE3_OUT_LEN]; blake3_hasher_finalize(&hasher, digest, sizeof(digest));
    return hexDigest(digest);
}

std::string Hashing::taskFingerprint(const Task& task, const std::filesystem::path& root, std::string& error) {
    blake3_hasher hasher; blake3_hasher_init(&hasher);
    updateString(hasher, "nexe-task-v1"); updateString(hasher, task.name); updateString(hasher, task.command);
    for (const std::string& inputName : task.inputs) {
        const std::string digest = hashFile(root / inputName, error);
        if (digest.empty()) return {};
        updateString(hasher, inputName); updateString(hasher, digest);
    }
    uint8_t digest[BLAKE3_OUT_LEN]; blake3_hasher_finalize(&hasher, digest, sizeof(digest));
    return hexDigest(digest);
}
} // namespace engine
