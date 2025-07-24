#pragma once

#include "Field.h"
#include <optional>
#include <string>

struct StringFieldConfig : FieldConfig {
    virtual ~StringFieldConfig() = default;
};

class StringField : public Field {
public:
    explicit StringField(const StringFieldConfig& config);
};
