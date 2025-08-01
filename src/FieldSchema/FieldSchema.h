#pragma once

#include <string>
#include <optional>
#include <memory>
#include <vector>

class FieldRuleSchema
{
public:
    virtual ~FieldRuleSchema() = default;

    virtual void apply(const std::optional<std::string> &value) const = 0;
};

// Base configuration struct for all fields
struct FieldSchemaConfig
{
    std::string name;
    bool required = false;
    std::optional<std::string> alias;

    virtual ~FieldSchemaConfig() = default;
};

// Abstract base class for all field types
class FieldSchema
{
public:
    explicit FieldSchema(FieldSchemaConfig config)
        : name_(config.name),
          required_(config.required),
          alias_(config.alias),
          config_(std::move(config))
    {
    }

    FieldSchema(const FieldSchema &) = delete;
    FieldSchema &operator=(const FieldSchema &) = delete;
    FieldSchema(FieldSchema &&) = default;
    FieldSchema &operator=(FieldSchema &&) = default;

    virtual ~FieldSchema() = default;

    // Getters for field properties
    const std::string &getName() const { return name_; }
    bool isRequired() const { return required_; }
    const std::optional<std::string> &getAlias() const { return alias_; }
    virtual std::string getTypeName() const = 0;
    const FieldSchemaConfig &getConfig() const { return config_; }

    // Add a rule to this field
    void addRule(std::unique_ptr<FieldRuleSchema> rule)
    {
        rules_.push_back(std::move(rule));
    }

    void validate(const std::optional<std::string> &value) const
    {
        for (const auto &rule : rules_)
        {
            rule->apply(value); // Will throw std::runtime_error if validation fails
        }
    }

    virtual std::string toJson() const = 0;

protected:
    std::string name_;
    bool required_;
    std::optional<std::string> alias_;
    FieldSchemaConfig config_;
    std::vector<std::unique_ptr<FieldRuleSchema>> rules_;
};
