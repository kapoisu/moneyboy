#ifndef CARTRIDGE_STORAGE_H
#define CARTRIDGE_STORAGE_H

#include <string>
#include <vector>

namespace gameboy::cartridge {
    struct Storage {
        std::vector<std::uint8_t> rom;
    };

    Storage create_storage(const std::string& file_name);
}

#endif