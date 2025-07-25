#pragma once

#include "FieldValue.h"
#include "ReferenceFieldSchema.h"
#include <string>
#include <optional>

class ReferenceFieldValue : public FieldValue {
public:
    explicit ReferenceFieldValue(const ReferenceFieldSchema& schema);

    bool setValueFromString(const std::string& value, std::string& error) override;
    bool validate(std::string& error) const override;

    // Get and set the referenced entity ID
    const std::optional<std::string>& getReferencedId() const;
    void setReferencedId(const std::string& id);

    std::string toString() const override;

private:
    const ReferenceFieldSchema& schema_;
    std::optional<std::string> referencedId_;
};
