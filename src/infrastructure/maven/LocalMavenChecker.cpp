#include "infrastructure/maven/LocalMavenChecker.h"

#include "infrastructure/maven/MavenWrapperGenerator.h"

bool LocalMavenChecker::isMavenInstalled() const
{
    return MavenWrapperGenerator::isMavenInstalled();
}
