/* omflattenstringnode.cc: Node which flattens a structure containing
 * 			   strings into a single node of type string.
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

#include "om/omindexernode.h"
#include "om/omstem.h"
#include "om/omerror.h"
#include "node_reg.h"

/** Node which turns a list containing strings into a single string.
 *
 *  The omflattenstring node takes as input any structure built of
 *  vectors and strings (but not numerical types) and concatenates the
 *  strings together into one.
 *
 *  Inputs:
 *  	in: The structure with strings.
 *
 *  Outputs:
 *  	out: The string consisting of all input strings concatenated.
 *
 *  Parameters: none
 */
class OmFlattenStringNode : public OmIndexerNode {
    public:
	OmFlattenStringNode(const OmSettings &config)
		: OmIndexerNode(config)
	{ }
    private:
	void calculate() {
	    request_inputs();
	    OmIndexerMessage input = get_input_record("in");
	    if (input.is_empty()) {
		// propagate empty messages
		set_empty_output("out");
		return;
	    }

	    set_output("out", flatten(input));
	}
	std::string flatten(const OmIndexerMessage &data) {
	    switch (data.get_type()) {
		case OmIndexerMessage::rt_string:
		    return data.get_string();
		case OmIndexerMessage::rt_empty:
		    return std::string();
		    break;
		case OmIndexerMessage::rt_vector:
		    {
			std::string accum;
			for (size_t i=0; i<data.get_vector_length(); i++) {
			    accum += flatten(data.get_element(i));
			}
			return accum;
		    }
		default:
		    throw OmTypeError("Can only flatten string leaves");
	    }
	}
};

NODE_BEGIN(OmFlattenStringNode, omflattenstring)
NODE_INPUT("in", "*1", mt_record)
NODE_OUTPUT("out", "string", mt_string)
NODE_END()
