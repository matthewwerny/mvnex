#pragma once

#include <string>

struct HttpResponse
{
    int statusCode;
    std::string body;
};

class HttpClient
{
public:
    virtual ~HttpClient() = default;

    virtual HttpResponse get(const std::string &url) const = 0;
};
