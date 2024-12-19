cmake_minimum_required(VERSION 3.10)

project(GuitarShop)

set(CMAKE_CXX_STANDARD 14)

# Указываем пути к заголовочным файлам и библиотекам
include_directories(/usr/local/include /usr/local/include/libpq)
link_directories(/usr/local/lib)

add_executable(client main.cpp src/Window.cpp src/GUI.cpp src/ShopPages.cpp src/UserInteraction.cpp src/ZmqTools.cpp)
target_link_libraries(client zmq fltk fltk_images jpeg crypto icuuc icuio icudata "-framework Cocoa")

add_executable(server server.cpp)
target_link_libraries(server zmq pq spdlog fmt)