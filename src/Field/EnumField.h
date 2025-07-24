#pragma once

#include "Field.h"
#include <string>
#include <vector>

struct EnumFieldConfig : FieldConfig {
    std::vector<std::string> allowedValues;

    virtual ~EnumFieldConfig() = default;
};

class EnumField : public Field {
public:
    explicit EnumField(const EnumFieldConfig& config);

private:
    std::vector<std::string> allowedValues_;
};