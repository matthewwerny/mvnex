#pragma once

enum class MavenProjectValidationStatus
{
    Passed,
    Failed,
    MavenNotFound
};

struct MavenProjectValidationResult
{
    MavenProjectValidationStatus status;
};

class MavenProjectValidator
{
public:
    virtual ~MavenProjectValidator() = default;

    virtual MavenProjectValidationResult validate() const = 0;
};
