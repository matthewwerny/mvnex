#include "application/init/InitUseCase.h"
#include "application/add/AddUseCase.h"

#include "infrastructure/cli/routing/CommandRegistry.h"
#include "infrastructure/cli/routing/CommandRouter.h"

#include "infrastructure/cli/commands/init/InitCommand.h"
#include "infrastructure/cli/commands/add/AddCommand.h"
#include "infrastructure/cli/commands/init/InitConfigCollector.h"
#include "infrastructure/cli/dependency/DependencyArgumentsParser.h"
#include "infrastructure/dependency/CompositeDependencyResolver.h"
#include "infrastructure/dependency/DepsDevDependencyResolver.h"
#include "infrastructure/dependency/MavenCentralDependencyResolver.h"
#include "infrastructure/cli/output/ConsoleOutput.h"
#include "infrastructure/cli/output/ConsoleProgressReporter.h"
#include "infrastructure/cli/output/HelpPrinter.h"
#include "infrastructure/cli/prompt/Prompt.h"
#include "infrastructure/filesystem/FileSystem.h"
#include "infrastructure/http/CprHttpClient.h"
#include "infrastructure/maven/LocalMavenChecker.h"
#include "infrastructure/maven/MavenWrapperGenerator.h"
#include "infrastructure/project/PomProjectDependencyRepository.h"
#include "infrastructure/project/ProjectGenerator.h"

#include <functional>
#include <memory>
#include <vector>

int main(int argc, char *argv[])
{
    HelpPrinter helpPrinter;
    ConsoleOutput consoleOutput;
    ConsoleProgressReporter progressReporter;
    Prompt prompt;

    FileSystem fileSystem;
    MavenWrapperGenerator mavenWrapperGenerator(fileSystem, progressReporter);
    ProjectGenerator projectGenerator(fileSystem, mavenWrapperGenerator);
    LocalMavenChecker mavenChecker;
    InitUseCase initUseCase(projectGenerator, mavenChecker);
    InitConfigCollector initConfigCollector(prompt);
    DependencyArgumentsParser dependencyArgumentsParser;
    CprHttpClient primaryHttpClient(5000);
    CprHttpClient fallbackHttpClient(1500);
    CprHttpClient depsDevHttpClient(5000);
    MavenCentralDependencyResolver sonatypeCentralDependencyResolver(
        primaryHttpClient,
        "https://central.sonatype.com/solrsearch/select");
    MavenCentralDependencyResolver mavenCentralDependencyResolver(fallbackHttpClient);
    DepsDevDependencyResolver depsDevDependencyResolver(depsDevHttpClient);
    CompositeDependencyResolver dependencyResolver({
        std::cref(static_cast<const DependencyResolver &>(sonatypeCentralDependencyResolver)),
        std::cref(static_cast<const DependencyResolver &>(mavenCentralDependencyResolver)),
        std::cref(static_cast<const DependencyResolver &>(depsDevDependencyResolver))});
    PomProjectDependencyRepository projectDependencyRepository(fileSystem);
    AddUseCase addUseCase(dependencyResolver, projectDependencyRepository);

    CommandRegistry registry;

    registry.registerCommand(std::make_shared<InitCommand>(
        helpPrinter,
        consoleOutput,
        prompt,
        initConfigCollector,
        initUseCase));
    registry.registerCommand(std::make_shared<AddCommand>(
        helpPrinter,
        consoleOutput,
        dependencyArgumentsParser,
        prompt,
        addUseCase));

    CommandRouter router(registry, helpPrinter, consoleOutput, MVNEX_VERSION);

    return router.execute(argc, argv);
}
