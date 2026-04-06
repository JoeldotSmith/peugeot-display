cmake_minimum_required(VERSION 3.20)
project(peugeot_display_sim)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)

add_subdirectory(lvgl)

target_compile_definitions(lvgl PUBLIC
    LV_CONF_INCLUDE_SIMPLE
    LV_LVGL_H_INCLUDE_SIMPLE
)

target_include_directories(lvgl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

# Link SDL2 into lvgl so its SDL driver can find it
target_link_libraries(lvgl PUBLIC ${SDL2_LIBRARIES})

add_executable(sim
    sim/main.cpp
    ui/ui.c
    ui/ui_Screen1.c
    ui/ui_font_bitcount_40.c
    ui/medium.c
    ui/largest.c
)

target_include_directories(sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    sim/
    ui/
    lvgl/
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(sim PRIVATE
    lvgl
    ${SDL2_LIBRARIES}
)
