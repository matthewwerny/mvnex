#pragma once

#include "application/ports/MavenChecker.h"

class LocalMavenChecker : public MavenChecker
{
public:
    bool isMavenInstalled() const override;
};
