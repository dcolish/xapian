/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted
 provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this list of conditions
 and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 and the following disclaimer in the documentation and/or other materials provided with the distribution.

 - Neither the name of Technology Concepts & Design, Inc. nor the names of its contributors may be used to
 endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 **/
 
#include <jni.h>
#include <xapian.h>

#include "xapian_jni.h"

using namespace std;
using namespace Xapian;

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1new__ (JNIEnv *env, jclass clazz) {
    TRY
        Database *db = new Database();
        return _database->put(db);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1new__Ljava_lang_String_2 (JNIEnv *env, jclass clazz, jstring path) {
    TRY
        const char *c_path = env->GetStringUTFChars(path, 0);
        Database *db = new Database(Auto::open(c_path));
        env->ReleaseStringUTFChars(path, c_path);
        return _database->put(db);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1new__J (JNIEnv *env, jclass clazz, jlong dbid) {
   TRY
        Database *db = (Database *) _database->get(dbid);
        Database *new_db = new Database(*db);
        return _database->put(new_db);
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_database_1add_1database (JNIEnv *env, jclass clazz, jlong dbid, jlong dbid_other) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        Database *other_db = (Database *) _database->get(dbid_other);
        db->add_database(*other_db);
    CATCH(;)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_database_1reopen (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        db->reopen();
    CATCH(;)
}

JNIEXPORT jstring JNICALL Java_org_xapian_XapianJNI_database_1get_1description (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        return env->NewStringUTF(db->get_description().c_str());
    CATCH(NULL)
}

//
// TODO:  These have not been implemented yet!
//
JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1postlist_1begin (JNIEnv *env, jclass clazz, jlong dbid, jstring) {
    TRY
        throw "Database::postlist_begin() not (yet) supported by XapianJNI";
    CATCH(-1)
}
JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1postlist_1end (JNIEnv *env, jclass clazz, jlong dbid, jstring) {
    TRY
        throw "Database::postlist_end() not (yet) supported by XapianJNI";
    CATCH(-1)
}
JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1termlist_1begin (JNIEnv *env, jclass clazz, jlong dbid, jlong) {
    TRY
        throw "Database::termlist_begin() not (yet) supported by XapianJNI";
    CATCH(-1)
}
JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1termlist_1end (JNIEnv *env, jclass clazz, jlong dbid, jlong) {
    TRY
        throw "Database::termlist_end() not (yet) supported by XapianJNI";
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1positionlist_1begin (JNIEnv *env, jclass clazz, jlong dbid, jlong dbdocid, jstring term) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        const char *c_term = env->GetStringUTFChars(term, 0);
        PositionIterator *itr;
        itr = new PositionIterator(db->positionlist_begin(dbdocid, c_term));
        env->ReleaseStringUTFChars(term, c_term);
        return _positioniterator->put(itr);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1positionlist_1end (JNIEnv *env, jclass clazz, jlong dbid, jlong dbdocid, jstring term) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        const char *c_term = env->GetStringUTFChars(term, 0);
        PositionIterator *itr = new PositionIterator (db->positionlist_end(dbdocid, c_term));
        env->ReleaseStringUTFChars(term, c_term);
        return _positioniterator->put(itr);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1allterms_1begin (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        TermIterator *itr = new TermIterator(db->allterms_begin());
        return _termiterator->put(itr);
    CATCH(-1)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1allterms_1end (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        TermIterator *itr = new TermIterator(db->allterms_end());
        return _termiterator->put(itr);
    CATCH(-1)
}

JNIEXPORT jint JNICALL Java_org_xapian_XapianJNI_database_1get_1doccount (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        return db->get_doccount();
    CATCH(-1)
}

JNIEXPORT jdouble JNICALL Java_org_xapian_XapianJNI_database_1get_1avlength (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        return db->get_avlength();
    CATCH(-1)
}

JNIEXPORT jint JNICALL Java_org_xapian_XapianJNI_database_1get_1termfreq (JNIEnv *env, jclass clazz, jlong dbid, jstring term) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        const char *c_term = env->GetStringUTFChars(term, 0);
        int freq = db->get_termfreq(c_term);
        env->ReleaseStringUTFChars(term, c_term);
        return freq;
    CATCH(-1)
}

JNIEXPORT jboolean JNICALL Java_org_xapian_XapianJNI_database_1term_1exists (JNIEnv *env, jclass clazz, jlong dbid, jstring term) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        const char *c_term = env->GetStringUTFChars(term, 0);
        bool exists = db->term_exists(c_term);
        env->ReleaseStringUTFChars(term, c_term);
        return exists;
    CATCH(0)
}

JNIEXPORT jint JNICALL Java_org_xapian_XapianJNI_database_1get_1collection_1freq (JNIEnv *env, jclass clazz, jlong dbid, jstring term) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        const char *c_term = env->GetStringUTFChars(term, 0);
        int freq = db->get_collection_freq(c_term);
        env->ReleaseStringUTFChars(term, c_term);
        return freq;
    CATCH(-1)
}

JNIEXPORT jdouble JNICALL Java_org_xapian_XapianJNI_database_1get_1doclength (JNIEnv *env, jclass clazz, jlong dbid, jlong docid) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        return db->get_doclength(docid);
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_database_1keep_1alive (JNIEnv *env, jclass clazz, jlong dbid) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        db->keep_alive();
    CATCH(;)
}

JNIEXPORT jlong JNICALL Java_org_xapian_XapianJNI_database_1get_1document (JNIEnv *env, jclass clazz, jlong dbid, jlong docid) {
    TRY
        Database *db = (Database *) _database->get(dbid);
        Document *doc = new Document(db->get_document(docid));
        return _document->put(doc);
    CATCH(-1)
}

JNIEXPORT void JNICALL Java_org_xapian_XapianJNI_database_1finalize (JNIEnv *env, jclass clazz, jlong dbid) {
    Database *db = (Database *) _database->remove(dbid);
    if (db) delete db;
}
