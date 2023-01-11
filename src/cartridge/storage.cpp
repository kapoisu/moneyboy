#include "storage.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace gameboy::cartridge {
    Storage create_storage(const std::string& file_name)
    {
        static constexpr int bank_size{0x4000};

        std::cout << "Load cartridge: " << file_name << "\n";
        std::ifstream file{file_name, std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error{"The cartridge file does not exist.\n"};
        }

        Storage storage{};
        storage.rom.resize(bank_size);
        std::generate(storage.rom.begin(), storage.rom.end(), [&file](){ return file.get(); });

        auto rom_type{storage.rom[0x0148]};
        auto number_of_rom_banks{(2 << (rom_type / 16)) + (2 << (rom_type % 16))};
        storage.rom.resize(bank_size * number_of_rom_banks);

        std::cout << "ROM size: " << storage.rom.size() << "\n";
        for (auto i{1}; i < number_of_rom_banks; ++i) {
            auto begin{storage.rom.begin() + i * bank_size};
            auto end{storage.rom.begin() + (i + 1) * bank_size};
            std::generate(begin, end, [&file](){ return file.get(); });
        }

        return storage;
    }
}