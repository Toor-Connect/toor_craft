#pragma once

#include <string>
#include <optional>
#include <memory>
#include <vector>

class FieldRule
{
public:
    virtual ~FieldRule() = default;

    virtual bool apply(const std::optional<std::string> &value, std::string &error) const = 0;
};

// Base configuration struct for all fields
struct FieldConfig
{
    std::string name;
    bool required = false;
    std::optional<std::string> alias;

    virtual ~FieldConfig() = default;
};

// Abstract base class for all field types
class Field
{
public:
    explicit Field(const FieldConfig &config)
        : name_(config.name),
          required_(config.required),
          alias_(config.alias)
    {
    }

    Field(const Field &) = default;
    Field &operator=(const Field &) = default;
    Field(Field &&) = default;
    Field &operator=(Field &&) = default;

    virtual ~Field() = default;

    // Getters for field properties
    const std::string &getName() const { return name_; }
    bool isRequired() const { return required_; }
    const std::optional<std::string> &getAlias() const { return alias_; }

    // Add a rule to this field
    void addRule(std::unique_ptr<FieldRule> rule)
    {
        rules_.push_back(std::move(rule));
    }

    // Validate by applying all rules sequentially
    bool validate(const std::optional<std::string> &value, std::string &error) const
    {
        for (const auto &rule : rules_)
        {
            if (!rule->apply(value, error))
            {
                return false;
            }
        }
        return true;
    }

protected:
    std::string name_;
    bool required_;
    std::optional<std::string> alias_;

    std::vector<std::unique_ptr<FieldRule>> rules_;
};
