#pragma once

#include "application/ports/ProgressReporter.h"

class ConsoleProgressReporter : public ProgressReporter
{
public:
    void run(const std::string &message, const std::function<void()> &operation) const override;
};
