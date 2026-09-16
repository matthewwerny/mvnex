#include "cli/routing/CommandRegistry.h"
#include "cli/routing/CommandRouter.h"

#include "commands/init/InitCommand.h"
#include "commands/add/AddCommand.h"

#include <memory>

int main(int argc, char *argv[])
{
    CommandRegistry registry;

    registry.registerCommand(std::make_shared<InitCommand>());
    registry.registerCommand(std::make_shared<AddCommand>());

    CommandRouter router(registry, MVNEX_VERSION);

    return router.execute(argc, argv);
}