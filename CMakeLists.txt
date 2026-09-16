cmake_minimum_required(VERSION 3.20)

project(mvnex VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    replxx
    GIT_REPOSITORY https://github.com/AmokHuginnsson/replxx.git
    GIT_TAG master
)

FetchContent_MakeAvailable(replxx)

add_executable(mvnex
    src/main.cpp

    src/commands/init/InitCommand.cpp
    src/commands/add/AddCommand.cpp

    src/project/ProjectNaming.cpp
    src/project/ProjectValidator.cpp
    src/project/ProjectGenerator.cpp

    src/cli/arguments/ArgumentParser.cpp
    src/cli/output/ConsoleOutput.cpp
    src/cli/output/HelpPrinter.cpp
    src/cli/prompt/Prompt.cpp
    src/cli/routing/CommandRegistry.cpp
    src/cli/routing/CommandRouter.cpp

    src/maven/MavenWrapperGenerator.cpp
    src/maven/MavenWrapperFiles.cpp
)

target_include_directories(mvnex PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(mvnex PRIVATE MVNEX_VERSION="${PROJECT_VERSION}")

target_link_libraries(mvnex PRIVATE
    replxx
)

include(CTest)
