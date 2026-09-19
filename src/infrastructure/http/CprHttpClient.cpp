#include "infrastructure/http/CprHttpClient.h"

#include "application/errors/DependencyResolverUnavailable.h"

#include <cpr/cpr.h>

CprHttpClient::CprHttpClient(int timeoutMilliseconds)
    : timeoutMilliseconds_(timeoutMilliseconds)
{
}

HttpResponse CprHttpClient::get(const std::string &url) const
{
    cpr::Response response = cpr::Get(
        cpr::Url{url},
        cpr::UserAgent{"mvnex"},
        cpr::Redirect{true},
        cpr::Timeout{timeoutMilliseconds_});

    if (response.error)
    {
        throw DependencyResolverUnavailable(response.error.message);
    }

    return HttpResponse{
        static_cast<int>(response.status_code),
        response.text};
}
