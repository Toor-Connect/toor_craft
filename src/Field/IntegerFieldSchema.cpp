#include "IntegerFieldSchema.h"
#include <charconv>

class IntegerRangeRuleSchema : public FieldRuleSchema
{
public:
    IntegerRangeRuleSchema(std::optional<int64_t> minVal, std::optional<int64_t> maxVal)
        : minValue_(minVal), maxValue_(maxVal) {}

    bool apply(const std::optional<std::string> &value, std::string &error) const override
    {
        if (!value.has_value())
            return true;
        const auto &valStr = *value;
        try
        {
            size_t pos;
            int64_t val = std::stoll(valStr, &pos);
            if (pos != valStr.size())
            {
                error = "Value '" + valStr + "' contains invalid characters.";
                return false;
            }
            if (minValue_ && val < *minValue_)
            {
                error = "Value " + valStr + " is less than minimum " + std::to_string(*minValue_);
                return false;
            }
            if (maxValue_ && val > *maxValue_)
            {
                error = "Value " + valStr + " exceeds maximum " + std::to_string(*maxValue_);
                return false;
            }
        }
        catch (...)
        {
            error = "Value '" + valStr + "' is not a valid integer.";
            return false;
        }
        return true;
    }

private:
    std::optional<int64_t> minValue_;
    std::optional<int64_t> maxValue_;
};

IntegerFieldSchema::IntegerFieldSchema(const IntegerFieldSchemaConfig& config)
    : FieldSchema(config) {
    addRule(std::make_unique<IntegerRangeRuleSchema>(config.minValue, config.maxValue));
}