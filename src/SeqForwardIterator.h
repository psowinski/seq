#pragma once
#include "ISeq.h"

namespace seq
{
   template <typename T>
   class SeqForwardIterator final
   {
      shared_ptr<ISeq<T>> m_seq;
      optional<T> m_value;

   public:
      explicit SeqForwardIterator<T>(shared_ptr<ISeq<T>> seq) : m_seq(move(seq))
      {
         if (!m_seq)
            throw invalid_argument("seq");
      }

      bool operator !=(const SeqForwardIterator<T>& iterator)
      {
         return m_seq != iterator.Seq()
            || HasValue() != iterator.HasValue()
            || (HasValue() && Value() != iterator.Value());
      }

      SeqForwardIterator<T>& operator++()
      {
         m_value = m_seq->Next();
         return *this;
      }

      const T& operator *() const
      {
         return Value();
      }

      bool HasValue() const
      {
         return m_value.has_value();
      }

      const T& Value() const
      {
         return m_value.value();
      }

      shared_ptr<ISeq<T>> Seq() const
      {
         return m_seq;
      }
   };

   template<typename T> SeqForwardIterator<T> begin(shared_ptr<ISeq<T>> seq)
   {
      if(seq) seq->Reset();
      SeqForwardIterator iterator(seq);
      return ++iterator;
   }

   template<typename T> SeqForwardIterator<T> end(shared_ptr<ISeq<T>> seq)
   {
      SeqForwardIterator iterator(seq);
      return iterator;
   }
}
