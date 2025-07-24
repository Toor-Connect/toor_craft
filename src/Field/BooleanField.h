#pragma once

#include "Field.h"
#include <optional>


struct BooleanFieldConfig : FieldConfig {
    virtual ~BooleanFieldConfig() = default;
};

class BooleanField : public Field {
public:
    explicit BooleanField(const BooleanFieldConfig& config);
};
