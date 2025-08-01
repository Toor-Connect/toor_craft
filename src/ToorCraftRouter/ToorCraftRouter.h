#pragma once
#include <string>

class ToorCraftRouter
{
public:
    static std::string handleRequest(const std::string &jsonRequest);
};