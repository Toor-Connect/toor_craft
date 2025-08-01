#pragma once

#include "FieldValue.h"
#include "ReferenceFieldSchema.h"
#include <string>
#include <optional>

class ReferenceFieldValue : public FieldValue
{
public:
    explicit ReferenceFieldValue(const ReferenceFieldSchema &schema);

    void setValueFromString(const std::string &value) override;
    void validate() const override;
    bool isEmpty() const override;

    const std::optional<std::string> &getReferencedId() const;
    void setReferencedId(const std::string &id);

    std::string toString() const override;
    std::string toJson() const override;

private:
    const ReferenceFieldSchema &schema_;
    std::optional<std::string> referencedId_;
};
