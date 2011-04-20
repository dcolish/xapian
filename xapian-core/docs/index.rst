
Xapian - the open source search engine
======================================

A number of pieces of documentation are available.
We suggest you start by reading the :doc:`Installation
Guide </install>`, which covers downloading the code, and
unpacking, configuring, building and installing it.

For a quick introduction to our software, including a walk-through
example of an application for searching through some data, read the
:doc:`Quickstart </quickstart>` document.

The :doc:`glossary </glossary>` provides definitions for specialized
terminology you might encounter while using Xapian.

The :doc:`Overview </overview>` explains the API which Xapian provides
to programmers. A full `API Reference <apidoc/html/index.html>`_ is
automatically collated using doxygen from documentation comments in the
source code. There's also a list of :doc:`deprecated features </deprecation>`
which lists features scheduled for removal, and also features already removed,
along with suggestions for replacements.

If you want to learn more about probabilistic information retrieval,
there's a (reasonably mathematical) :doc:`introduction to the ideas behind
Xapian </intro_ir>` which also suggests some books you might want
to read.

There are a number of documents which cover particular features:

-  :doc:`BM25 Weighting Scheme </bm25>`
-  :doc:`Collapsing </collapsing>`
-  :doc:`Database Replication </replication>`
-  :doc:`Faceting </facets>`
-  :doc:`Indexing </termgenerator>`
-  :doc:`PostingSource </postingsource>`
-  :doc:`Query Parser </queryparser>`
-  :doc:`Remote Backend </remote>`
-  :doc:`Serialising Queries and Documents </serialisation>`
-  :doc:`Sorting Results </sorting>`
-  :doc:`Spelling Correction </spelling>`
-  :doc:`Stemming Algorithms </stemming>`
-  :doc:`Synonym Support </synonyms>`
-  :doc:`Value Ranges </valueranges>`

For those wishing to do development work on the Xapian library itself,
there is :doc:`documentation of Xapian's internals </internals>`
available.

We also have a `wiki <http://trac.xapian.org/wiki>`_ for documentation
and examples contributed by the Xapian community.

.. toctree::
   :hidden:
   :numbered:

   install
   quickstart
   admin_notes
   scalability
   intro_ir
   bm25
   collapsing
   replication
   facets
   termgenerator
   postingsource
   queryparser
   remote
   serialisation
   sorting
   spelling
   stemming
   synonyms
   valueranges
   internals
   overview
   code_structure
   matcherdesign
   remote_protocol
   replication_protocol
   tests
   deprecation
   glossary
             
