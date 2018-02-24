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
   class SeqFlattener final : public ISeq<T>
   {
      bool m_initialized;
      shared_ptr<ISeq<shared_ptr<ISeq<T>>>> m_seq;
      optional<shared_ptr<ISeq<T>>> m_current;

   public:
      explicit SeqFlattener(shared_ptr<ISeq<shared_ptr<ISeq<T>>>> seq) : m_initialized(false), m_seq(move(seq))
      {
         if (!m_seq)
            throw invalid_argument("seq");
      }

      void Reset() override
      {
         m_seq->Reset();
         m_initialized = false;
         m_current = nullopt;
      }

      optional<T> Next() override
      {
         if(!m_initialized)
         {
            m_initialized = true;
            m_current = m_seq->Next();
            m_current->Reset();
         }

         while (m_current.has_value())
         {
            auto next = m_current->Next();
            if (next.has_value())
               return next;
            m_current = m_seq->Next();
            m_current->Reset();
         }
         return nullopt;
      };
   };
}
