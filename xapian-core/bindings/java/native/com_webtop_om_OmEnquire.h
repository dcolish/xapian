/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_webtop_om_OmEnquire */

#ifndef _Included_com_webtop_om_OmEnquire
#define _Included_com_webtop_om_OmEnquire
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_webtop_om_OmEnquire
 * Method:    createNativeObject
 * Signature: (Lcom/webtop/om/OmDatabaseGroup;)J
 */
JNIEXPORT jlong JNICALL Java_com_webtop_om_OmEnquire_createNativeObject
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_webtop_om_OmEnquire
 * Method:    deleteNativeObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmEnquire_deleteNativeObject
  (JNIEnv *, jobject);

/*
 * Class:     com_webtop_om_OmEnquire
 * Method:    get_doc
 * Signature: (Lcom/webtop/om/OmMSetItem;)Lcom/webtop/om/OmDocument;
 */
JNIEXPORT jobject JNICALL Java_com_webtop_om_OmEnquire_get_1doc__Lcom_webtop_om_OmMSetItem_2
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_webtop_om_OmEnquire
 * Method:    get_doc
 * Signature: (I)Lcom/webtop/om/OmDocument;
 */
JNIEXPORT jobject JNICALL Java_com_webtop_om_OmEnquire_get_1doc__I
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_webtop_om_OmEnquire
 * Method:    get_mset
 * Signature: (IILcom/webtop/om/OmRSet;Lcom/webtop/om/OmMatchOptions;Lcom/webtop/om/OmMatchDecider;)Lcom/webtop/om/OmMSet;
 */
JNIEXPORT jobject JNICALL Java_com_webtop_om_OmEnquire_get_1mset
  (JNIEnv *, jobject, jint, jint, jobject, jobject, jobject);

/*
 * Class:     com_webtop_om_OmEnquire
 * Method:    set_query
 * Signature: (Lcom/webtop/om/OmQuery;)V
 */
JNIEXPORT void JNICALL Java_com_webtop_om_OmEnquire_set_1query
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_webtop_om_OmEnquire
 * Method:    get_eset
 * Signature: (ILcom/webtop/om/OmRSet;Lcom/webtop/om/OmExpandOptions;Lcom/webtop/om/OmExpandDecider;)Lcom/webtop/om/OmESet;
 */
JNIEXPORT jobject JNICALL Java_com_webtop_om_OmEnquire_get_1eset
  (JNIEnv *, jobject, jint, jobject, jobject, jobject);

/*
 * Class:     com_webtop_om_OmEnquire
 * Method:    get_matching_terms
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_webtop_om_OmEnquire_get_1matching_1terms
  (JNIEnv *, jobject, jint);

#ifdef __cplusplus
}
#endif
#endif
