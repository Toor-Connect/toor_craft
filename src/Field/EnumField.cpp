#include "EnumField.h"
#include <algorithm>

class EnumRule : public FieldRule
{
public:
    explicit EnumRule(std::vector<std::string> allowed) : allowedValues_(std::move(allowed)) {}

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

EnumField::EnumField(const EnumFieldConfig& config)
    : Field(config) {
    addRule(std::make_unique<EnumRule>(config.allowedValues));
}