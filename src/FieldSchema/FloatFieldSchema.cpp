#include "FloatFieldSchema.h"
#include <charconv>
#include <nlohmann/json.hpp>

class FloatRangeRuleSchema : public FieldRuleSchema
{
public:
    FloatRangeRuleSchema(std::optional<double> minVal, std::optional<double> maxVal)
        : minValue_(minVal), maxValue_(maxVal) {}

    void apply(const std::optional<std::string> &value) const override
    {
        if (!value.has_value())
            return; // nothing to validate if the field is empty

        const auto &valStr = *value;
        try
        {
            double val = std::stod(valStr);

            if (minValue_ && val < *minValue_)
            {
                throw std::runtime_error(
                    "Value " + valStr + " is less than minimum " + std::to_string(*minValue_));
            }

            if (maxValue_ && val > *maxValue_)
            {
                throw std::runtime_error(
                    "Value " + valStr + " exceeds maximum " + std::to_string(*maxValue_));
            }
        }
        catch (const std::invalid_argument &)
        {
            throw std::runtime_error("Value '" + valStr + "' is not a valid float.");
        }
        catch (const std::out_of_range &)
        {
            throw std::runtime_error("Value '" + valStr + "' is out of range for a float.");
        }
    }

private:
    std::optional<double> minValue_;
    std::optional<double> maxValue_;
};

FloatFieldSchema::FloatFieldSchema(FloatFieldSchemaConfig config)
    : FieldSchema(std::move(config)),
      minValue_(config.minValue),
      maxValue_(config.maxValue)
{

    addRule(std::make_unique<FloatRangeRuleSchema>(minValue_, maxValue_));
}

std::string FloatFieldSchema::toJson() const
{
    nlohmann::json j;
    j["type"] = "float";
    if (minValue_)
        j["min"] = *minValue_;
    if (maxValue_)
        j["max"] = *maxValue_;
    j["required"] = isRequired();
    if (getAlias())
        j["alias"] = *getAlias();
    return j.dump();
}
