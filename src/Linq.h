#pragma once
#include <functional>
#include <optional>
#include "ISeq.h"
#include "ISeqGroup.h"
#include "SeqForwardIterator.h"
#include "SeqFilter.h"
#include "SeqMapper.h"
#include "SeqFlattener.h"
#include "SeqGrouper.h"
using namespace std;

namespace seq
{
   template <typename T>
   class Linq final
   {
      shared_ptr<ISeq<T>> m_seq;
   public:
      explicit Linq(shared_ptr<ISeq<T>> seq) : m_seq(move(seq))
      {
         if (!m_seq)
            throw invalid_argument("seq");
      }
      
      Linq<T> Filter(function<bool(const T&)> predicate)
      {
         return Linq<T>(make_shared<SeqFilter<T>>(m_seq, predicate));
      }

      template <typename Key> Linq<ISeqGroup<Key, T>> Group(function<Key(const T&)> getkey)
      {
         return Linq<ISeqGroup<Key, T>>(make_shared<SeqGrouper<Key, T>>(m_seq, getkey));
      }

      template <typename U> Linq<U> Map(function<U(const T&)> mapper)
      {
         return Linq<U>(make_shared<SeqMapper<T, U>>(m_seq, mapper));
      }

      template <typename U> Linq<U> Choose(function<optional<U>(const T&)> chooser)
      {
         function<U(const optional<U>&)> getValue = [](const optional<U>& x) { return x.value(); };
         return Map<optional<U>>(chooser)
            .Filter([](const optional<U>& x) { return x.has_value(); })
            .Map(getValue);
      }

      template <typename U> Linq<U> Flatten()
      {
         return Linq<U>::On(make_shared<SeqFlattener<T>>(m_seq));
      }

      int Count()
      {
         return Reduce<int>([](const T&, int sum) {return ++sum;}, 0);
      }

      optional<T> First()
      {
         auto beg = begin(m_seq);
         if (beg != end(m_seq))
            return *beg;
         return nullopt;
      }

      optional<T> First(function<bool(const T&)> predicate)
      {
         return Filter(predicate).First();
      }

      bool Any()
      {
         return begin(m_seq) != end(m_seq);
      }

      bool Any(function<bool(const T&)> predicate)
      {
         return Filter(predicate).Any();
      }

      template <typename Acc> Acc Reduce(function<Acc(const T&, Acc)> reducer, Acc zero = Acc())
      {
         Acc accumulator = move(zero);
         for(auto&& x : m_seq)
            accumulator = move(reducer(x, move(accumulator)));
         return accumulator;
      }

      shared_ptr<ISeq<T>> Seq() { return m_seq; }
   };
}
