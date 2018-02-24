#pragma once
#include <optional>
#include "ISeq.h"
namespace seq
{
   template <typename Key, typename T>
   class ISeqGroup final : public ISeq<T>
   {
   public:
      virtual ~ISeqGroup() = default;
      virtual Key GetKey() = 0;
   };
}
   