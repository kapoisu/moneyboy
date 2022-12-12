add_library(io STATIC)
target_sources(io PRIVATE bankable.cpp)
target_sources(io PRIVATE bus.cpp)
target_sources(io PRIVATE cartridge.cpp)