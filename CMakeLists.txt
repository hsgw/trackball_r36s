cmake_minimum_required(VERSION 3.13)
project(trackball_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── SDL2 / SDL2_ttf / SDL2_mixer (pkg-config 経由) ──────────
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2       REQUIRED sdl2)
pkg_check_modules(SDL2_TTF   REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

# ── 実行ファイル ────────────────────────────────────────────
add_executable(trackball_demo src/main.cpp)

target_compile_options(trackball_demo PRIVATE
    -Wall -Wextra
    ${SDL2_CFLAGS_OTHER}
    ${SDL2_TTF_CFLAGS_OTHER}
    ${SDL2_MIXER_CFLAGS_OTHER}
)

target_include_directories(trackball_demo PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
)

target_link_directories(trackball_demo PRIVATE
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_TTF_LIBRARY_DIRS}
    ${SDL2_MIXER_LIBRARY_DIRS}
)

target_link_libraries(trackball_demo PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)

# Copy font to build directory
add_custom_command(TARGET trackball_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/PressStart2P-Regular.ttf"
    "${CMAKE_CURRENT_BINARY_DIR}/PressStart2P-Regular.ttf")

# Copy optional WAV files if they exist (ignore errors if missing)
add_custom_command(TARGET trackball_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shot.wav"
    "${CMAKE_CURRENT_BINARY_DIR}/shot.wav" || ${CMAKE_COMMAND} -E true)

add_custom_command(TARGET trackball_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/hit.wav"
    "${CMAKE_CURRENT_BINARY_DIR}/hit.wav" || ${CMAKE_COMMAND} -E true)
