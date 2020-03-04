project(Game)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(prog main.cpp game_state.cpp)

target_compile_features(prog PRIVATE cxx_std_17)
target_link_libraries(prog PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CONAN_LIBS})