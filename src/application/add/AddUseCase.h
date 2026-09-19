#pragma once
#include "domain/dependency/DependencyRequest.h"
#include "domain/dependency/ResolvedDependency.h"

#include <vector>

struct AddUseCaseRequest
{
    std::vector<DependencyRequest> dependencies;
};

struct AddUseCaseResponse
{
    std::vector<ResolvedDependency> addedDependencies;
    std::vector<ResolvedDependency> skippedDependencies;
};

class DependencyResolver;
class ProjectDependencyRepository;

class AddUseCase{
    public:
        AddUseCase(
            const DependencyResolver &dependencyResolver,
            const ProjectDependencyRepository &projectDependencyRepository
        );
        AddUseCaseResponse execute(const AddUseCaseRequest &request) const;

    private:
        const DependencyResolver &dependencyResolver_;
        const ProjectDependencyRepository &projectDependencyRepository_;
};
