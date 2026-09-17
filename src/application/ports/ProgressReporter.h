#pragma once

#include <functional>
#include <string>

class ProgressReporter
{
public:
    virtual ~ProgressReporter() = default;

    virtual void run(const std::string &message, const std::function<void()> &operation) const = 0;
};

class NoopProgressReporter : public ProgressReporter
{
public:
    void run(const std::string &, const std::function<void()> &operation) const override
    {
        operation();
    }
};
