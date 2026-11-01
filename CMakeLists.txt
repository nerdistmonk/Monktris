cmake_minimum_required(VERSION 3.16)
project(monktris LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(VORBIS REQUIRED vorbisfile)

include_directories(
    src
    ${VORBIS_INCLUDE_DIRS}
)

set(SOURCES
    src/main.c
    src/playfield.c
    src/piece_control.c
    src/spawn_prng.c
    src/line_clear.c
    src/ppu.c
    src/apu.c
    src/tetris_game.c
    src/hud_score.c
    src/scores.c
)

add_executable(monktris ${SOURCES})

target_link_libraries(monktris PRIVATE
    SDL3::SDL3
    ${VORBIS_LIBRARIES}
    m
)

# Auto-copy sound tracks and gamepad definitions next to the binary
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/snd DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gamecontrollerdb.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})