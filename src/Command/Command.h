#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Entity; // Forward declare your Entity class

// Base config for commands
struct CommandConfig
{
    std::string id;
    std::string type;
    virtual ~CommandConfig() = default;
};

class Command
{
public:
    explicit Command(CommandConfig config)
        : id_(std::move(config.id)), type_(std::move(config.type))
    {
    }
    virtual ~Command() = default;

    const std::string &getId() const { return id_; }
    const std::string &getType() const { return type_; }
    virtual void execute(const Entity &entity) const = 0;

protected:
    std::string id_;
    std::string type_;
};
