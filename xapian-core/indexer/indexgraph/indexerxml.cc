/* indexerxml.cc: Functions dealing with XML input
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "config.h"
#include "omdebug.h"
#include "indexerxml.h"
#include "om/omerror.h"
#include "register_core.h"
#include <parser.h>  // libxml
#ifdef HAVE_LIBXML_VALID
#include <valid.h>
#endif
#include <stdarg.h>
#include <algorithm>
#include <map>

/** This clump of defines is for compatibility across libxml1 and 2.
 *  libxml2 and later versions of libxml1 should have these already.
 *  These should work for earlier libxml1 versions.
 */
#ifndef xmlChildrenNode
#define xmlChildrenNode childs
#define xmlRootNode root
#endif

/** libxml1 defined CHAR -> xmlChar. libxml2 doesn't, apparently */
#ifndef CHAR
#define CHAR xmlChar
#endif

/** libxml2 behaves properly wrt all whitespace, whereas libxml1 didn't.
 *  This enables us to still work with both.
 */
#ifndef HAVE_LIBXML2
#define xmlIsBlankNode(x) (0)
#endif

/** A trivial "smart pointer" which calls xmlFreeDoc when going out of
 *  scope.
 */
class auto_xmlDocPtr {
    public:
	/** Construct a freeer object */
	auto_xmlDocPtr(xmlDocPtr doc_) : doc(doc_) {}

	/** Free the xml document tree */
	~auto_xmlDocPtr() {
	    if (doc) {
		xmlFreeDoc(doc);
	    }
	}

	/** return the contained xmlDocPtr */
	xmlDocPtr get() const {
	    return doc;
	}
    private:
	xmlDocPtr doc;
};

/** Return true if this xml Document is valid */
static bool doc_is_valid(xmlDocPtr doc);

/** Convert an xmlChar * into a std::string */
std::string xmlChar2string(const CHAR *s) {
    return std::string((char *)s);
}

/** Walk the xml tree to make an OmIndexerDesc */
static AutoPtr<OmIndexerDesc::Internal> desc_from_tree(xmlDocPtr doc);

AutoPtr<OmIndexerDesc::Internal>
desc_from_xml_file(const std::string &filename)
{
    auto_xmlDocPtr doc = xmlParseFile(filename.c_str());

    if (!doc.get()) {
	throw OmInvalidDataError("Error parsing XML file");
    }

    if (!doc_is_valid(doc.get())) {
	throw OmInvalidDataError("Graph did not validate");
    }

    return desc_from_tree(doc.get());
}

AutoPtr<OmIndexerDesc::Internal>
desc_from_xml_string(const std::string &xmldesc)
{
    auto_xmlDocPtr doc = xmlParseMemory(const_cast<char *>(xmldesc.c_str()),
				   xmldesc.length());

    if (!doc.get() || !doc_is_valid(doc.get())) {
	throw OmInvalidDataError("Graph definition is invalid");
    }

    return desc_from_tree(doc.get());
}

extern "C" {
static void xml_error_func(void *ctx, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    throw OmInvalidDataError("xml is not valid");
}

static void xml_warn_func(void *ctx, const char *fmt, ...) {
    va_list ap;
    std::string message("XML WARNING: ");

#if HAVE_VSNPRINTF  // FIXME: actually check for vsnprintf
    va_start(ap, fmt);
    char *buf = 0;
    try {
	const size_t max_size = 1024;
	size_t size = 64;
	buf = new char[size];

	bool done = false;
	while (!done) {
	    int result = vsnprintf(buf, size, fmt, ap);
	    if (result < 0 || result > size) {
		// didn't have enough space
		size *= 2;
		if (size > max_size) {
		    // write what we have and give up on the rest
		    done = true;
		    message += buf;
		    message += "<TRUNCATED>";
		    delete [] buf;
		    buf = 0;
		} else {
		    // enlarge the buffer and try again
		    delete [] buf;
		    buf = 0;
		    buf = new char[size];
		}
	    } else {
		// success!
		done = true;
		message += buf;
		delete [] buf;
		buf = 0;
	    }

	}
    } catch (...) {
	if (buf) {
	    delete [] buf;
	}
	throw;
    }
    va_end(ap);
#else
    // FIXME XXX: this may overflow.
    va_start(ap, fmt);
    char *buf = 0;
    try {
	buf = new char[16384];
	vsprintf(buf, fmt, ap);
	message += buf;
	delete [] buf;
	buf = 0;
    } catch (...) {
	if (buf) {
	    delete [] buf;
	    buf = 0;
	}
    }
    va_end(ap);
#endif // HAVE_VSNPRINTF

    DEBUGLINE(INDEXER, message);
}

} // extern "C"

CHAR *get_dtd_path()
{
    const char *envpath = getenv("OM_DTD_PATH");
    CHAR *result = 0;
    if (envpath) {
	result = (CHAR *)envpath;
    } else {
	result = (CHAR *)DATADIR "/omindexer.dtd";
    }
    DEBUGLINE(INDEXER, "Using DTD Path: " << (char *)result);
    return result;
}

void
add_dtd_if_needed(xmlDocPtr doc) {
    // add the internal subset Dtd if it's not there
    if (doc->intSubset == 0) {
	xmlCreateIntSubset(doc, (const xmlChar *)"omindexer", 0, get_dtd_path());
    }
}

static bool
doc_is_valid(xmlDocPtr doc)
{
#ifdef HAVE_LIBXML_VALID
    xmlValidCtxt ctxt;
    ctxt.error = &xml_error_func;
    ctxt.error = &xml_warn_func;

    // Add our predefined "START" id
    xmlAttrPtr attr = xmlNewDocProp(doc, (const xmlChar *)"id", (const xmlChar *)"START");
    xmlAddID(&ctxt, doc, (const xmlChar *)"START", attr);

    add_dtd_if_needed(doc);

    xmlFreeProp(attr);
    attr = 0;

    return xmlValidateDocument(&ctxt, doc);
#else  // HAVE_LIBXML_VALID
    return true;
#endif
}

typedef std::map<std::string, std::string> attrmap;
typedef std::map<std::string, std::string> typemap;

static std::string
get_prop(xmlNodePtr node, const std::string &prop)
{
    /* CHAR is used because it works with older libxmls, before
     * the xmlChar typedef.
     */
    CHAR *temp = 0;
    std::string retval;
    try {
	temp = xmlGetProp(node, (const xmlChar *)prop.c_str());
	if (temp) {
	    retval = xmlChar2string(temp);
	    free(temp);
	}
	temp = 0;
    } catch (...) {
	if (temp) free(temp);
	temp = 0;
	throw;
    }
    return retval;
}

#if 0
// FIXME: handle the unicode stuff rather than char *
static attrmap attr_to_map(xmlNodePtr node)
{
    xmlAttrPtr attr = node->properties;
    std::map<std::string, std::string> result;
    while (attr) {
	std::string name = xmlChar2string(attr->name);
	CHAR *temp = 0;
	std::string value;
	try {
	    //cerr << "Attr " << node->name << "." << name << "=" << endl;
	    temp = xmlGetProp(node, name.c_str());
	    if (temp) {
		value = xmlChar2string(temp);
		//cerr << "\tvalue = " << value << endl;
		free(temp);
		temp = 0;
	    }
#if 0  /* this is there to help in debugging odd attributes */
	    {
		xmlDocPtr doc = node->doc;
		xmlAttributePtr attrDecl;
		if (doc->intSubset) {
		    attrDecl = xmlGetDtdAttrDesc(doc->intSubset, node->name,
						 name.c_str());
		    if (attrDecl) {
			//cerr << "\tintSubset default = "
			//	<< attrDecl->defaultValue << endl;
		    }
		}
		if (doc->extSubset) {
		    attrDecl = xmlGetDtdAttrDesc(doc->extSubset, node->name,
						 name.c_str());
		    if (attrDecl) {
			//cerr << "\tintSubset default = "
			//	<< attrDecl->defaultValue << endl;
		    }
		}
	    }
#endif
	} catch (...) {
	    if (temp) free(temp);
	    throw;
	}
	/*
	 *
	 * xmlNodePtr val = attr->val;
	 * std::string value = xmlChar2string(val->content);
	 */

	result[name] = value;

	attr = attr->next;
    }
    return result;
}
#endif /* 0 */

/** Turn the parameters specified in <param> tags into OmSettings
 *  entries.
 *  
 *  @param node		The first <param> node
 *  @param config	The OmSettings object to modify
 *
 *  @return The first non-<param> node found.
 */
xmlNodePtr
get_config_values(xmlNodePtr node, OmSettings &config)
{
    while (node != 0) {
	// skip comments...
	if (node->type == XML_COMMENT_NODE || xmlIsBlankNode(node)) {
	    node = node->next;
	    continue;
	}
	
	// finish if we're no longer seeing <param ...>
	if (std::string(xmlChar2string(node->name)) != "param") {
	    break;
	}

	std::string type = get_prop(node, "type");
	if (type == "string") {
	    std::string name = get_prop(node, "name");
	    std::string value = get_prop(node, "value");

	    if (value.length() == 0) {
		// fall back on the data inside the tag
		CHAR *content = xmlNodeGetContent(node);
		if (content) {
		    value = xmlChar2string(content);
		}
	    }
	    config.set(name, value);
#if 0
FIXME: do something to replace this?
	} else if (type == "list") {
	    xmlNodePtr items = node->xmlChildrenNode;
	    std::vector<std::string> values;
	    while (items) {
		if (items->type == XML_COMMENT_NODE || xmlIsBlankNode(items)) {
		    items = items->next;
		    continue;
		}
		std::string name(xmlChar2string(items->name));
		if (name != "item") {
		    throw OmInvalidDataError(std::string("Unexpected tag `")
					     + name + "'");
		}
		values.push_back(get_prop(items, "value"));
		items = items->next;
	    }
	    config.set(get_prop(node, "name"),
		       values.begin(),
		       values.end());
#endif
	} else {
	    throw OmInvalidDataError(std::string("Invalid <param> type `")
				     + type + "'");
	}
	node = node->next;
    }
    return node;
}

static AutoPtr<OmIndexerDesc::Internal>
desc_from_tree(xmlDocPtr doc)
{
    xmlNodePtr root = doc->xmlRootNode;
    //cerr << "intSubset = " << doc->intSubset << endl;
    //cerr << "extSubset = " << doc->extSubset << endl;
    if (!root) {
	throw OmInvalidDataError("Error parsing graph description");
    }

    /* Add the DTD declarations, so that default values etc. work. */
    add_dtd_if_needed(doc);

    std::string rootname = xmlChar2string(root->name);
    if (rootname != "omindexer") {
	throw OmInvalidDataError(std::string("Root tag was `")
				 + rootname + "', not <omindexer>");
    }

    AutoPtr<OmIndexerDesc::Internal> result(new OmIndexerDesc::Internal);

    for (xmlNodePtr node = root->xmlChildrenNode;
	 node != 0;
	 node = node->next) {

	if (node->type == XML_COMMENT_NODE || xmlIsBlankNode(node)) {
	    continue;
	}
	std::string type = xmlChar2string(node->name);
	if (type == "node") {
	    OmIndexerDesc::Internal::NodeInstance ndesc;

	    ndesc.type = get_prop(node, "type");
	    ndesc.id = get_prop(node, "id");

	    xmlNodePtr child = node->xmlChildrenNode;

	    child = get_config_values(child, ndesc.param);

	    // translate the inputs
	    while (child != 0) {
		if (child->type == XML_COMMENT_NODE || xmlIsBlankNode(child)) {
		    child = child->next;
		    continue;
		}

		if (std::string(xmlChar2string(child->name)) != "input") {
		    throw OmInvalidDataError(std::string("<input> tag expected, found ") + std::string(xmlChar2string(child->name)));
		}
		OmIndexerDesc::Internal::Connect conn;
		conn.input_name = get_prop(child, "name");
		conn.feeder_node = get_prop(child, "node");
		conn.feeder_out = get_prop(child, "out_name");
		ndesc.input.push_back(conn);

		child = child->next;
	    }
	    result->data->nodes.push_back(ndesc);
	} else if (type == "output") {
	    result->data->output_node = get_prop(node, "node");
	    result->data->output_pad = get_prop(node, "out_name");
	}
    }
    return result;
}
