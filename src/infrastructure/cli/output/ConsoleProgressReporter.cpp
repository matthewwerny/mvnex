#include "infrastructure/cli/output/ConsoleProgressReporter.h"

#include "infrastructure/cli/output/Style.h"

#include <chrono>
#include <future>
#include <iostream>
#include <thread>

void ConsoleProgressReporter::run(
    const std::string &message,
    const std::function<void()> &operation) const
{
    auto task = std::async(
        std::launch::async,
        [&operation]()
        {
            operation();
        });

    const char frames[] = {'|', '/', '-', '\\'};
    std::size_t frame = 0;

    while (task.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready)
    {
        std::cout
            << "\r  "
            << Style::CYAN
            << frames[frame++ % 4]
            << Style::RESET
            << " "
            << message
            << std::flush;
    }

    task.get();

    std::cout
        << "\r  "
        << Style::CYAN
        << "✓"
        << Style::RESET
        << " "
        << message
        << "     "
        << '\n';
}
