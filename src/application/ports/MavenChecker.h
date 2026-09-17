#pragma once

class MavenChecker
{
public:
    virtual ~MavenChecker() = default;

    virtual bool isMavenInstalled() const = 0;
};
