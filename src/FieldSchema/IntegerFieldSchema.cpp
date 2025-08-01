#include "IntegerFieldSchema.h"
#include <charconv>
#include <nlohmann/json.hpp>

class IntegerRangeRuleSchema : public FieldRuleSchema
{
public:
    IntegerRangeRuleSchema(std::optional<int64_t> minVal, std::optional<int64_t> maxVal)
        : minValue_(minVal), maxValue_(maxVal) {}

    void apply(const std::optional<std::string> &value) const override
    {
        if (!value.has_value())
            return; // nothing to validate if no value provided

        const auto &valStr = *value;

        try
        {
            size_t pos;
            int64_t val = std::stoll(valStr, &pos);

            if (pos != valStr.size())
            {
                throw std::runtime_error("Value '" + valStr + "' contains invalid characters.");
            }

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
            throw std::runtime_error("Value '" + valStr + "' is not a valid integer.");
        }
        catch (const std::out_of_range &)
        {
            throw std::runtime_error("Value '" + valStr + "' is out of range for an integer.");
        }
    }

private:
    std::optional<int64_t> minValue_;
    std::optional<int64_t> maxValue_;
};

IntegerFieldSchema::IntegerFieldSchema(IntegerFieldSchemaConfig config)
    : FieldSchema(std::move(config)),
      minValue_(config.minValue),
      maxValue_(config.maxValue)
{

    addRule(std::make_unique<IntegerRangeRuleSchema>(minValue_, maxValue_));
}

std::string IntegerFieldSchema::toJson() const
{
    nlohmann::json j;
    j["type"] = "integer";
    if (minValue_)
        j["min"] = *minValue_;
    if (maxValue_)
        j["max"] = *maxValue_;
    j["required"] = isRequired();
    if (getAlias())
        j["alias"] = *getAlias();
    return j.dump();
}