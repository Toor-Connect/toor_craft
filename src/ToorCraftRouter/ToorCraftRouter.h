#pragma once
#include <string>

class ToorCraftRouter
{
public:
    static ToorCraftRouter &instance();

    std::string handleRequest(const std::string &jsonRequest);

private:
    ToorCraftRouter() = default;
    ToorCraftRouter(const ToorCraftRouter &) = delete;
    ToorCraftRouter &operator=(const ToorCraftRouter &) = delete;
};
