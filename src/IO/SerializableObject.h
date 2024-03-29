#pragma once

#include "IO/JsonParser.h"

namespace omp {
class SerializableObject {
private:
  uint64_t m_SerializationId;

public:
  SerializableObject() = default;
  virtual void serialize(JsonParser<> &parser) = 0;
  virtual void deserialize(JsonParser<> &parser) = 0;
  virtual ~SerializableObject() = default;

  virtual void serializeDependency(SerializableObject* object)
  {
      object->SerializeId();
  }
private:
  void SerializeId()
  {
    // Add id throught asset metadata
  }
};
} // namespace omp
