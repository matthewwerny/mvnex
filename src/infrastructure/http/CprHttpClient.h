#pragma once

#include "application/ports/HttpClient.h"

class CprHttpClient : public HttpClient
{
public:
    explicit CprHttpClient(int timeoutMilliseconds);

    HttpResponse get(const std::string &url) const override;

private:
    int timeoutMilliseconds_;
};
