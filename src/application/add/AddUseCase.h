#pragma once
#include "domain/dependency/DependencyRequest.h"
#include "domain/dependency/ResolvedDependency.h"
#include "application/ports/MavenProjectValidator.h"

#include <vector>

struct AddUseCaseRequest
{
    std::vector<DependencyRequest> dependencies;
};

struct AddUseCaseResponse
{
    std::vector<ResolvedDependency> addedDependencies;
    std::vector<ResolvedDependency> skippedDependencies;
    MavenProjectValidationResult validationResult;
};

class DependencyResolver;
class ProjectDependencyRepository;
class MavenProjectValidator;

class AddUseCase{
    public:
        AddUseCase(
            const DependencyResolver &dependencyResolver,
            const ProjectDependencyRepository &projectDependencyRepository,
            const MavenProjectValidator &mavenProjectValidator
        );
        AddUseCaseResponse execute(const AddUseCaseRequest &request) const;

    private:
        const DependencyResolver &dependencyResolver_;
        const ProjectDependencyRepository &projectDependencyRepository_;
        const MavenProjectValidator &mavenProjectValidator_;
};
