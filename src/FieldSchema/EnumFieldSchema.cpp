#include "EnumFieldSchema.h"
#include <algorithm>

class EnumRuleSchema : public FieldRuleSchema
{
public:
    explicit EnumRuleSchema(std::vector<std::string> allowed) : allowedValues_(std::move(allowed)) {}

    void apply(const std::optional<std::string> &value) const override
    {
        if (!value.has_value())
            return; // no value means no validation needed

        const auto &val = *value;

        if (std::find(allowedValues_.begin(), allowedValues_.end(), val) == allowedValues_.end())
        {
            throw std::runtime_error("Value '" + val + "' is not allowed.");
        }
    }

private:
    std::vector<std::string> allowedValues_;
};

EnumFieldSchema::EnumFieldSchema(EnumFieldSchemaConfig config)
    : FieldSchema(std::move(config)),
      allowedValues_(std::move(config.allowedValues))
{
}
