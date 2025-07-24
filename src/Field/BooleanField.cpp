#include "BooleanField.h"
#include <vector>
#include <algorithm>

class BooleanRule : public FieldRule {
public:
    bool apply(const std::optional<std::string>& value, std::string& error) const override {
        if (!value.has_value()) return true;
        static const std::vector<std::string> validValues = { "true", "false", "1", "0" };
        const auto& val = *value;
        if (std::find(validValues.begin(), validValues.end(), val) == validValues.end()) {
            error = "Value '" + val + "' is not a valid boolean.";
            return false;
        }
        return true;
    }
};

BooleanField::BooleanField(const BooleanFieldConfig& config)
    : Field(config) {
    addRule(std::make_unique<BooleanRule>());
}
