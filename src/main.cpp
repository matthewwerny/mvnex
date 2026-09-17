#include "application/init/InitUseCase.h"

#include "infrastructure/cli/routing/CommandRegistry.h"
#include "infrastructure/cli/routing/CommandRouter.h"

#include "infrastructure/cli/commands/init/InitCommand.h"
#include "infrastructure/cli/commands/add/AddCommand.h"
#include "infrastructure/cli/commands/init/InitConfigCollector.h"
#include "infrastructure/cli/output/ConsoleOutput.h"
#include "infrastructure/cli/output/ConsoleProgressReporter.h"
#include "infrastructure/cli/output/HelpPrinter.h"
#include "infrastructure/cli/prompt/Prompt.h"
#include "infrastructure/filesystem/FileSystem.h"
#include "infrastructure/maven/LocalMavenChecker.h"
#include "infrastructure/maven/MavenWrapperGenerator.h"
#include "infrastructure/project/ProjectGenerator.h"

#include <memory>

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

    CommandRegistry registry;

    registry.registerCommand(std::make_shared<InitCommand>(
        helpPrinter,
        consoleOutput,
        prompt,
        initConfigCollector,
        initUseCase));
    registry.registerCommand(std::make_shared<AddCommand>(
        helpPrinter,
        consoleOutput));

    CommandRouter router(registry, helpPrinter, consoleOutput, MVNEX_VERSION);

    return router.execute(argc, argv);
}
