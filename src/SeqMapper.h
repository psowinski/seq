#pragma once
#include <functional>
#include <optional>
#include "ISeq.h"
#include "ISeqGroup.h"
#include "SeqForwardIterator.h"
using namespace std;

namespace seq
{
   template <typename T, typename U>
   class SeqMapper final : public ISeq<U>
   {
      shared_ptr<ISeq<T>> m_seq;
      function<U(const T&)> m_mapper;

   public:
      explicit SeqMapper(shared_ptr<ISeq<T>> seq, function<U(const T&)> mapper) : m_seq(move(seq)), m_mapper(move(mapper))
      {
         if (!m_seq)
            throw invalid_argument("seq");
         if (!m_mapper)
            throw invalid_argument("m_mapper");
      }

      void Reset() override { m_seq->Reset(); }

      optional<U> Next() override
      {
         auto next = m_seq->Next();
         if (next.has_value())
            return m_mapper(next.value());
         return nullopt;
      };
   };
}
