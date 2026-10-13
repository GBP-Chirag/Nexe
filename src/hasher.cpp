#include "engine.hpp"

std::vector<std::string> Engine::splitData(const std::string& data) {
    std::vector<std::string> parts;
    std::string current;
    for (char ch : data) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(ch);
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    return parts;
}

std::string Engine::hashData(const std::string& data) {
#if HAVE_BLAKE3
    uint8_t output[32];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, data.data(), data.size());
    blake3_hasher_finalize(&hasher, output, sizeof(output));

    std::ostringstream os;
    os << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < sizeof(output); ++i) {
        os << std::setw(2) << static_cast<int>(output[i]);
    }
    return os.str();
#else
    const std::size_t value = std::hash<std::string>{}(data);
    std::ostringstream os;
    os << std::hex << value;
    return os.str();
#endif
}

void Engine::processHashing() {
    const std::string sample = "example data to hash";
    const auto parts = splitData(sample);
    for (const auto& piece : parts) {
        const std::string hash = hashData(piece);
        dataStore_[piece] = hash;
        std::cout << "Hashed '" << piece << "' -> " << hash << '\n';
    }
}
