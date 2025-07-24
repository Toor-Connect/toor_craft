#include "EnumFieldSchema.h"
#include <algorithm>

class EnumRuleSchema : public FieldRuleSchema
{
public:
    explicit EnumRuleSchema(std::vector<std::string> allowed) : allowedValues_(std::move(allowed)) {}

    bool apply(const std::optional<std::string> &value, std::string &error) const override
    {
        if (!value.has_value())
            return true;
        const auto &val = *value;
        if (std::find(allowedValues_.begin(), allowedValues_.end(), val) == allowedValues_.end())
        {
            error = "Value '" + val + "' is not allowed.";
            return false;
        }
        return true;
    }

private:
    std::vector<std::string> allowedValues_;
};

EnumFieldSchema::EnumFieldSchema(const EnumFieldSchemaConfig& config)
    : FieldSchema(config) {
    addRule(std::make_unique<EnumRuleSchema>(config.allowedValues));
}