#include "FloatFieldSchema.h"
#include <charconv>

class FloatRangeRuleSchema : public FieldRuleSchema
{
public:
    FloatRangeRuleSchema(std::optional<double> minVal, std::optional<double> maxVal)
        : minValue_(minVal), maxValue_(maxVal) {}

    bool apply(const std::optional<std::string> &value, std::string &error) const override
    {
        if (!value.has_value())
            return true;
        const auto &valStr = *value;
        try
        {
            double val = std::stod(valStr);
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
            error = "Value '" + valStr + "' is not a valid float.";
            return false;
        }
        return true;
    }

private:
    std::optional<double> minValue_;
    std::optional<double> maxValue_;
};

FloatFieldSchema::FloatFieldSchema(const FloatFieldSchemaConfig& config)
    : FieldSchema(config) {
    addRule(std::make_unique<FloatRangeRuleSchema>(config.minValue, config.maxValue));
}