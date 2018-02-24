#pragma once
#include <functional>
#include <optional>
#include "ISeq.h"
#include "ISeqGroup.h"
#include "SeqForwardIterator.h"
using namespace std;

namespace seq
{
   template <typename T, typename Key>
   class SeqGroup final : public ISeq<T>
   {
      bool m_initialized;
      vector<T> m_items;
      typename vector<T>::iterator m_current;
      Key m_key;

   public:
      explicit SeqGroup(Key key) : m_initialized(false), m_key(move(key))
      {
      }

      void Reset() override
      {
         m_initialized = false;
      }

      optional<T> Next() override
      {
         if(!m_initialized)
         {
            m_initialized = true;
            m_current = m_items.begin();
         }
         else if (m_current != m_items.end())
         {
            ++m_current;
         }
         if (m_current == m_items.end())
            return nullopt;
         return *m_current;
      }

      Key GetKey() override { return m_key; };
      void AddItem(const T& item)
      {
         if(m_initialized)
            throw invalid_argument("item cannot be added after initialization");
         m_items.push_back(item);
      }
   };

   template <typename Key, typename T>
   class SeqGrouper final : public ISeq<ISeqGroup<Key, T>>
   {
      map<Key, shared_ptr<SeqGroup<Key, T>>> m_items;
      typename map<Key, shared_ptr<SeqGroup<Key, T>>>::iterator m_current;
      bool m_initialized;
      shared_ptr<ISeq<T>> m_seq;
      function<Key(const T&)> m_getkey;
   public:
      explicit SeqGrouper(shared_ptr<ISeq<T>> seq, function<Key(const T&)> getkey) : m_initialized(false), m_seq(move(seq)), m_getkey(move(getkey))
      {
         if (!m_seq)
            throw invalid_argument("seq");
         if (!m_getkey)
            throw invalid_argument("getkey");

         Group();
      }

      void Reset() override
      {
         m_initialized = false;
      }

      optional<ISeqGroup<Key, T>> Next() override
      {
         if (!m_initialized)
         {
            m_initialized = true;
            m_current = m_items.begin();
         }
         else if (m_current != m_items.end())
         {
            ++m_current;
         }
         if (m_current == m_items.end())
            return nullopt;
         return *m_current;
      }

   private:
      void Group()
      {
         for (T&& item : m_seq)
         {
            Key key = m_getkey(item);
            auto&& it = m_items.find(key);
            shared_ptr<SeqGroup<Key, T>> group;
            if (it != m_items.end())
            {
               group = *it;
            }
            else
            {
               group = make_shared<SeqGroup<Key, T>>(key);
               m_items.insert(make_pair<Key, shared_ptr<SeqGroup<Key, T>>>(key, group));
            }
            group->AddItem(item);
         }
      }
   };
}
