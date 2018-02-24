#pragma once
#include "../../../Dependencies/msxml6.tlh"
#include "ISeq.h"

namespace seq
{
   class XmlElement final
   {
      MSXML2::IXMLDOMElementPtr m_element;
   public:
      XmlElement() = default;
      ~XmlElement();

      explicit XmlElement(const MSXML2::IXMLDOMElementPtr& element) : m_element(element)
      {
      }

      XmlElement(const XmlElement& other) : m_element(other.Src())
      {
      }

      XmlElement(XmlElement&& other) noexcept
      {
         m_element = other.m_element;
         other.m_element = nullptr;
      }

      XmlElement& operator =(const XmlElement& other)
      {
         m_element = other.Src();
         return *this;
      }

      XmlElement& operator =(XmlElement&& other) noexcept
      {
         if (this != &other)
         {
            m_element = other.m_element;
            other.m_element = nullptr;
         }
         return *this;
      }

      bool operator !=(const XmlElement& other) const
      {
         return m_element != other.Src();
      }

      bstr_t GetName() const
      {
         bstr_t value;
         if (m_element)
            m_element->get_baseName(value.GetAddress());
         return value;
      }

      bstr_t GetAttrOrEmpty(const bstr_t& name) const
      {
         auto attr = GetAttributeVariant(name);
         if (attr.vt != VT_NULL && attr.vt != VT_EMPTY)
            return static_cast<bstr_t>(attr);
         return bstr_t(L"");
      }

      std::optional<bstr_t> GetAttribute(const bstr_t& name) const
      {
         auto attr = GetAttributeVariant(name);
         if(attr.vt != VT_NULL && attr.vt != VT_EMPTY)
            return static_cast<bstr_t>(attr);
         return std::nullopt;
      }

      variant_t GetAttributeVariant(const bstr_t& name) const
      {
         variant_t value;
         if (m_element)
            m_element->getAttribute(name, value.GetAddress());
         return value;
      }

      optional<XmlElement> GetParent() const
      {
         MSXML2::IXMLDOMNodePtr parent;
         MSXML2::IXMLDOMElementPtr parentElement;
         if (m_element && SUCCEEDED(m_element->get_parentNode(&parent)) && (parentElement = parent) != nullptr)
            return XmlElement(parentElement);
         return nullopt;
      }

      MSXML2::IXMLDOMElementPtr Src() const { return m_element; }

      shared_ptr<ISeq<XmlElement>> GetChildrenEnumerator() const;
      shared_ptr<ISeq<XmlElement>> GetDescendantsByNameEnumerator(const bstr_t& name) const;
      shared_ptr<ISeq<pair<bstr_t, variant_t>>> GetAttributesEnumerator() const;
   };

   inline XmlElement::~XmlElement()
   {
      m_element = nullptr;
   }

   class XmlSeqChildrenNodes final : public ISeq<MSXML2::IXMLDOMNodePtr>
   {
      friend class XmlSeqChildrenNodes;

      bool m_initialized;
      MSXML2::IXMLDOMNodePtr m_parent;
      MSXML2::IXMLDOMNodePtr m_current;

   public:
      explicit XmlSeqChildrenNodes(const MSXML2::IXMLDOMNodePtr& parent)
      {
         m_parent = parent;
         m_current = nullptr;
         m_initialized = false;
      }

      void Reset() override
      {
         m_initialized = false;
         m_current = nullptr;
      }

      optional<MSXML2::IXMLDOMNodePtr> Next() override
      {
         if (!m_initialized && m_parent)
         {
            m_initialized = true;
            m_parent->get_firstChild(&m_current);
         }
         else if (m_current != nullptr)
         {
            MSXML2::IXMLDOMNodePtr sibling;
            if (FAILED(m_current->get_nextSibling(&sibling)))
               m_current = nullptr;
            else
               m_current = sibling;
         }

         if (m_current != nullptr)
            return m_current;
         return nullopt;
      }
   };

   class XmlSeqChildrenElements final : public ISeq<XmlElement>
   {
      XmlSeqChildrenNodes m_nodesSeq;
   public:
      explicit XmlSeqChildrenElements(const MSXML2::IXMLDOMNodePtr& parent) : m_nodesSeq(parent)
      {
      }

      void Reset() override
      {
         m_nodesSeq.Reset();
      }

      optional<XmlElement> Next() override
      {
         MSXML2::IXMLDOMElementPtr element;
         optional<MSXML2::IXMLDOMNodePtr> next;
         do
         {
            next = m_nodesSeq.Next();
            if (next.has_value() && (element = next.value()) != nullptr)
               return XmlElement(element);

         } while (next.has_value());

         return nullopt;
      }
   };

   class XmlSeqElementsByName final : public ISeq<XmlElement>
   {
      bool m_initialized;
      MSXML2::IXMLDOMElementPtr m_parent;
      MSXML2::IXMLDOMNodeListPtr m_nodes;
      MSXML2::IXMLDOMElementPtr m_current;
      bstr_t m_tag;

   public:
      explicit XmlSeqElementsByName(const MSXML2::IXMLDOMElementPtr& parent, const bstr_t& tag)
      {
         m_parent = parent;
         m_tag = tag;

         m_nodes = nullptr;
         m_current = nullptr;
         m_initialized = false;
      }

      void Reset() override
      {
         m_nodes = nullptr;
         m_current = nullptr;
         m_initialized = false;
      }

      optional<XmlElement> Next() override
      {
         if (!m_initialized && m_parent)
         {
            m_initialized = true;
            PrepareElements();
            NextNode();
         }
         else if (m_current != nullptr)
         {
            NextNode();
         }

         if (m_current != nullptr)
            return XmlElement(m_current);
         return nullopt;
      }

   private:
      void PrepareElements()
      {
         if (m_parent)
            m_parent->getElementsByTagName(m_tag, &m_nodes);
      }

      void NextNode()
      {
         MSXML2::IXMLDOMNodePtr next;
         if (m_nodes && FAILED(m_nodes->nextNode(&next)))
            m_current = nullptr;
         else
            m_current = next;
      }
   };

   class XmlSeqAttributes final : public ISeq<pair<bstr_t, variant_t>>
   {
      bool m_initialized;
      MSXML2::IXMLDOMNodePtr m_node;
      MSXML2::IXMLDOMNamedNodeMapPtr m_attributes;
      MSXML2::IXMLDOMAttributePtr m_current;

   public:
      explicit XmlSeqAttributes(const MSXML2::IXMLDOMNodePtr& node)
      {
         m_node = node;
         m_current = nullptr;
         m_initialized = false;
      }

      void Reset() override
      {
         m_initialized = false;
         m_attributes = nullptr;
         m_current = nullptr;
      }

      optional<pair<bstr_t, variant_t>> Next() override
      {
         if (!m_initialized)
         {
            m_initialized = true;
            PrepareAttributes();
            NextNode();
         }
         else if (m_current != nullptr)
         {
            NextNode();
         }
         if (m_current != nullptr)
            return CurrentToPair();
         return nullopt;
      }

   private:
      void PrepareAttributes()
      {
         if (m_node)
            m_node->get_attributes(&m_attributes);
      }

      void NextNode()
      {
         MSXML2::IXMLDOMNodePtr attrNode;
         if (m_attributes && FAILED(m_attributes->nextNode(&attrNode)))
            m_current = nullptr;
         else
            m_current = attrNode;
      }

      pair<bstr_t, variant_t> CurrentToPair() const
      {
         if (m_current)
         {
            bstr_t name;
            variant_t value;
            if (SUCCEEDED(m_current->get_name(name.GetAddress()))
               && SUCCEEDED(m_current->get_value(value.GetAddress())))
               return make_pair(name, value);
         }
         return pair<bstr_t, variant_t>(); 
      }
   };

   inline shared_ptr<ISeq<XmlElement>> XmlElement::GetChildrenEnumerator() const
   {
      return make_shared<XmlSeqChildrenElements>(m_element);
   }

   inline shared_ptr<ISeq<XmlElement>> XmlElement::GetDescendantsByNameEnumerator(const bstr_t& name) const
   {
      return make_shared<XmlSeqElementsByName>(m_element, name);
   }

   inline shared_ptr<ISeq<pair<bstr_t, variant_t>>> XmlElement::GetAttributesEnumerator() const
   {
      return make_shared<XmlSeqAttributes>(m_element);
   }
}
