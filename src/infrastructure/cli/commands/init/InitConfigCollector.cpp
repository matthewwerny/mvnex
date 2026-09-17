#include "infrastructure/cli/commands/init/InitConfigCollector.h"

#include "infrastructure/cli/arguments/ArgumentParser.h"
#include "infrastructure/cli/prompt/Prompt.h"
#include "domain/project/ProjectNaming.h"
#include "domain/project/ProjectValidator.h"

#include <string>

InitConfigCollector::InitConfigCollector(Prompt &prompt)
    : prompt_(prompt)
{
}

ProjectConfig InitConfigCollector::collectConfig(const ArgumentParser &parser) const
{
    std::string projectName = parser.getArgument(0);
    std::string groupId = parser.getOption("group-id");
    std::string packageName = parser.getOption("package");
    std::string javaVersion = parser.getOption("java");
    bool hasProjectNameArgument = !parser.getArguments().empty();

    if (!javaVersion.empty())
    {
        ProjectValidator::validateJavaVersion(javaVersion);
    }

    if (hasProjectNameArgument)
    {
        ProjectValidator::validateProjectName(projectName, javaVersion.empty() ? "21" : javaVersion);
    }

    if (!groupId.empty())
    {
        ProjectValidator::validateGroupId(groupId, javaVersion.empty() ? "21" : javaVersion);
    }

    if (!packageName.empty())
    {
        ProjectValidator::validateGroupId(packageName, javaVersion.empty() ? "21" : javaVersion);
    }

    if (!hasProjectNameArgument)
    {
        projectName = prompt_.text(
            "Project name",
            "my-project");

        if (projectName.empty())
        {
            return {};
        }
    }

    if (groupId.empty())
    {
        groupId = prompt_.text(
            "Group ID",
            "com.example");

        if (groupId.empty())
        {
            return {};
        }
    }

    if (javaVersion.empty())
    {
        javaVersion = prompt_.select(
            "Java version",
            {"8", "11", "17", "21", "25"},
            "21");

        if (javaVersion.empty())
        {
            return {};
        }
    }

    if (packageName.empty())
    {
        packageName =
            groupId + "." +
            ProjectNaming::toPackageName(projectName);
    }

    return {
        projectName,
        groupId,
        packageName,
        javaVersion};
}
