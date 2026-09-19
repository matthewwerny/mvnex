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

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.11.2
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(APPLE)
    set(CPR_USE_SYSTEM_CURL ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(replxx cpr)

add_executable(mvnex
    src/main.cpp

    src/application/errors/DependencyResolutionError.cpp
    src/application/errors/DependencyNotFound.cpp
    src/application/errors/DependencyResolverUnavailable.cpp
    src/application/errors/MultipleDependencyMatches.cpp

    src/application/add/AddUseCase.cpp
    src/application/init/InitUseCase.cpp

    src/infrastructure/cli/command/CommandMetadata.cpp
    src/infrastructure/cli/commands/add/AddCommand.cpp
    src/infrastructure/cli/commands/init/InitCommand.cpp
    src/infrastructure/cli/commands/init/InitConfigCollector.cpp

    src/domain/dependency/DependencyRequest.cpp
    src/domain/dependency/ResolvedDependency.cpp
    src/infrastructure/cli/dependency/DependencyArgumentsParser.cpp
    src/infrastructure/dependency/CompositeDependencyResolver.cpp
    src/infrastructure/dependency/DepsDevDependencyResolver.cpp
    src/infrastructure/dependency/MavenCentralDependencyResolver.cpp

    src/domain/project/ProjectNaming.cpp
    src/domain/project/ProjectValidator.cpp

    src/infrastructure/project/ProjectGenerator.cpp
    src/infrastructure/project/PomProjectDependencyRepository.cpp
    src/infrastructure/filesystem/FileSystem.cpp
    src/infrastructure/http/CprHttpClient.cpp

    src/infrastructure/cli/arguments/ArgumentParser.cpp
    src/infrastructure/cli/output/ConsoleOutput.cpp
    src/infrastructure/cli/output/ConsoleProgressReporter.cpp
    src/infrastructure/cli/output/HelpPrinter.cpp
    src/infrastructure/cli/prompt/Prompt.cpp
    src/infrastructure/cli/routing/CommandRegistry.cpp
    src/infrastructure/cli/routing/CommandRouter.cpp

    src/infrastructure/maven/MavenWrapperGenerator.cpp
    src/infrastructure/maven/MavenWrapperFiles.cpp
    src/infrastructure/maven/LocalMavenChecker.cpp
    src/infrastructure/maven/LocalMavenProjectValidator.cpp
)

target_include_directories(mvnex PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(mvnex PRIVATE MVNEX_VERSION="${PROJECT_VERSION}")

target_link_libraries(mvnex PRIVATE
    replxx
    cpr::cpr
)

include(CTest)
