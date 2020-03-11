project(Game)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(prog main.cpp game_state.cpp)

if(MSVC)
    add_compile_options("/W4" "$<$<CONFIG:RELEASE>:/O2>")
else()
    add_compile_options("-Wall" "-Wextra" "-Werror" "-pedantic" "$<$<CONFIG:RELEASE>:-O3>")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_link_libraries(prog PRIVATE "-ltbb")
    endif()
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("-stdlib=libc++")
    endif()
endif()

target_compile_features(prog PRIVATE cxx_std_17)
target_include_directories(prog PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(prog PRIVATE ${CONAN_LIBS})