/* htmlparse.cc: simple HTML parser for omega indexer
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Olly Betts
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

#include <string>
#include <map>

using std::string;
using std::map;

class HtmlParser {
    protected:
	void decode_entities(string &s);
	static map<string, unsigned int> named_ents;
    public:
	virtual void process_text(const string &text) { }
	virtual void opening_tag(const string &tag,
				 const map<string,string> &p) { }
	virtual void closing_tag(const string &tag) { }
	virtual void parse_html(const string &text);
	HtmlParser() {
	    static struct ent {const char *n; unsigned int v;} ents[] = {
		{ "quot", 34 },
		{ "amp", 38 },
		{ "lt", 60 },
		{ "gt", 62 },
		{ "AElig", 198 },
		{ "Aacute", 193 },
		{ "Acirc", 194 },
		{ "Agrave", 192 },
		{ "Aring", 197 },
		{ "Atilde", 195 },
		{ "Auml", 196 },
		{ "Ccedil", 199 },
		{ "ETH", 208 },
		{ "Eacute", 201 },
		{ "Ecirc", 202 },
		{ "Egrave", 200 },
		{ "Euml", 203 },
		{ "Iacute", 205 },
		{ "Icirc", 206 },
		{ "Igrave", 204 },
		{ "Iuml", 207 },
		{ "Ntilde", 209 },
		{ "Oacute", 211 },
		{ "Ocirc", 212 },
		{ "Ograve", 210 },
		{ "Oslash", 216 },
		{ "Otilde", 213 },
		{ "Ouml", 214 },
		{ "THORN", 222 },
		{ "Uacute", 218 },
		{ "Ucirc", 219 },
		{ "Ugrave", 217 },
		{ "Uuml", 220 },
		{ "Yacute", 221 },
		{ "aacute", 225 },
		{ "acirc", 226 },
		{ "acute", 180 },
		{ "aelig", 230 },
		{ "agrave", 224 },
		{ "aring", 229 },
		{ "atilde", 227 },
		{ "auml", 228 },
		{ "brvbar", 166 },
		{ "ccedil", 231 },
		{ "cedil", 184 },
		{ "cent", 162 },
		{ "copy", 169 },
		{ "curren", 164 },
		{ "deg", 176 },
		{ "divide", 247 },
		{ "eacute", 233 },
		{ "ecirc", 234 },
		{ "egrave", 232 },
		{ "eth", 240 },
		{ "euml", 235 },
		{ "frac12", 189 },
		{ "frac14", 188 },
		{ "frac34", 190 },
		{ "iacute", 237 },
		{ "icirc", 238 },
		{ "iexcl", 161 },
		{ "igrave", 236 },
		{ "iquest", 191 },
		{ "iuml", 239 },
		{ "laquo", 171 },
		{ "macr", 175 },
		{ "micro", 181 },
		{ "middot", 183 },
		{ "nbsp", 160 },
		{ "not", 172 },
		{ "ntilde", 241 },
		{ "oacute", 243 },
		{ "ocirc", 244 },
		{ "ograve", 242 },
		{ "ordf", 170 },
		{ "ordm", 186 },
		{ "oslash", 248 },
		{ "otilde", 245 },
		{ "ouml", 246 },
		{ "para", 182 },
		{ "plusmn", 177 },
		{ "pound", 163 },
		{ "raquo", 187 },
		{ "reg", 174 },
		{ "sect", 167 },
		{ "shy", 173 },
		{ "sup1", 185 },
		{ "sup2", 178 },
		{ "sup3", 179 },
		{ "szlig", 223 },
		{ "thorn", 254 },
		{ "times", 215 },
		{ "uacute", 250 },
		{ "ucirc", 251 },
		{ "ugrave", 249 },
		{ "uml", 168 },
		{ "uuml", 252 },
		{ "yacute", 253 },
		{ "yen", 165 },
		{ "yuml", 255 },
// iso8859-1 only for now	{ "OElig", 338 },
// ditto			{ "oelig", 339 },
		{ NULL, 0 }
	    };
	    if (named_ents.empty()) {
		struct ent *i = ents;
		while (i->n) {
		    named_ents[string(i->n)] = i->v;
		    ++i;
		}
	    }
	}
	virtual ~HtmlParser() { }
};
