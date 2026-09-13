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
    src/cli/help/HelpPrinter.cpp
    src/cli/prompt/Prompt.cpp

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

if(BUILD_TESTING)
    add_executable(mvnex_project_validator_tests
        tests/ProjectValidatorTests.cpp
        src/project/ProjectValidator.cpp
        src/project/ProjectNaming.cpp
    )

    target_include_directories(mvnex_project_validator_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    add_test(NAME project_validator COMMAND mvnex_project_validator_tests)

    add_executable(mvnex_project_generator_tests
        tests/ProjectGeneratorTests.cpp
        src/project/ProjectGenerator.cpp
        src/project/ProjectNaming.cpp
        src/maven/MavenWrapperGenerator.cpp
        src/maven/MavenWrapperFiles.cpp
    )

    target_include_directories(mvnex_project_generator_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    add_test(NAME project_generator COMMAND mvnex_project_generator_tests)
endif()
