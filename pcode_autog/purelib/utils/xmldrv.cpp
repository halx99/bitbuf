//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fstream>
#include <iostream>
//#include <cocos2d.h>
#include "super_cast.h"
#include "object_pool.h"
#include "xxfsutility.h"
#include "xmldrv.h"
#include "crypto_wrapper.h"

using namespace xmldrv;

// TDGame key
// static const char* xmldrvkey = "wvZ5MsscMVV4ZCgRQFqxIiQZdRnEl,pQ";

// Editor key
static const char* xmldrvkey = "gsvEjsNq2BhmwXMLR,8eGHdBAzWW8kN,";
static bool security_used = false;

void xmldrv::set_security(bool security)
{
    security_used = security;
}

/*************************common impl******************/
#ifndef _IMPL_COMM
#define _IMPL_COMM 1
#endif
#ifdef _IMPL_COMM
element::element(xml4wNodePtr _Ptr) : _Mynode(_Ptr)
{
}

element::~element(void)
{
}

element element::operator[](int index) const
{
    return this->get_child(index);
}

element element::get_child(const char* name, int index) const
{
    auto ptr = *this;
    __xml4wts_algo_cond(ptr,
        ptr.get_first_child(),
        ptr.get_next_sibling(),
        name == ptr.get_name() && (0 == index--),
        break
        );
    return ptr;
}

element element::get_child(int index) const
{
    auto ptr = *this;
    __xml4wts_algo_cond(ptr,
        ptr.get_first_child(),
        ptr.get_next_sibling(),
        (0 == index--),
        break
        );
    return ptr;
}

element element::add_child(const element& e)
{
    if (is_good() && e.is_good()) {
        return this->add_child(e.get_name().c_str(), e.get_value("").c_str());
    }
    return nullptr;
}

void element::remove_child(int index)
{
    this->get_child(index).remove_self();
}

void element::remove_child(const char* name, int index)
{
    this->get_child(name, index).remove_self();
}

std::string element::get_value(const std::string& default_value) const
{
    return this->get_value(default_value.c_str());
}

std::string element::get_attribute_value(const char* name, const std::string& default_value) const
{
    return this->get_attribute_value(name, default_value.c_str());
}

void element::set_value(const std::string& value)
{
    this->set_value(value.c_str());
}

document::document(void)
    : impl_(nullptr)
{
}

document::document(const char* filename)
    : impl_(nullptr)
{
    this->open(filename);
}

document::document(const char* filename, const char* rootname)
    : impl_(nullptr)
{
    this->open(filename, rootname);
}

document::document(const char* memory, int length)
    : impl_(nullptr)
{
    this->open(memory, length);
}

document::~document(void)
{
    this->close();
}

bool document::is_open(void) const
{
    return impl_ != nullptr;
}

#endif /* end of common impl */

/*************************rapidxml wrapper impl*****************************/
#ifdef _USING_RAPIDXML 
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_iterators.hpp"
#include "rapidxml/rapidxml_print.hpp"

inline rapidxml::xml_node<>* detail(void* raw)
{
    return (rapidxml::xml_node<>*)raw;
}
inline rapidxml::xml_node<>* detail(const element& elem)
{
    return detail(static_cast<void*>(elem));
}
inline bool is_element(xml4wNodePtr ptr)
{
    return detail(ptr)->type() == rapidxml::node_element;
}

element element::clone(void) const
{
    if(is_good())
    {
        auto doc = detail(_Mynode)->document();
        if (doc != nullptr)
            return doc->clone_node(detail(_Mynode));
    }
    return nullptr;
}

element element::get_parent(void) const
{
    if (is_good())
        return detail(_Mynode)->parent();
    return nullptr;
}

element element::get_first_child(void) const
{
    auto ptr = detail(_Mynode);
    __xml4wts_algo_cond(ptr,
        ptr->first_node(),
        ptr->next_sibling(),
        is_element(_Mynode),
        break
        );
    return ptr;
}

element element::get_prev_sibling(void) const
{
    auto ptr = detail(_Mynode);
    __xml4wts_algo_cond(ptr,
        ptr->previous_sibling(),
        ptr->previous_sibling(),
        is_element(_Mynode),
        break
        );
    return ptr;
}

element element::get_next_sibling(void) const
{
    auto ptr = detail(_Mynode);
    __xml4wts_algo_cond(ptr,
        ptr->next_sibling(),
        ptr->next_sibling(),
        is_element(_Mynode),
        break
        );
    return ptr;
}

std::string element::get_name(void) const
{
    if(is_good()) {
        return std::string(detail(_Mynode)->name(), detail(_Mynode)->name_size());
    }
    return "null";
}

std::string element::get_value(const char* default_value) const
{
    if (is_good()) {
        return std::string(detail(_Mynode)->value(), detail(_Mynode)->value_size());
    }

    std::cerr << "xml4w::element::get_value failed, detail(element:" << this->get_name() << "), use the default value["
        << default_value << "] insteaded!\n";

    return default_value;
}

std::string element::get_attribute_value(const char* name, const char* default_value) const
{
    if(is_good())
    {
        auto attr = detail(_Mynode)->first_attribute(name);
        if(attr != nullptr) {
            return std::string(attr->value(), attr->value_size());
        }
    }

    std::cerr << "xml4w::element::get_attribute_value failed, detail(element:" << this->get_name() << ",property:" << name << "), use the default value["
        << default_value << "] insteaded!\n";

    return default_value;
}

void element::set_value(const char* value)
{
    if (is_good())
    {
        auto doc = detail(_Mynode)->document();
        auto parent = detail(_Mynode)->parent();
        detail(_Mynode)->value(doc->allocate_string(value), strlen(value));
    }
}

#ifdef _USE_IN_COCOS2DX

void element::set_attribute_value(const char* name, const cocos2d::Color3B& value)
{
	char svalue[128] = { 0 };
	sprintf(svalue, "%u,%u,%u", (unsigned int)value.r, (unsigned int)value.g, (unsigned int)value.b);
	this->set_attribute_value(name, svalue);
}

void element::set_attribute_value(const char* name, const cocos2d::Color4B& value)
{
	char svalue[128] = { 0 };
	sprintf(svalue, "%u,%u,%u,%u", (unsigned int)value.r, (unsigned int)value.g, (unsigned int)value.b, (unsigned int)value.a);
	set_attribute_value(name, svalue);
}

void  element::set_attribute_value(const char* name, const cocos2d::Color4F& value)
{
	char svalue[128] = { 0 };
	sprintf(svalue, "%.3f,%.3f,%.3f,%.3f", value.r, value.g, value.b, value.a);
	set_attribute_value(name, svalue);
}


void  element::set_attribute_value(const char* name, const cocos2d::Rect& value)
{
	char svalue[128] = { 0 };
	sprintf(svalue, "%.3f,%.3f,%.3f,%.3f", value.origin.x, value.origin.y, value.size.width, value.size.height);
	set_attribute_value(name, svalue);
}

void element::set_attribute_value(const char* name, const cocos2d::Vec2& value)
{
	char svalue[128] = { 0 };
	sprintf(svalue, "%.3f,%.3f", value.x, value.y);
	set_attribute_value(name, svalue);
}
void element::set_attribute_value(const char* name, const cocos2d::Size& value)
{
	char svalue[128] = { 0 };
	sprintf(svalue, "%.3f,%.3f", value.width, value.height);
	set_attribute_value(name, svalue);
}

cocos2d::Color3B element::get_attribute_value(const char* name, const cocos2d::Color3B& default_value) const
{
	if (is_good())
	{
		auto attr = detail(_Mynode)->first_attribute(name);
		if (attr != nullptr) {
			auto value = nsc::parse3i(std::string(attr->value(), attr->value_size()), ',');
			return cocos2d::Color3B(std::get<0>(value), std::get<1>(value), std::get<2>(value));
		}
	}

	return default_value;
}
cocos2d::Color4B element::get_attribute_value(const char* name, const cocos2d::Color4B& default_value) const
{
	if (is_good())
	{
		auto attr = detail(_Mynode)->first_attribute(name);
		if (attr != nullptr) {
			auto value = nsc::parse4i(std::string(attr->value(), attr->value_size()), ',');
			return cocos2d::Color4B(std::get<0>(value), std::get<1>(value), std::get<2>(value), std::get<3>(value));
		}
	}

	return default_value;
}
cocos2d::Color4F element::get_attribute_value(const char* name, const cocos2d::Color4F& default_value) const
{
	if (is_good())
	{
		auto attr = detail(_Mynode)->first_attribute(name);
		if (attr != nullptr) {
			auto value = nsc::parse4f(std::string(attr->value(), attr->value_size()), ',');
			return cocos2d::Color4F(std::get<0>(value), std::get<1>(value), std::get<2>(value), std::get<3>(value));
		}
	}

	return default_value;
}
cocos2d::Rect    element::get_attribute_value(const char* name, const cocos2d::Rect& default_value) const
{
	if (is_good())
	{
		auto attr = detail(_Mynode)->first_attribute(name);
		if (attr != nullptr) {
			auto value = nsc::parse4f(std::string(attr->value(), attr->value_size()), ',');
			return cocos2d::Rect(std::get<0>(value), std::get<1>(value), std::get<2>(value), std::get<3>(value));
		}
	}

	return default_value;
}
cocos2d::Vec2    element::get_attribute_value(const char* name, const cocos2d::Vec2& default_value) const
{
	if (is_good())
	{
		auto attr = detail(_Mynode)->first_attribute(name);
		if (attr != nullptr) {
			auto value = nsc::parse2f(std::string(attr->value(), attr->value_size()), ',');
			return cocos2d::Vec2(std::get<0>(value), std::get<1>(value));
		}
	}

	return default_value;
}
cocos2d::Size    element::get_attribute_value(const char* name, const cocos2d::Size& default_value) const
{
	if (is_good())
	{
		auto attr = detail(_Mynode)->first_attribute(name);
		if (attr != nullptr) {
			auto value = nsc::parse2f(std::string(attr->value(), attr->value_size()), ',');
			return cocos2d::Size(std::get<0>(value), std::get<1>(value));
		}
	}


	return default_value;
}

#endif

void element::set_attribute_value(const char* name, const char* value)
{
    if (is_good()) {
        auto where = detail(_Mynode)->first_attribute(name);
        auto doc = detail(_Mynode)->document();
        if (where) {
            where->value(doc->allocate_string(value), strlen(value));
        }
        else {
            detail(_Mynode)->insert_attribute(where,
                doc->allocate_attribute(
                name, doc->allocate_string(value)));
        }
    }
}

element element::add_child(const char* name, const char* value /* = nullptr */)
{
    if (is_good()) {
        auto doc = detail(_Mynode)->document();
        auto newnode = doc->allocate_node(rapidxml::node_type::node_element, name, value != nullptr ? doc->allocate_string(value) : nullptr);
        detail(_Mynode)->append_node(newnode);
        return newnode;
    }
    return nullptr;
}

void element::remove_children(void)
{
    if (is_good()) {
        detail(_Mynode)->remove_all_nodes();
    }
}

void element::remove_children(const char* name)
{
    if (is_good())
    {
        auto first = detail(_Mynode)->first_node();
        decltype(first) next = nullptr;
        for (decltype(first)* curr = &first; *curr;)
        {
            decltype(first) entry = *curr;
            if (0 == memcmp(entry->name(), name, (std::min)(entry->name_size(), strlen(name)) ) )
            {
                *curr = entry->next_sibling();
                detail(_Mynode)->remove_node(entry);
            }
            else {
                next = entry->next_sibling();
                curr = &next;
            }
        }
    }
}

void element::remove_self(void)
{
    if (is_good()) {
        detail(_Mynode)->parent()->remove_node(detail(_Mynode));
        _Mynode = nullptr;
    }
}

std::string element::to_string(bool formatted) const
{
    if (is_good()) {
        std::string text;
        rapidxml::print(std::back_inserter(text), *detail(_Mynode), !formatted ? rapidxml::print_no_indenting : 0);
        return std::move(text);
    }
    return "";
}


/*----------------------------- friendly division line ----------------------------------------*/

struct xml4wDoc
{
    std::string              filename;
    std::string              buf;
    rapidxml::xml_document<> doc;
};

bool document::open(const char* filename)
{
    if (!is_open()) {
        impl_ = new(std::nothrow) xml4wDoc();
        if (impl_ != nullptr)
        {
            impl_->filename = filename;
#ifdef _USE_IN_COCOS2DX
            impl_->buf = cocos2d::FileUtils::getInstance()->getFileData(impl_->filename);
#else
			impl_->buf = fsutil::read_file_data(impl_->filename.c_str());
#endif
			if (impl_->buf.empty())
				return false;
#if _XMLDRV_SECURITY_SUPPORT == 1
            if (security_used)
            {
                crypto::aes::overlapped::decrypt(impl_->buf, xmldrvkey);
                impl_->buf = std::move(crypto::zlib::inflate(impl_->buf));
            }
#endif
			static const int parse_fastest_with_entity_translation = rapidxml::parse_no_string_terminators | rapidxml::parse_no_data_nodes;
            if (!impl_->buf.empty()) {
				impl_->doc.parse<parse_fastest_with_entity_translation>((char*)&impl_->buf.front(), (int)impl_->buf.size());
            }
            else {
                //cocos2d::showMessageBox("open file failed!", "xmldrv::document::open");
                delete impl_;
            }
        }
    }
    return is_open();
}

bool document::open(const char* filename, const char* rootname)
{
    if (!is_open()) {
        impl_ = new(std::nothrow) xml4wDoc();
        if (impl_ != nullptr)
        {
            impl_->doc.append_node(impl_->doc.allocate_node(rapidxml::node_type::node_element, rootname));
            impl_->filename = filename;
        }
    }
    return is_open();
}

bool document::open(const char* xmlstring, int length)
{
    if (!is_open()) {
        impl_ = new(std::nothrow) xml4wDoc();
        if (impl_ != nullptr)
        {
            impl_->buf.resize(length);
            ::memcpy(&impl_->buf.front(), xmlstring, length);

            if (!impl_->buf.empty()) {
                impl_->doc.parse<rapidxml::parse_fastest>((char*)&impl_->buf.front(), (int)impl_->buf.size());
                impl_->filename = "xmldrv.memory.xml";
            }
            else {
                //cocos2d::showMessageBox("open file failed!", "xmldrv::document::open");
                delete impl_;
            }
        }
    }
    return is_open();
}

void document::save(bool formatted) const
{
    if (is_open())
        this->save(impl_->filename.c_str(), formatted);
}

void document::save(const char* filename, bool formatted) const
{
    if (is_open()) {
        std::string stream = this->to_string(formatted && !security_used);
#if _XMLDRV_SECURITY_SUPPORT == 1
		if (security_used){
            stream = std::move(crypto::zlib::deflate(stream));
            crypto::aes::overlapped::encrypt(stream, xmldrvkey);
        }
#endif
        fsutil::write_file_data(filename, stream.data(), stream.size());
    }
}

element document::root(void) const
{
    return impl_->doc.first_node();
}

element document::create_element(const char* name)
{
    return impl_->doc.allocate_node(rapidxml::node_type::node_element, impl_->doc.allocate_string(name));
}

element document::select_element(const char*, int) const
{
    throw std::logic_error("It's just supported by xerces-c which has 3.0.0 or more version for xml4w using xpath to operate xml!");
    return nullptr;
}

std::string document::to_string(bool formatted) const
{
    if (is_open())
    {
        return ((element)&impl_->doc).to_string(formatted);
    }
    return "";
}

#elif defined(_USING_TINYXML2)
/*************************tinyxml wrapper impl*****************************/
#include "tinyxml/tinyxml2.h"

inline tinyxml2::XMLNode* detail(void* raw)
{
    return (tinyxml2::XMLNode*)raw;
}

inline tinyxml2::XMLNode* detail(const element& elem)
{
    return detail(static_cast<void*>(elem));
}

element element::clone(void) const
{
    if(is_good())
    {
        return detail(_Mynode)->ShallowClone(detail(_Mynode)->GetDocument());
    }
    return nullptr;
}

element element::get_parent(void) const
{
    if (is_good())
    {
        return detail(_Mynode)->Parent();
    }
    return nullptr;
}

element element::get_first_child(void) const
{
    if (is_good())
        return detail(_Mynode)->FirstChildElement();
    return nullptr;
}

element element::get_prev_sibling(void) const
{
    if (is_good())
        return detail(_Mynode)->PreviousSiblingElement();
    return nullptr;
}

element element::get_next_sibling(void) const
{
    if (is_good())
        return detail(_Mynode)->NextSiblingElement();
    return nullptr;
}

std::string element::get_name(void) const
{
    if(is_good()) {
        return dynamic_cast<tinyxml2::XMLElement*>(detail(_Mynode))->Name();
    }
    return "null";
}

std::string element::get_value(const char* default_value) const
{
    if (is_good()) {

        const char* text = detail(_Mynode)->ToElement()->GetText();
        if (text != nullptr) {
            return text;
        }
    }

    std::cerr << "xml4w::element::get_value failed, detail(element:" << this->get_name() << "), use the default value["
        << default_value << "] insteaded!\n";

    return default_value;
}

std::string element::get_attribute_value(const char* name, const char* default_value) const
{
    if(is_good())
    {
        const char* attr_value = dynamic_cast<tinyxml2::XMLElement*>(detail(_Mynode))->Attribute(name);
        if(attr_value != nullptr) {
            return dynamic_cast<tinyxml2::XMLElement*>(detail(_Mynode))->Attribute(name);
        }
    }

    std::cerr << "xml4w::element::get_attribute_value failed, detail(element:" << this->get_name() << ",property:" << name << "), use the default value["
        << default_value << "] insteaded!\n";

    return default_value;
}

void element::set_value(const char* value)
{
    if (is_good())
    {
        do { if (detail(_Mynode)->FirstChild() != nullptr){ detail(_Mynode)->FirstChild()->SetValue(value); } else { detail(_Mynode)->InsertFirstChild(detail(_Mynode)->GetDocument()->NewText(value)); } } while (false);
    }
}

void element::set_attribute_value(const char* name, const char* value)
{
    if (is_good()) {
        detail(_Mynode)->ToElement()->SetAttribute(name, value);
    }
}

void element::remove_children(void)
{
    if (is_good()) {
        detail(_Mynode)->DeleteChildren();
    }
}

void element::remove_children(const char* name)
{
    /*std::vector<element> children;
    this->get_children(name, children);
    std::for_each(
        children.begin(),
        children.end(),
        std::mem_fun_ref(&element::remove_self)
        );*/
}

void element::remove_self(void)
{
    if (is_good()) {
        detail(_Mynode)->GetDocument()->DeleteNode(detail(_Mynode));
        _Mynode = nullptr;
    }
}

std::string element::to_string(bool formatted) const
{
    if (is_good()) {
        detail(_Mynode)->ToElement()->ToText()->Value();
        return "";
    }
    return "";
}

element element::add_child(const char* name, const char* value /* = nullptr */)
{
    if (is_good())
    {
        auto doc = detail(_Mynode)->GetDocument();
        if (doc) {
            auto node = doc->NewElement(name);
            if (node) {
                node->SetValue(value);
                detail(_Mynode)->InsertEndChild(node);
            }
            return node;
        }
    }
    return nullptr;
}


/*----------------------------- friendly division line ----------------------------------------*/

struct xml4wDoc : public tinyxml2::XMLDocument
{
    std::string filename;
};

bool document::open(const char* filename)
{
    if (!is_open())
    {
        impl_ = new(std::nothrow) xml4wDoc();
        if (impl_ && tinyxml2::XML_NO_ERROR == impl_->LoadFile(filename))
        {
            impl_->filename = filename;
        }
        else {
            close();
        }
    }
    return is_open();
}

bool document::open(const char* filename, const char* rootname)
{
    if (!is_open()) {
        impl_ = new(std::nothrow) xml4wDoc();
        impl_->LinkEndChild(impl_->NewDeclaration("1.0"));
        impl_->LinkEndChild((impl_->NewElement(rootname)));
        impl_->filename = /*cocos2d::CCFileUtils::sharedFileUtils()->getWritablePath() + */filename;
        this->save();
        return true;
    }
    return is_open();
}

bool document::open(const char* xmlstring, int length)
{
    if (!is_open()) {

        impl_ = new(std::nothrow) xml4wDoc();
        if (tinyxml2::XML_NO_ERROR == impl_->Parse(xmlstring, length))
        {
            impl_->filename = "xml4w.memory.xml";
        }
        else {
            close();
        }
    }
    return is_open();
}

void document::save(bool formatted) const
{
    if (is_open())
        this->save(impl_->filename.c_str());
}

void document::save(const char* filename, bool formatted) const
{
    if (is_open()) {
        impl_->SaveFile(filename, formatted);
    }
}

element document::root(void)
{
    if (is_open())
        return impl_->RootElement();
    return nullptr;
}

std::string document::to_string(bool formatted) const
{
    if (is_open())
    {
        tinyxml2::XMLPrinter writer(0, !formatted);
        impl_->Print(&writer);
        return writer.CStr();//((element)impl_).to_string(formatted);
    }
    return "";
} 

/* end of tinyxml impl */

#elif defined(_USING_PUGIXML)
/*************************pugixml wrapper impl*****************************/
#include "pugixml/pugixml.hpp"

inline pugi::xml_node detail(void* raw)
{
    return static_cast<pugi::xml_node>( (pugi::xml_node_struct*)raw );
}
inline pugi::xml_node detail(const element& elem)
{
    return detail(static_cast<void*>(elem));
}
inline xml4wNodePtr simplify(const pugi::xml_node& detail)
{
    auto temp = detail;

    auto internal = *reinterpret_cast<void**>(&temp); // internal struct pointer

    return internal;
}
inline bool is_element(xml4wNodePtr ptr)
{
    return detail(ptr).type() == pugi::node_element;
}

element element::clone(void) const
{ // pugixml does not implement
    return nullptr;
}

element element::get_parent(void) const
{
    if (is_good())
        return simplify(detail(_Mynode).parent());
    return nullptr;
}

element element::get_first_child(void) const
{
    auto ptr = _Mynode;
    __xml4wts_algo_cond(ptr,
        simplify(detail(ptr).first_child()),
        simplify(detail(ptr).next_sibling()),
        is_element(_Mynode),
        break
        );
    return ptr;
}

element element::get_prev_sibling(void) const
{
    auto ptr = _Mynode;
    __xml4wts_algo_cond(ptr,
        simplify(detail(ptr).previous_sibling()),
        simplify(detail(ptr).previous_sibling()),
        is_element(_Mynode),
        break
        );
    return ptr;
}

element element::get_next_sibling(void) const
{
    auto ptr = _Mynode;
    __xml4wts_algo_cond(ptr,
        simplify(detail(ptr).next_sibling()),
        simplify(detail(ptr).next_sibling()),
        is_element(_Mynode),
        break
        );
    return ptr;
}

std::string element::get_name(void) const
{
    if (is_good()) {
        return detail(_Mynode).name();
    }
    return "null";
}

std::string element::get_value(const char* default_value) const
{
    if (is_good())
    {
        return detail(_Mynode).text().as_string();
    }

    std::cerr << "xml4w::element::get_value failed, detail(element:" << this->get_name() << "), use the default value["
        << default_value << "] insteaded!\n";
    return default_value;
}


std::string element::get_attribute_value(const char* name, const char* default_value) const
{
    if (is_good())
    {
        // real detail(_Mynode) pointer is internal struct pointer
        
        auto attr = detail(_Mynode).attribute(name);
        if (!attr.empty())
        {
            return attr.value();
        }
    }

    std::cerr << "xml4w::element::get_attribute_value failed, detail(element:" << this->get_name() << ",property:" << name << "), use the default value["
        << default_value << "] insteaded!\n";

    return default_value;
}

void element::set_value(const char* value)
{
    if (is_good())
        detail(_Mynode).set_value(value);
}

void element::set_attribute_value(const char* name, const char* value)
{
    if (is_good()) {
        auto attrib = detail(_Mynode).attribute(name);
        if (!attrib.empty())
            attrib.set_value(value);
        else {
            detail(_Mynode).append_attribute(name).set_value(value);
        }
    }
}

void element::remove_children(void)
{
}

void element::remove_children(const char* name)
{
    if (is_good())
        detail(_Mynode).remove_child(name);
}

void element::remove_self(void)
{
    if (is_good()) {
        auto self = detail(_Mynode);
        auto parent = self.parent();
        if (!parent.empty())
            parent.remove_child(self);
        _Mynode = nullptr;
    }
}

std::string element::to_string(bool formatted) const
{
    if (is_good()) {
        std::ostringstream oss;
        detail(_Mynode).print(oss, "  ", formatted ? pugi::format_indent : pugi::format_raw);
        return oss.str();
    }
    return "";
}

element element::add_child(const char* name, const char* value)
{
    if (is_good()) {
        auto newe = detail(_Mynode).append_child(name);
        newe.set_value(value);
        return simplify(newe);
    }
    return nullptr;
}


/*----------------------------- friendly division line ----------------------------------------*/

struct xml4wDoc
{
    std::string        filename;
    pugi::xml_document doc;
};

bool document::open(const char* filename)
{
    if (!is_open()) {
        this->impl_ = new (std::nothrow) xml4wDoc();
        if (this->impl_)
        {
            auto stats = this->impl_->doc.load_file(filename);
            bool succeed = stats;
            if (!succeed)
            {
                const char* msg = stats.description();
                close();
            }
        }
            
    }
    return is_open();
}

bool document::open(const char* filename, const char* rootname)
{
    if (!is_open()) {
        // TODO: impl pugi
    }
    return is_open();
}

bool document::open(const char* xmlstring, int length)
{
    if (!is_open()) {
        this->impl_ = new (std::nothrow) xml4wDoc();
        if (this->impl_)
            if (!this->impl_->doc.load_buffer(xmlstring, length))
                close();
    }
    return is_open();
}

element document::root(void)
{
    if (is_open())
        return simplify(impl_->doc.root().first_child());
    return nullptr;
}

void document::save(bool formatted) const
{
    if (is_open())
        this->save(this->impl_->filename.c_str(), formatted);
}

void document::save(const char* filename, bool formatted) const
{
    if (is_open()) {
        std::ofstream fout(filename, std::ios_base::binary);
        if (fout.is_open())
            this->impl_->doc.save(fout, "  ", (formatted ? pugi::format_indent : pugi::format_raw) | pugi::format_save_file_text);
    }
}

std::string document::to_string(bool formatted) const
{
    if (is_open())
    {
        std::ostringstream ss;
        this->impl_->doc.save(ss, "  ", formatted ? pugi::format_indent : pugi::format_raw);
        return ss.str();
    }
    return "";
}

xml4wXPathResultPtr document::xpath_eval(const char* xpath) const
{
    if (is_open())
        return new pugi::xpath_node_set(impl_->doc.select_nodes(xpath));
    return nullptr;
}

xml4wNodePtr        document::xpath_node_of(xml4wXPathResultPtr ptr, size_t index) const
{
    auto detail = (pugi::xpath_node_set*)ptr;
    if (index < detail->size())
        return (*detail)[index];
    return nullptr;
}

void                document::xpath_free_result(xml4wXPathResultPtr ptr) const
{
    auto detail = (pugi::xpath_node_set*)ptr;
    if (detail)
        delete detail;
}

/* end of pugixml impl */


/*************************rapidxml wrapper impl*****************************/
#elif defined(_USING_LIBXML2)
#include <libxml/xpath.h>

inline xmlNodePtr detail(void* raw)
{
    return (xmlNodePtr)raw;
}
inline xmlNodePtr detail(const element& elem)
{
    return detail(static_cast<void*>(elem));
}
inline bool is_element(xml4wNodePtr ptr)
{
    return detail(ptr)->type == XML_ELEMENT_NODE;
}

element element::clone(void) const
{
    if(is_good())
    {
        return xmlCopyNode(detail(_Mynode), 1);
    }
    return nullptr;
}

element element::get_parent(void) const
{
    if (is_good())
        return detail(_Mynode)->parent;
    return nullptr;
}

element element::get_prev_sibling(void) const
{
    auto ptr = detail(_Mynode);
    __xml4wts_algo_cond(ptr,
        ptr->prev,
        ptr->prev,
        is_element(_Mynode),
        break
        );
    return ptr;
}

element element::get_next_sibling(void) const
{
    auto ptr = detail(_Mynode);
    __xml4wts_algo_cond(ptr,
        ptr->next,
        ptr->next,
        is_element(_Mynode),
        break
        );
    return ptr;
}

element element::get_first_child(void) const
{
    auto ptr = detail(_Mynode);
    __xml4wts_algo_cond(ptr,
        ptr->children,
        ptr->next,
        is_element(_Mynode),
        break
        );
    return ptr;
}

std::string element::get_name(void) const
{
    if(is_good()) {
        return (const char*) detail(_Mynode)->name;
    }
    return "null";
}

std::string element::get_value(const char* default_value) const
{
    if (is_good()) {
        pod_ptr<char>::type value((char*)xmlNodeGetContent(detail(_Mynode)));
        return value.get();
    }

    std::cerr << "xml4w::element::get_value failed, detail(element:" << this->get_name() << "), use the default value["
        << default_value << "] insteaded!\n";

    return default_value;
}

std::string element::get_attribute_value(const char* name, const char* default_value) const
{
    if(is_good())
    {
        pod_ptr<xmlChar>::type value(xmlGetProp(detail(_Mynode), BAD_CAST name));
        if (value != nullptr)
        {
            return (const char*)value.get()/*, n*/;
        }
    }

    std::cerr << "xml4w::element::get_attribute_value failed, detail(element:" << this->get_name() << ",property:" << name << "), use the default value["
        << default_value << "] insteaded!\n";

    return default_value;
}

void element::set_value(const char* value)
{
    if (is_good())
    {
        xmlNodeSetContent(detail(_Mynode), BAD_CAST value);
    }
}

void element::set_attribute_value(const char* name, const char* value)
{
    if (is_good()) {
        xmlSetProp(detail(_Mynode), BAD_CAST name, BAD_CAST value);
    }
}

//element element::add_child(const element& e)
//{
//    if (is_good() && e.is_good()) {
//        xmlAddChild(detail(_Mynode), detail(e));
//        return e;
//    }
//    return nullptr;
//}

void element::remove_children(void)
{
    if (is_good()) {
        // xmlDOMWrapRemoveNode
        // detail(_Mynode)->remove_all_nodes();
    }
}

void element::remove_children(const char* name)
{
    if (is_good())
    {
        /*auto first = detail(_Mynode)->first_node();
        decltype(first) next = nullptr;
        for (decltype(first)* curr = &first; *curr;)
        {
            decltype(first) entry = *curr;
            if (0 == memcmp(entry->name(), name, (std::min)(entry->name_size(), strlen(name))))
            {
                *curr = entry->next_sibling();
                detail(_Mynode)->remove_node(entry);
            }
            else {
                next = entry->next_sibling();
                curr = &next;
            }
        }*/
    }
}

void element::remove_self(void)
{
    if (is_good()) {
        xmlDOMWrapRemoveNode(nullptr, detail(_Mynode)->doc, detail(_Mynode), 0);
        _Mynode = nullptr;
    }
}

std::string element::to_string(bool formatted) const
{
    if (is_good()) {
        simple_ptr<xmlBuffer, xmlBufferFree> buf(xmlBufferCreate());
        xmlNodeDump(buf, detail(_Mynode)->doc, detail(_Mynode), 1, formatted ? 1 : 0);
        return std::string((const char*)buf->content);
    }
    return "";
}

element element::add_child(const char* name, const char* value /* = nullptr */)
{
    if (is_good()) {
        auto newnode = xmlNewNode(nullptr, BAD_CAST name);
        xmlNodeSetContent(newnode, BAD_CAST value);
        xmlAddChild(detail(_Mynode), newnode);
        return newnode;
    }
    return nullptr;
}

/*----------------------------- friendly division line ----------------------------------------*/

struct xml4wDoc
{
    xml4wDoc(void) : doc(nullptr) {};
    ~xml4wDoc(void){ if (doc) xmlFreeDoc(doc); }
    xmlDocPtr                doc;
    std::string              filename;
};

bool document::open(const char* filename)
{
    if (!is_open()) {
        impl_ = new(std::nothrow) xml4wDoc();
        if (impl_ != nullptr)
        {
            xmlKeepBlanksDefault(0);
            impl_->doc = xmlReadFile(filename, "UTF-8", XML_PARSE_RECOVER | XML_PARSE_NOBLANKS | XML_PARSE_NODICT);
            // _Myroot = xmlDocGetRootElement(impl_->doc);
        }
    }
    return is_open();
}

bool document::open(const char* filename, const char* rootname)
{
    if (!is_open()) {
        impl_ = new(std::nothrow) xml4wDoc();
        if (impl_ != nullptr)
        {
            impl_->doc = xmlNewDoc(BAD_CAST "1.0");
            auto root = xmlNewNode(NULL, BAD_CAST rootname);
            xmlDocSetRootElement(impl_->doc, root);

            impl_->filename.assign(filename);
        }
    }
    return is_open();
}

bool document::open(const char* xmlstring, int length)
{
    if (!is_open()) {
        impl_ = new(std::nothrow) xml4wDoc();
        if (impl_ != nullptr)
        {
            xmlKeepBlanksDefault(0);
            impl_->doc = xmlReadMemory(xmlstring, length, "memory.libxml4w.xml", "UTF-8", 1);
            impl_->filename = "xml4w_memory.xml";
        }
    }
    return is_open();
}

void document::save(bool formatted) const
{
    if (is_open())
        this->save(impl_->filename.c_str(), formatted);
}

void document::save(const char* filename, bool formatted) const
{
    if (is_open()) {
        xmlSaveFormatFile(filename, impl_->doc, formatted ? 1 : 0);
    }
}

element document::root(void)
{
    if (is_open())
        return xmlDocGetRootElement(impl_->doc);
    return nullptr;
}

element document::select_element(const char*, int) const
{
    throw std::logic_error("It's just supported by xerces-c which has 3.0.0 or more version for xml4w using xpath to operate xml!");
    return nullptr;
}

std::string document::to_string(bool formatted) const
{
    if (is_open())
    {
        pod_ptr<xmlChar>::type buf;
        int size = 0;
        xmlDocDumpFormatMemory(impl_->doc, &buf, &size, formatted ? 1 : 0);
        return std::string((const char*) buf.get());
    }
    return "";
}

xml4wXPathResultPtr  document::xpath_eval(const char* xpath) const
{
    if (is_open())
    {
        simple_ptr<xmlXPathContext, xmlXPathFreeContext> ctx(xmlXPathNewContext(impl_->doc));
        return xmlXPathEvalExpression(BAD_CAST xpath, ctx);
    }
    return nullptr;
}

xml4wNodePtr  document::xpath_node_of(xml4wXPathResultPtr ptr, size_t index) const
{
    auto detail = (xmlXPathObjectPtr)ptr;
    if (index < detail->nodesetval->nodeNr)
    {
        return detail->nodesetval->nodeTab[index];
    }
    return nullptr;
}

void  document::xpath_free_result(xml4wXPathResultPtr result)  const
{
    if (result != nullptr)
        xmlXPathFreeObject((xmlXPathObject*)result);
}

namespace {
    struct XMLPlatformUtilsInitializer
    {
        XMLPlatformUtilsInitializer(void)
        {
            xmlInitParser();
        }
        ~XMLPlatformUtilsInitializer(void)
        {
            xmlCleanupParser();
        }
    };
    XMLPlatformUtilsInitializer __init_utils;
};

#elif defined(_USING_XERCESC)
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#    if XERCES_VERSION_MAJOR == 2
#        pragma comment(lib, "xerces-c_2D.lib")
#    elif XERCES_VERSION_MAJOR == 3
#        pragma comment(lib, "xerces-c_3_1D.lib")
#    endif
#ifdef _WIN32
#define XMLWRAP_URI_PREFIX "file:///"
#else
#define XMLWRAP_URI_PREFIX "file://"
#endif
using namespace xercesc;
inline xercesc::DOMNode* detail(void* raw)
{
    return (xercesc::DOMNode*)raw;
}

inline xercesc::DOMNode* detail(const element& elem)
{
    return detail(static_cast<void*>(elem));
}

inline bool is_element(xercesc::DOMNode* ptr)
{
    return xercesc::DOMNode::ELEMENT_NODE == ptr->getNodeType();
}

std::string _xml4w_transcode(const XMLCh* source)
{
    char* value = XMLString::transcode(source);
    
    if (value != nullptr)
    {
        std::string sRet(value);
        XMLString::release(&value);
        return std::move(sRet);
    }
    return "";
}

std::basic_string<XMLCh> _xml4w_transcode(const char* source)
{
    XMLCh* value = XMLString::transcode(source);
    if (value != nullptr)
    {
        std::basic_string<XMLCh> sRet(value);
        XMLString::release(&value);
        return std::move(sRet);
    }
    return std::basic_string<XMLCh>();
}

void _xml4w_serialize(xercesc::DOMNode* _TheNode, XMLFormatTarget& _Target, bool _Formated = true)
{
    static const XMLCh tempLS[] = {chLatin_L, chLatin_S, chNull};

#if XERCES_VERSION_MAJOR >= 3
    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(tempLS);
    DOMLSSerializer*   writer = ((DOMImplementationLS*)impl)->createLSSerializer();
    DOMLSOutput*       output = ((DOMImplementationLS*)impl)->createLSOutput();

    output->setByteStream(&_Target);

    writer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, _Formated);

    writer->write(_TheNode, output);

    output->release();
    writer->release();
#else
    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(tempLS);
    DOMWriter* writer = ((DOMImplementationLS*)impl)->createDOMWriter();

    writer->setFeature((XMLCh*)L"format-pretty-print", _Formated);

    writer->writeNode(&_Target, *_TheNode);

    writer->release();
#endif
}

element element::clone(void) const
{
    if(is_good())
    {
        return detail(_Mynode)->cloneNode(true);
    }
    return nullptr;
}

element element::get_parent(void) const
{
    if (is_good())
    {
        return detail(_Mynode)->getParentNode();
    }
    return nullptr;
}

element element::get_first_child(void) const
{
    if (is_good()) {
        auto ptr = dynamic_cast<DOMElement*>(detail(_Mynode));
        if (ptr != nullptr)
            return ptr->getFirstElementChild();
    }
    return nullptr;
}

element element::get_prev_sibling(void) const
{
    if (is_good()) {
        auto ptr = dynamic_cast<DOMElement*>(detail(_Mynode));
        if (ptr != nullptr)
            return ptr->getPreviousElementSibling();
    }
    return nullptr;
}

element element::get_next_sibling(void) const
{
    if (is_good()) {
        auto ptr = dynamic_cast<DOMElement*>(detail(_Mynode));
        if (ptr != nullptr)
            return ptr->getNextElementSibling();
    }
    return nullptr;
}

std::string element::get_name(void) const
{
    if(is_good()) {
        return _xml4w_transcode(detail(_Mynode)->getNodeName());
    }
    return "null";
}

std::string element::get_value(const char* default_value) const
{
    if (is_good()) {

        return _xml4w_transcode(detail(_Mynode)->getTextContent());
    }

    std::cerr << "xml4w::element::get_value failed, detail(element:" << this->get_name() << "), use the default value["
        << default_value << "] insteaded!\n";

    return default_value;
}

std::string element::get_attribute_value(const char* name, const char* default_value) const
{
    if(is_good())
    {
        auto elem = dynamic_cast<xercesc::DOMElement*>(detail(_Mynode));
        if (elem->hasAttribute(_xml4w_transcode(name).c_str()))
        {
            return _xml4w_transcode(elem->getAttribute(_xml4w_transcode(name).c_str()));
        }
    }

    std::cerr << "xml4w::element::get_attribute_value failed, detail(element:" << this->get_name() << ",property:" << name << "), use the default value["
        << default_value << "] insteaded!\n";

    return default_value;
}

void element::set_value(const char* value)
{
    if (is_good())
    {
        auto theval = XMLString::transcode(value);
        if (theval != nullptr) {
            detail(_Mynode)->setTextContent(theval);
            XMLString::release(&theval);
        }
    }
}

void element::set_attribute_value(const char* name, const char* value)
{
    if (is_good()) {

        auto n = XMLString::transcode(name);
        if (_IsNull(n))
            return;

        auto v = XMLString::transcode(value);
        if (_IsNull(v)) {
            XMLString::release(&n);
            return;
        }

        ((xercesc::DOMElement*)_Mynode)->setAttribute(n, v);

        XMLString::release(&n);
        XMLString::release(&v);
    }
}

void element::remove_children(void)
{
    if (is_good()) {
        auto current = dynamic_cast<xercesc::DOMElement*>(detail(_Mynode));
        auto ptr = current->getFirstElementChild();
        xercesc::DOMNode* removing = nullptr;
        while (ptr != nullptr)
        {
            removing = ptr;
            ptr = ptr->getNextElementSibling();
            current->removeChild(removing);
        }
    }
}

void element::remove_children(const char* name)
{
    if (is_good()) {
        auto current = dynamic_cast<xercesc::DOMElement*>(detail(_Mynode));
        auto ptr = current->getFirstElementChild();
        xercesc::DOMNode* removing = nullptr;
        while (ptr != nullptr)
        {
            removing = ptr;
            ptr = ptr->getNextElementSibling();
            if (_xml4w_transcode(name) == ptr->getNodeName())
                current->removeChild(removing);
        }
    }
}

void element::remove_self(void)
{
    if (is_good()) {
        detail(_Mynode)->getParentNode()->removeChild(detail(_Mynode));
        _Mynode = nullptr;
    }
}

std::string element::to_string(bool formatted) const
{
    if (is_good()) {
        MemBufFormatTarget target;
        _xml4w_serialize(detail(_Mynode), target, formatted);
        return std::string((const char*)target.getRawBuffer());
    }
    return "";
}

element element::add_child(const char* name, const char* value /* = nullptr */)
{
    if (is_good())
    {
        auto doc = detail(_Mynode)->getOwnerDocument();
        if (doc) {
            auto node = doc->createElement(_xml4w_transcode(name).c_str());
            if (node) {
                node->setTextContent(_xml4w_transcode(value).c_str());
                detail(_Mynode)->appendChild(node);
            }
            return node;
        }
    }
    return nullptr;
}


/*----------------------------- friendly division line ----------------------------------------*/

struct xml4wDoc 
{
    xml4wDoc(bool bForParse = true) : doc(nullptr)
    {
        if(bForParse)
            parser = new xercesc::XercesDOMParser();
    }
    ~xml4wDoc(void)
    {
        if(parser != nullptr)
            delete parser; // doc will be released by parser
        else {
            doc->release(); // release doc self.
        }
    }
    std::string               filename;
    xercesc::DOMDocument*     doc;
    xercesc::XercesDOMParser* parser;
};

bool document::open(const char* filename)
{
    if (!is_open())
    {
        impl_ = new(std::nothrow) xml4wDoc();
        if (impl_)
        {
            try {
                impl_->parser->setValidationScheme(XercesDOMParser::Val_Always);
                if (impl_->parser != nullptr) {
                    impl_->parser->parse(filename);
                    impl_->doc = impl_->parser->getDocument();
                    impl_->filename = filename;
                }
            }
            catch (...)
            {
                close();
            }
        }
        else {
            close();
        }
    }
    return is_open();
}

bool document::open(const char* filename, const char* rootname)
{
    if (!is_open()) {
        impl_ = new(std::nothrow) xml4wDoc(false);
        if(impl_ != nullptr)
        {
            impl_->doc = DOMImplementationRegistry::getDOMImplementation(nullptr)->createDocument();
            impl_->doc->createElement(_xml4w_transcode(rootname).c_str());
            impl_->doc->setDocumentURI(_xml4w_transcode(filename).c_str());
            impl_->filename = /*cocos2d::CCFileUtils::sharedFileUtils()->getWritablePath() + */filename;
        }
        return true;
    }
    return is_open();
}

bool document::open(const char* xmlstring, int length)
{
    if (!is_open()) {

        impl_ = new(std::nothrow) xml4wDoc();
        if (impl_ != nullptr)
        {
            MemBufInputSource* memBufIS = nullptr;
            impl_->parser->setValidationScheme(XercesDOMParser::Val_Auto);
            try {
                memBufIS = new MemBufInputSource(
                    (const XMLByte*)xmlstring
                    , (XMLSize_t)length
                    , (const char *const)nullptr);

                impl_->parser->parse(*memBufIS);

                impl_->doc = impl_->parser->getDocument();

                impl_->filename = "xml4w.memory.xml";
            }
            catch(...)
            {
                close();
            }

            if(memBufIS != nullptr)
                delete memBufIS;  
        }
        else {
            close();
        }
    }
    return is_open();
}

void document::save(bool formatted) const
{
    if (is_open())
    {
        std::string uri = _xml4w_transcode(impl_->doc->getDocumentURI());
        if(strstr(uri.c_str(), XMLWRAP_URI_PREFIX) != nullptr)
        {
            this->save(uri.c_str() + sizeof(XMLWRAP_URI_PREFIX) - 1);
        }
        else {
            this->save(uri.c_str());
        }
    }
}

void document::save(const char* filename, bool formatted) const
{
    if (is_open()) {
        LocalFileFormatTarget target(filename);
        _xml4w_serialize(impl_->doc, target, formatted);
    }
}

element document::root(void)
{
    if (is_open())
        return impl_->doc->getDocumentElement();
    return nullptr;
}

std::string document::to_string(bool formatted) const
{
    if (is_open())
    {
        return static_cast<element>(impl_->doc->getDocumentElement()).to_string(formatted);
    }
    return "";
}

xml4wXPathResultPtr  document::xpath_eval(const char* xpath) const
{
    if (is_open())
    {
        return impl_->doc->evaluate(_xml4w_transcode(xpath).c_str(),
            impl_->doc->getDocumentElement(),
            nullptr,
            DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
            nullptr
            );
    }
    return nullptr;
}

xml4wNodePtr  document::xpath_node_of(xml4wXPathResultPtr ptr, size_t index) const
{
    auto detail = (DOMXPathResult*)ptr;
    if(detail->snapshotItem(index))
        return detail->getNodeValue();
    return nullptr;
}

void  document::xpath_free_result(xml4wXPathResultPtr result)  const
{
    if (result != nullptr)
        ((DOMXPathResult*)result)->release();
}

namespace {
    struct XMLPlatformUtilsInitializer
    {
        XMLPlatformUtilsInitializer(void)
        {
            try {
                XMLPlatformUtils::Initialize();
            }
            catch (const XMLException& toCatch) {
                char *pMessage = XMLString::transcode(toCatch.getMessage());
                fprintf(stderr, "xml4wrapper3_6_2: Error during XMLPlatformUtils::Initialize(). \n"
                    "  Message is: %s\n", pMessage);
                XMLString::release(&pMessage);
                return;
            }
        }
        ~XMLPlatformUtilsInitializer(void)
        {
            XMLPlatformUtils::Terminate();
        }
    };
    XMLPlatformUtilsInitializer __init_utils;
};

/* end of xerces-c impl

/*************************vtd-xml wrapper impl*****************************/
#elif defined(_USING_VTDXML)
#include "vtd-xml/vtdGen.h"
#include "vtd-xml/vtdNav.h"
#include "vtd-xml/autoPilot.h"
#include "vtd-xml/bookMark.h"
#include "vtd-xml/XMLModifier.h"
#include "vtd-xml/nodeRecorder.h"
#include "vtd-xml/textIter.h"

using namespace com_ximpleware;

struct xml4wDoc : public VTDGen
{
    VTDNav* nav;
};

struct xml4wNode
{
    xml4wNode(xml4wDoc* arg1) : vg(arg1)
    {
    }
    xml4wDoc* vg;
};

static purelib::gc::object_pool<xml4wNode> s_node_pool;

element::element(xml4wNode* ptr) : detail(_Mynode)(ptr)
{

}

std::string element::get_attribute_value(const char* name, const char* default_value) const
{
    if (_IsNotNull(detail(_Mynode)))
    {
        //detail(_Mynode)->nav-
        //this->detail(_Mynode)->
    }

    return default_value;
}

element::~element(void)
{
    if (detail(_Mynode) != nullptr)
        s_node_pool.release(detail(_Mynode));
}

document::document(void) : impl(nullptr)
{
}

document::~document(void)
{
    if (_IsNotNull(impl))
    {
        delete impl->getXML();
        delete impl;
    }
}

bool document::is_open(void) const
{
    return this->impl != nullptr;
}

bool document::open(const char* filename)
{
    if (!is_open())
    {
        try{
            this->impl = new xml4wDoc();
            this->impl->parseFile(false, filename);
            //auto nav = this->impl->getNav();
            //// nav->toElement(FIRST_CHILD);
            //auto depth = nav->getCurrentDepth();
            //auto index = nav->getCurrentIndex();
            //int len = nav->getRawStringLength(index);
            //auto value = nav->toRawString(index);
            //
            //
            //delete nav;
            return true;
        }
        catch (ParseException &e){
            //vg.printLineNumber();
            printf(" error ===> %s \n", e.getMessage());
        }
        catch (...) {
            printf(" unknown error ===> \n");
        }

        if (impl)
            delete impl, impl = nullptr;

        return false;
    }

    return true;
}

element document::root(void)
{
    return new(s_node_pool.get()) xml4wNode(this->impl);
}

#endif

#ifdef _IMPL_COMM
void document::close(void)
{
    if (is_open())
    {
        delete impl_;
        impl_ = nullptr;
    }
}

/// xpath support impl
#if defined(_XML4W_XPATH_SUPPORTED)
element document::select_element(const char* xpath, int index) const
{
    auto result = this->xpath_eval(xpath);

    xml4wNodePtr ptr = this->xpath_node_of(result, index);

    this->xpath_free_result(result);

    return ptr;
}
#endif

#endif


