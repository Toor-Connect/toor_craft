#include "BooleanFieldSchema.h"
#include <vector>
#include <algorithm>

class BooleanRuleSchema : public FieldRuleSchema
{
public:
    void apply(const std::optional<std::string> &value) const override
    {
        if (!value.has_value())
            return; // no value means nothing to validate

        static const std::vector<std::string> validValues = {"true", "false", "1", "0"};
        const auto &val = *value;

        if (std::find(validValues.begin(), validValues.end(), val) == validValues.end())
        {
            throw std::runtime_error("Value '" + val + "' is not a valid boolean.");
        }
    }
};

BooleanFieldSchema::BooleanFieldSchema(BooleanFieldSchemaConfig config)
    : FieldSchema(std::move(config))
{
}
