/* omindexerdescinternal.h: An intermediate form for the graph description
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_OMINDEXERDESCINTERNAL_H
#define OM_HGUARD_OMINDEXERDESCINTERNAL_H

#include <string>
#include <vector>
#include "om/omsettings.h"
#include "om/omindexerdesc.h"
#include "refcnt.h"

/** An intermediate form for the node graphs.
 *  This is a description of an indexer graph which can be used instead
 *  of an XML file to build an indexer.  It contains the same information.
 */
class OmIndexerDesc::Internal {
    public:
	Internal() : data(new Data) {}
	/** Connect describes a node's connection to other nodes. */
	struct Connect {
	    /** The name of the input being described. */
	    std::string input_name;
	    /** The id of the node this input is connected to. */
	    std::string feeder_node;
	    /** The feeder's output connection */
	    std::string feeder_out;
	};
	/** NodeInstance contains all the information about a particular node
	 *  in the graph.
	 */
	struct NodeInstance {
	    /** The type of the node.  This must be one of the registered node
	     *  types.
	     */
	    std::string type;
	    /** This node's id, which must be unique. */
	    std::string id;

	    /** This node's input connections */
	    std::vector<Connect> input;

	    /** This node's initial parameters */
	    OmSettings param;
	};

	struct Data : public RefCntBase {
	    /** The information about all the nodes in the graph */
	    std::vector<NodeInstance> nodes;

	    /** The id of the node from which the final results come */
	    std::string output_node;

	    /** The name of the output connection to use on the final node. */
	    std::string output_pad;
	};

	RefCntPtr<Data> data;
};

#endif /* OM_HGUARD_OMINDEXERDESCINTERNAL_H */
