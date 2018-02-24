#pragma once
#include <functional>
#include <optional>
#include "ISeq.h"
#include "ISeqGroup.h"
#include "SeqForwardIterator.h"
using namespace std;

namespace seq
{
   template <typename T>
   class SeqFilter final : public ISeq<T>
   {
      shared_ptr<ISeq<T>> m_seq;
      function<bool(const T&)> m_predicate;

   public:
      explicit SeqFilter(shared_ptr<ISeq<T>> seq, function<bool(const T&)> predicate) : m_seq(move(seq)), m_predicate(move(predicate))
      {
         if (!m_seq)
            throw invalid_argument("seq");
         if (!m_predicate)
            throw invalid_argument("predicate");
      }

      void Reset() override { m_seq->Reset(); }

      optional<T> Next() override
      {
         do
         {
            auto next = m_seq->Next();
            if (!next.has_value() || m_predicate(next.value()))
               return next;
         } while (true);
      };
   };
}
