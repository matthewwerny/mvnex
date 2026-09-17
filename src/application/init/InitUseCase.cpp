#include "application/init/InitUseCase.h"

#include "domain/project/ProjectValidator.h"

InitUseCase::InitUseCase(
    const ProjectCreator &projectCreator,
    const MavenChecker &mavenChecker)
    : projectCreator_(projectCreator),
      mavenChecker_(mavenChecker)
{
}

InitUseCaseResult InitUseCase::execute(const InitUseCaseRequest &request) const
{
    ProjectValidator::validate(request.config);

    bool mavenMissing =
        request.skipWrapper &&
        !mavenChecker_.isMavenInstalled();

    projectCreator_.create(request.config, request.skipWrapper);

    return {mavenMissing};
}
