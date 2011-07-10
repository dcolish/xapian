noinst_HEADERS +=\
	spelling/editdistance.h\
	spelling/extended_edit_distance.h\
	spelling/spelling_base.h\
	spelling/spelling_corrector.h\
	spelling/spelling_splitter.h\
	spelling/spelling_keyboard.h\
	spelling/spelling_keyboard_layouts.h\
	spelling/spelling_phonetic.h\
	spelling/spelling_phonetic_dmsoundex.h\
	spelling/spelling_phonetic_metaphone.h\
	spelling/spelling_transliteration.h\
	spelling/spelling_transliteration_alphabets.h

EXTRA_DIST +=\
	spelling/dir_contents\
	spelling/Makefile

lib_src +=\
	spelling/editdistance.cc\
	spelling/extended_edit_distance.cc\
	spelling/spelling_base.cc\
	spelling/spelling_corrector.cc\
	spelling/spelling_splitter.cc\
	spelling/spelling_keyboard.cc\
	spelling/spelling_phonetic_dmsoundex.cc\
	spelling/spelling_phonetic_metaphone.cc\
	spelling/spelling_transliteration.cc
