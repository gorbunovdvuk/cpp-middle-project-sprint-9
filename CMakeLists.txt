cmake_minimum_required(VERSION 3.30)
project(MandelbrotFractal  VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Ищем необходимые библиотеки
find_package(GTest REQUIRED)

include(FetchContent)
FetchContent_Declare(
  CPM
  GIT_REPOSITORY https://github.com/cpm-cmake/CPM.cmake
  GIT_TAG origin/master
)
FetchContent_MakeAvailable(CPM)

CPMAddPackage(
    NAME stdexec
    GITHUB_REPOSITORY NVIDIA/stdexec
    GIT_TAG main # This will always pull the latest code from the `main` branch. You may also use a specific release version or tag
    OPTIONS
        "STDEXEC_BUILD_EXAMPLES OFF"
        "STDEXEC_BUILD_TESTS OFF"
)

CPMAddPackage(
        NAME SFML
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 3.0.1
        OPTIONS
        "BUILD_SHARED_LIBS=OFF"
        "SFML_BUILD_EXAMPLES=OFF"
        "SFML_BUILD_DOC=OFF"
)

file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/include/*.hpp")
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")

# Создаём статическую библиотеку
add_library(${PROJECT_NAME}_imp ${SRC_FILES} ${HEADER_FILES})

# Добавляем в проект используемые сторонние PRIVATE
target_include_directories(${PROJECT_NAME}_imp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${stdexec_SOURCE_DIR}/include
    ${SFML_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}_imp PUBLIC
    STDEXEC::stdexec
    sfml-graphics sfml-window sfml-system)

# Создаём исполняемый таргет и линкуем к нему статическую библиотеку
add_executable(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/src/main.cpp")
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_imp)

#
# Тесты
#
# Рекурсивно ищем все используемые в тестах .cpp файлы
file(GLOB TEST_SRC_FILES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_executable(${PROJECT_NAME}_tests "${TEST_SRC_FILES}")
target_link_libraries(${PROJECT_NAME}_tests PUBLIC ${PROJECT_NAME}_imp gtest::gtest)

# Включаем тестирование
enable_testing()
add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
