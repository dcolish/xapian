/* omindexernode.cc: base class for the indexer network node.
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

#include "om/omindexernode.h"
#include "om/omerror.h"
#include "omdebug.h"
#include <typeinfo>

class OmIndexerNode::Internal {
    public:
	/** Constructor, taking a pointer to the holding OmIndexerNode */
	Internal(const OmSettings &settings_, OmIndexerNode *owner);

	/** Used by the graph builder to connect nodes together. */
	void connect_input(const std::string &input_name,
			   OmIndexerNode *other_node,
			   const std::string &other_outputname);

	/** Set a configuration value.  Used by the system to set
	 *  configuration values on behalf of the "user".
	 */
	void set_config_string(const std::string &key, const std::string &value);

	/** These output functions are used to pass the information between
	 *  the nodes.  Four possible message types are allowed:
	 *     int
	 *     double
	 *     string
	 *     Message, a structured record.
	 */
	/** Get a string output */
	std::string get_output_string(const std::string &output_name);

	/** Get an int output */
	int get_output_int(const std::string &output_name);

	/** Get a double output */
	double get_output_double(const std::string &output_name);

	/** Get a Record output */
	OmIndexerMessage get_output_record(const std::string &output_name);

	/** Used by node implementations to actually fetch data from the
	 *  input connections.  They can then be accessed by get_input_*.
	 */
	void request_inputs();

	/** Invalidate the outputs.  When an error occurs, or the network
	 *  needs to be reset, this function may be called.  It will cause
	 *  all outputs to be recalculated when next needed.
	 */
	void invalidate_outputs();

	/** Used by concrete node implementations to fetch the input as
	 *  a record or basic type.  The nodes connected to each input
	 *  may provide data directly in any of these formats.  The system
	 *  can automatically convert to or from the Record type if needed,
	 *  but not between basic types.
	 *
	 *  @param input_name	The name of the input connection to use.
	 */
	OmIndexerMessage get_input_record(const std::string &input_name);
	std::string get_input_string(const std::string &input_name);
	int get_input_int(const std::string &input_name);
	double get_input_double(const std::string &input_name);

	/** The functions which can be called from calculate() to set
	 *  the output data.  One of these should be called for each
	 *  provided output.  Data can be converted to or from the
	 *  generic Record type but not between basic types.
	 *
	 *  @param output_name  The name of the output connection
	 *
	 *  @param value	The value to provide to the input.
	 */
	void set_output(const std::string &output_name, int value);
	void set_output(const std::string &output_name, double value);
	void set_output(const std::string &output_name,
			const std::string &value);
	void set_output(const std::string &output_name,
			const OmIndexerMessage &value);

	/* The implementation's interface to the configuration data. */

	/** Return the current value of a given configuration parameter. */
	std::string get_config_string(const std::string &key) const;

	/** Return the current value of a given configuration parameter. */
	int get_config_int(const std::string &key) const;

	/** Return the current value of a given configuration parameter. */
	double get_config_double(const std::string &key) const;

#if 0
	/** Return the current value of a given configuration parameter. */
	std::vector<std::string>
		get_config_vector(const std::string &key) const;
#endif
    private:
	/* ********** Private member functions ************/
	/** Calculate outputs if needed
	 *  @param output_name  The output currently needed.  If this
	 *                      output is not available, then
	 *                      calculate() will be called.
	 */
	void calculate_if_needed(const std::string &output_name);
	
	/** Find the given input in the cached inputs and handle any
	 *  errors.
	 */
	OmIndexerMessage find_input_record(const std::string &input_name);

	/* ********** Private types ************/
	/** Description of inputs */
	struct input_desc {
	    /** The other node */
	    OmIndexerNode *node;
	    /** The other node's output name */
	    std::string node_output;
	};

	typedef unsigned int nodever_t;

	/* ********** Private data ************/
	/** The configuration strings */
	OmSettings settings;

	/** This node instance's ID */
	std::string id;

	/** The "version number" associated with the last calculate() */
	nodever_t version;

	/** The number of nodes in this graph which have more than one
	 *  output. */
	int num_splitting_nodes;

	/** The splitter number of this node, of -1 if it has only one
	 *  output.
	 */
	int my_split_id;

	/** A list of the last seen versions of the splitting nodes */
	std::vector<nodever_t> split_versions;

	/** The owning node (used for calling virtual methods) */
	OmIndexerNode *owner;

	typedef std::map<std::string, OmIndexerMessage> messagemap;

	/** Message outputs */
	messagemap outputs_record;

	/** The table of input connections */
	std::map<std::string, input_desc> inputs;

	/** The collected inputs */
	messagemap stored_inputs;
};

OmIndexerMessage
OmIndexerNode::get_output_record(const std::string &output_name)
{
    return internal->get_output_record(output_name);
}

OmIndexerMessage
OmIndexerNode::Internal::get_output_record(const std::string &output_name)
{
    calculate_if_needed(output_name);
    messagemap::iterator i;
    i = outputs_record.find(output_name);

    OmIndexerMessage result;

    // FIXME: check for validity of output_name in debugging code?

    if (i == outputs_record.end()) {
	// FIXME: OmInvalidArgumentError doesn't sound right here (and
	// below)...
	throw OmInvalidArgumentError(std::string("Request for output ") + 
				     output_name +
				     ", which wasn't calculated, from " +
				     typeid(*this).name());
    } else {
	result = i->second;
	outputs_record.erase(i);
    }
    DEBUGLINE(INDEXER, "Output " << id << "[" <<
	      output_name << "] = " << result);
    return result;
}

int
OmIndexerNode::get_output_int(const std::string &output_name)
{
    return internal->get_output_int(output_name);
}

int
OmIndexerNode::Internal::get_output_int(const std::string &output_name)
{
    calculate_if_needed(output_name);
    messagemap::iterator i;
    i = outputs_record.find(output_name);

    int result;

    // FIXME: check for validity of output_name in debugging code?

    if (i == outputs_record.end()) {
	throw OmInvalidArgumentError(std::string("Request for output ") + 
		output_name + ", which wasn't calculated, from " +
		typeid(*this).name());
    } else {
	if (i->second.get_type() != OmIndexerMessage::rt_int) {
	    throw OmTypeError(std::string("Attempt to convert a non-int output (") + output_name + ") into a int");
	}
	result = i->second.get_int();
	outputs_record.erase(i);
    }
    DEBUGLINE(INDEXER, "Output " << id << "[" <<
	      output_name << "] = " << result);
    return result;
}

double
OmIndexerNode::get_output_double(const std::string &output_name)
{
    return internal->get_output_double(output_name);
}

double
OmIndexerNode::Internal::get_output_double(const std::string &output_name)
{
    calculate_if_needed(output_name);
    messagemap::iterator i;
    i = outputs_record.find(output_name);

    double result;

    // FIXME: check for validity of output_name in debugging code?

    if (i == outputs_record.end()) {
	throw OmInvalidArgumentError(std::string("Request for output ") + 
		output_name + ", which wasn't calculated, from " +
		typeid(*this).name());
    } else {
	if (i->second.get_type() != OmIndexerMessage::rt_double) {
	    throw OmTypeError(std::string("Attempt to convert a non-double output (") + output_name + ") into a double");
	}
	result = i->second.get_double();
	outputs_record.erase(i);
    }
    DEBUGLINE(INDEXER, "Output " << id << "[" <<
	      output_name << "] = " << result);
    return result;
}

std::string
OmIndexerNode::get_output_string(const std::string &output_name)
{
    return internal->get_output_string(output_name);
}

std::string
OmIndexerNode::Internal::get_output_string(const std::string &output_name)
{
    calculate_if_needed(output_name);
    messagemap::iterator i;
    i = outputs_record.find(output_name);

    std::string result;

    // FIXME: check for validity of output_name in debugging code?

    if (i == outputs_record.end()) {
	throw OmInvalidArgumentError(std::string("Request for output ") + 
		output_name + ", which wasn't calculated, from " +
		typeid(*this).name());
    } else {
	if (i->second.get_type() != OmIndexerMessage::rt_string) {
	    throw OmTypeError(std::string("Attempt to convert a non-string output (") + output_name + ") into a string");
	}
	result = i->second.get_string();
	outputs_record.erase(i);
    }
    DEBUGLINE(INDEXER, "Output " << id << "[" <<
	      output_name << "] = " << result);
    return result;
}

void
OmIndexerNode::Internal::calculate_if_needed(const std::string &output_name)
{
    messagemap::iterator i;
    i = outputs_record.find(output_name);

    if (i == outputs_record.end()) {
	outputs_record.clear();
	// start with no inputs.
	stored_inputs.clear();
	DEBUGLINE(INDEXER, "Calculating " << typeid(*owner).name() <<
		           " (output " << output_name << " requested)");
	/*
	cerr << "Calculating " << typeid(*owner).name() <<
		           " (output " << output_name << " requested)" << endl;
	 */
	// increment the version number of our results
	version++;
	//cerr << "[" << id << "]: incrementing version to " << version << endl;
	//cerr << "My split_id = " << my_split_id << endl;

	owner->calculate();

	if (my_split_id >= 0) {
	    split_versions[my_split_id] = version;
	}
	/*
	cerr << "Calculated!" << endl;
	 */
    }
}

OmIndexerNode::OmIndexerNode(const OmSettings &settings_)
	: internal(new Internal(settings_, this))
{
}

OmIndexerNode::Internal::Internal(const OmSettings &settings_,
				  OmIndexerNode *owner_)
	: settings(settings_),
	  id(settings_.get("omindexer_node_id")),
	  version(0),
	  num_splitting_nodes(settings.get_int("omindexer_numsplitting")),
	  my_split_id(settings.get_int("omindexer_mysplitid", -1)),
	  split_versions(num_splitting_nodes),
	  owner(owner_)
{
    for (int i=0; i<num_splitting_nodes; ++i) {
	split_versions[i] = 0;
    }
}

OmIndexerNode::~OmIndexerNode()
{
    delete internal;
}

void
OmIndexerNode::config_modified(const std::string &key)
{
}

void
OmIndexerNode::connect_input(const std::string &input_name,
			     OmIndexerNode *other_node,
			     const std::string &other_outputname)
{
    return internal->connect_input(input_name,
				   other_node,
				   other_outputname);
}

void
OmIndexerNode::Internal::connect_input(const std::string &input_name,
			     OmIndexerNode *other_node,
			     const std::string &other_outputname)
{
    input_desc con;
    con.node = other_node;
    con.node_output = other_outputname;

    inputs[input_name] = con;
}

void OmIndexerNode::set_empty_output(const std::string &output_name)
{
    internal->set_output(output_name, OmIndexerMessage());
}

void OmIndexerNode::set_output(const std::string &output_name,
			       const OmIndexerMessage &value)
{
    return internal->set_output(output_name, value);
}

void
OmIndexerNode::Internal::set_output(const std::string &output_name,
				    const OmIndexerMessage &value)
{
    /*cout << "Setting output \"" << output_name
	 << "\" to record:" << value << endl; */
    // TODO: check that it isn't already set?
    outputs_record[output_name] = value;
}

void
OmIndexerNode::set_output(const std::string &output_name,
				      const std::string &value)
{
    return internal->set_output(output_name, value);
}

void
OmIndexerNode::Internal::set_output(const std::string &output_name,
				      const std::string &value)
{
    /*cout << "Setting output \"" << output_name
	 << "\" to record:" << value << endl; */
    // TODO: check that it isn't already set?
    outputs_record[output_name] = value;
}

void
OmIndexerNode::set_output(const std::string &output_name,
			  int value)
{
    return internal->set_output(output_name, value);
}

void
OmIndexerNode::Internal::set_output(const std::string &output_name,
				    int value)
{
    /*cout << "Setting output \"" << output_name
	 << "\" to record:" << value << endl; */
    // TODO: check that it isn't already set?
    outputs_record[output_name] = value;
}

void
OmIndexerNode::set_output(const std::string &output_name,
			  double value)
{
    return internal->set_output(output_name, value);
}

void
OmIndexerNode::Internal::set_output(const std::string &output_name,
				    double value)
{
    /*cout << "Setting output \"" << output_name
	 << "\" to record:" << value << endl; */
    // TODO: check that it isn't already set?
    outputs_record[output_name] = value;
}

std::string
OmIndexerNode::get_config_string(const std::string &key) const
{
    return internal->get_config_string(key);
}

int
OmIndexerNode::get_config_int(const std::string &key) const
{
    return internal->get_config_int(key);
}

double
OmIndexerNode::get_config_double(const std::string &key) const
{
    return internal->get_config_double(key);
}

#if 0
std::vector<std::string>
OmIndexerNode::get_config_vector(const std::string &key) const
{
    return internal->get_config_vector(key);
}
#endif

std::string
OmIndexerNode::Internal::get_config_string(const std::string &key) const
{
    return settings.get(key);
}

int
OmIndexerNode::Internal::get_config_int(const std::string &key) const
{
    return settings.get_int(key);
}

double
OmIndexerNode::Internal::get_config_double(const std::string &key) const
{
    return settings.get_real(key);
}

#if 0
std::vector<std::string>
OmIndexerNode::Internal::get_config_vector(const std::string &key) const
{
    return settings.get_vector(key);
}
#endif

void
OmIndexerNode::set_config_string(const std::string &key,
				 const std::string &value)
{
    return internal->set_config_string(key, value);
}

void
OmIndexerNode::Internal::set_config_string(const std::string &key,
				 const std::string &value)
{
    settings.set(key, value);
    owner->config_modified(key);
}

OmIndexerMessage
OmIndexerNode::get_input_record(const std::string &input_name)
{
    return internal->get_input_record(input_name);
}

OmIndexerMessage
OmIndexerNode::Internal::find_input_record(const std::string &input_name)
{
    messagemap::iterator val;
    val = stored_inputs.find(input_name);
    if (val == stored_inputs.end()) {
	throw OmInvalidArgumentError(std::string("Input ") +
				     input_name + " was asked for more than once or before request_inputs()");
    }
    OmIndexerMessage retval = val->second;
    stored_inputs.erase(val);
    return retval;
}

OmIndexerMessage
OmIndexerNode::Internal::get_input_record(const std::string &input_name)
{
    std::map<std::string, input_desc>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw OmInvalidArgumentError(std::string("Unknown input ") +
				     id + "[" + input_name + "]");
    } else {
	return find_input_record(input_name);
    }
}

std::string
OmIndexerNode::get_input_string(const std::string &input_name)
{
    return internal->get_input_string(input_name);
}

std::string
OmIndexerNode::Internal::get_input_string(const std::string &input_name)
{
    std::map<std::string, input_desc>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw OmInvalidArgumentError(std::string("Unknown input ") +
				     id + "[" + input_name + "]");
    } else {
	return find_input_record(input_name).get_string();
    }
}

int
OmIndexerNode::get_input_int(const std::string &input_name)
{
    return internal->get_input_int(input_name);
}

int
OmIndexerNode::Internal::get_input_int(const std::string &input_name)
{
    std::map<std::string, input_desc>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw OmInvalidArgumentError(std::string("Unknown input ") +
				     id + "[" + input_name + "]");
    } else {
	return find_input_record(input_name).get_int();
    }
}

double
OmIndexerNode::get_input_double(const std::string &input_name)
{
    return internal->get_input_double(input_name);
}

double
OmIndexerNode::Internal::get_input_double(const std::string &input_name)
{
    std::map<std::string, input_desc>::const_iterator i;
    i = inputs.find(input_name);

    if (i == inputs.end()) {
	throw OmInvalidArgumentError(std::string("Unknown input ") +
				     id + "[" + input_name + "]");
    } else {
	return find_input_record(input_name).get_double();
    }
}

void
OmIndexerNode::invalidate_outputs()
{
    return internal->invalidate_outputs();
}

void
OmIndexerNode::Internal::invalidate_outputs()
{
    outputs_record.clear();
}

void
OmIndexerNode::request_inputs()
{
    internal->request_inputs();
}

void OmIndexerNode::Internal::request_inputs()
{
    // dump the old inputs, if any
    stored_inputs.clear();

    // start our seen versions again
    for (int i=0; i<num_splitting_nodes; ++i) {
	split_versions[i] = 0;
    }

    std::map<std::string, input_desc>::const_iterator i;
    for (i=inputs.begin(); i!=inputs.end(); ++i) {
	// Physically fetch the input
	stored_inputs[i->first] = 
		i->second.node->get_output_record(i->second.node_output);
	
	/* Check that we're not merging results which come from two different
	 * calculations of the same node
	 */
	/*
	cerr << "[" << id << "] Merging " << num_splitting_nodes
		<< " inputs: ";
	 */
	for (int num=0; num<num_splitting_nodes; ++num) {
	    if (split_versions[num] == 0) {
		// take the version from the input, as we haven't seen it yet.
		split_versions[num] =
			i->second.node->internal->split_versions[num];
	    } else if (i->second.node->internal->split_versions[num] != 0
		       && i->second.node->internal->split_versions[num] !=
		          split_versions[num]) {
		std::string message = "Detected flow problem: ";
		message += "splitting node ";
		message += om_tostring(num);
		message += " versions ";
		message += om_tostring(split_versions[num]);
		message += " vs ";
		message += om_tostring(i->second.node->internal->split_versions[num]);
		// FIXME: find enough information to identify the node
		// causing the conflict, then have a useful message.
		throw OmDataFlowError(message);
	    }
	    /*
	    cerr << split_versions[num];
	    */
	}
	/*
	cerr << endl;
	*/
    }
}
