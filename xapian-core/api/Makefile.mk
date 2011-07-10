noinst_HEADERS +=\
	api/documentvaluelist.h\
	api/editdistance.h\
	api/extended_edit_distance.h\
	api/spelling_base.h\
	api/spelling_corrector.h\
	api/spelling_splitter.h\
	api/spelling_keyboard.h\
	api/spelling_keyboard_layouts.h\
	api/spelling_phonetic.h\
	api/spelling_phonetic_dmsoundex.h\
	api/spelling_phonetic_metaphone.h\
	api/spelling_transliteration.h\
	api/spelling_transliteration_alphabets.h \
	api/maptermlist.h

EXTRA_DIST +=\
	api/dir_contents\
	api/Makefile

lib_src +=\
	api/compactor.cc\
	api/decvalwtsource.cc\
	api/documentvaluelist.cc\
	api/editdistance.cc\
	api/extended_edit_distance.cc\
	api/spelling_base.cc\
	api/spelling_corrector.cc\
	api/spelling_splitter.cc\
	api/spelling_keyboard.cc\
	api/spelling_phonetic_dmsoundex.cc\
	api/spelling_phonetic_metaphone.cc\
	api/spelling_transliteration.cc\
	api/emptypostlist.cc\
	api/error.cc\
	api/errorhandler.cc\
	api/expanddecider.cc\
	api/keymaker.cc\
	api/leafpostlist.cc\
	api/matchspy.cc\
	api/omdatabase.cc\
	api/omdocument.cc\
	api/omenquire.cc\
	api/ompositionlistiterator.cc\
	api/ompostlistiterator.cc\
	api/omquery.cc\
	api/omqueryinternal.cc\
	api/omtermlistiterator.cc\
	api/postingsource.cc\
	api/postlist.cc\
	api/registry.cc\
	api/replication.cc\
	api/sortable-serialise.cc\
	api/termlist.cc\
	api/valueiterator.cc\
	api/valuerangeproc.cc\
	api/valuesetmatchdecider.cc\
	api/version.cc
