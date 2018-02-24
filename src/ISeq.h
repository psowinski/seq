#pragma once
#include <optional>
namespace seq
{
   template <typename T>
   class ISeq
   {
   public:
      virtual ~ISeq() = default;
      virtual void Reset() = 0;
      virtual optional<T> Next() = 0;
   };
}
   