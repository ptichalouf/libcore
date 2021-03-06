/**
*******************************************************************************
* Copyright (C) 1996-2005, International Business Machines Corporation and    *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
*******************************************************************************
*/

#define LOG_TAG "NativeCollation"

#include "JNIHelp.h"
#include "JniConstants.h"
#include "JniException.h"
#include "ScopedStringChars.h"
#include "ScopedUtfChars.h"
#include "UniquePtr.h"
#include "ucol_imp.h"
#include "unicode/ucol.h"
#include "unicode/ucoleitr.h"

static UCollator* toCollator(jint address) {
    return reinterpret_cast<UCollator*>(static_cast<uintptr_t>(address));
}

static UCollationElements* toCollationElements(jint address) {
    return reinterpret_cast<UCollationElements*>(static_cast<uintptr_t>(address));
}

static void NativeCollation_closeCollator(JNIEnv*, jclass, jint address) {
    ucol_close(toCollator(address));
}

static void NativeCollation_closeElements(JNIEnv*, jclass, jint address) {
    ucol_closeElements(toCollationElements(address));
}

static jint NativeCollation_compare(JNIEnv* env, jclass, jint address, jstring javaLhs, jstring javaRhs) {
    ScopedStringChars lhs(env, javaLhs);
    if (lhs.get() == NULL) {
        return 0;
    }
    ScopedStringChars rhs(env, javaRhs);
    if (rhs.get() == NULL) {
        return 0;
    }
    return ucol_strcoll(toCollator(address), lhs.get(), lhs.size(), rhs.get(), rhs.size());
}

static jint NativeCollation_getAttribute(JNIEnv* env, jclass, jint address, jint type) {
    UErrorCode status = U_ZERO_ERROR;
    jint result = ucol_getAttribute(toCollator(address), (UColAttribute) type, &status);
    maybeThrowIcuException(env, status);
    return result;
}

static jint NativeCollation_getCollationElementIterator(JNIEnv* env, jclass, jint address, jstring javaSource) {
    ScopedStringChars source(env, javaSource);
    if (source.get() == NULL) {
        return -1;
    }
    UErrorCode status = U_ZERO_ERROR;
    UCollationElements* result = ucol_openElements(toCollator(address), source.get(), source.size(), &status);
    maybeThrowIcuException(env, status);
    return static_cast<jint>(reinterpret_cast<uintptr_t>(result));
}

static jint NativeCollation_getMaxExpansion(JNIEnv*, jclass, jint address, jint order) {
    return ucol_getMaxExpansion(toCollationElements(address), order);
}

static jint NativeCollation_getOffset(JNIEnv*, jclass, jint address) {
    return ucol_getOffset(toCollationElements(address));
}

static jstring NativeCollation_getRules(JNIEnv* env, jclass, jint address) {
    int32_t length = 0;
    const UChar* rules = ucol_getRules(toCollator(address), &length);
    return env->NewString(rules, length);
}

static jbyteArray NativeCollation_getSortKey(JNIEnv* env, jclass, jint address, jstring javaSource) {
    ScopedStringChars source(env, javaSource);
    if (source.get() == NULL) {
        return NULL;
    }
    const UCollator* collator  = toCollator(address);
    uint8_t byteArray[UCOL_MAX_BUFFER * 2];
    UniquePtr<uint8_t[]> largerByteArray;
    uint8_t* usedByteArray = byteArray;
    size_t byteArraySize = ucol_getSortKey(collator, source.get(), source.size(), usedByteArray, sizeof(byteArray) - 1);
    if (byteArraySize > sizeof(byteArray) - 1) {
        // didn't fit, try again with a larger buffer.
        largerByteArray.reset(new uint8_t[byteArraySize + 1]);
        usedByteArray = largerByteArray.get();
        byteArraySize = ucol_getSortKey(collator, source.get(), source.size(), usedByteArray, byteArraySize);
    }
    if (byteArraySize == 0) {
        return NULL;
    }
    jbyteArray result = env->NewByteArray(byteArraySize);
    env->SetByteArrayRegion(result, 0, byteArraySize, reinterpret_cast<jbyte*>(usedByteArray));
    return result;
}

static jint NativeCollation_next(JNIEnv* env, jclass, jint address) {
    UErrorCode status = U_ZERO_ERROR;
    jint result = ucol_next(toCollationElements(address), &status);
    maybeThrowIcuException(env, status);
    return result;
}

static jint NativeCollation_openCollator(JNIEnv* env, jclass, jstring localeName) {
    ScopedUtfChars localeChars(env, localeName);
    if (localeChars.c_str() == NULL) {
        return 0;
    }
    UErrorCode status = U_ZERO_ERROR;
    UCollator* c = ucol_open(localeChars.c_str(), &status);
    maybeThrowIcuException(env, status);
    return static_cast<jint>(reinterpret_cast<uintptr_t>(c));
}

static jint NativeCollation_openCollatorFromRules(JNIEnv* env, jclass, jstring javaRules, jint mode, jint strength) {
    ScopedStringChars rules(env, javaRules);
    if (rules.get() == NULL) {
        return -1;
    }
    UErrorCode status = U_ZERO_ERROR;
    UCollator* c = ucol_openRules(rules.get(), rules.size(),
            UColAttributeValue(mode), UCollationStrength(strength), NULL, &status);
    maybeThrowIcuException(env, status);
    return static_cast<jint>(reinterpret_cast<uintptr_t>(c));
}

static jint NativeCollation_previous(JNIEnv* env, jclass, jint address) {
    UErrorCode status = U_ZERO_ERROR;
    jint result = ucol_previous(toCollationElements(address), &status);
    maybeThrowIcuException(env, status);
    return result;
}

static void NativeCollation_reset(JNIEnv*, jclass, jint address) {
    ucol_reset(toCollationElements(address));
}

static jint NativeCollation_safeClone(JNIEnv* env, jclass, jint address) {
    UErrorCode status = U_ZERO_ERROR;
    jint bufferSize = U_COL_SAFECLONE_BUFFERSIZE;
    UCollator* c = ucol_safeClone(toCollator(address), NULL, &bufferSize, &status);
    maybeThrowIcuException(env, status);
    return static_cast<jint>(reinterpret_cast<uintptr_t>(c));
}

static void NativeCollation_setAttribute(JNIEnv* env, jclass, jint address, jint type, jint value) {
    UErrorCode status = U_ZERO_ERROR;
    ucol_setAttribute(toCollator(address), (UColAttribute)type, (UColAttributeValue)value, &status);
    maybeThrowIcuException(env, status);
}

static void NativeCollation_setOffset(JNIEnv* env, jclass, jint address, jint offset) {
    UErrorCode status = U_ZERO_ERROR;
    ucol_setOffset(toCollationElements(address), offset, &status);
    maybeThrowIcuException(env, status);
}

static void NativeCollation_setText(JNIEnv* env, jclass, jint address, jstring javaSource) {
    ScopedStringChars source(env, javaSource);
    if (source.get() == NULL) {
        return;
    }
    UErrorCode status = U_ZERO_ERROR;
    ucol_setText(toCollationElements(address), source.get(), source.size(), &status);
    maybeThrowIcuException(env, status);
}

static JNINativeMethod gMethods[] = {
    NATIVE_METHOD(NativeCollation, closeCollator, "(I)V"),
    NATIVE_METHOD(NativeCollation, closeElements, "(I)V"),
    NATIVE_METHOD(NativeCollation, compare, "(ILjava/lang/String;Ljava/lang/String;)I"),
    NATIVE_METHOD(NativeCollation, getAttribute, "(II)I"),
    NATIVE_METHOD(NativeCollation, getCollationElementIterator, "(ILjava/lang/String;)I"),
    NATIVE_METHOD(NativeCollation, getMaxExpansion, "(II)I"),
    NATIVE_METHOD(NativeCollation, getOffset, "(I)I"),
    NATIVE_METHOD(NativeCollation, getRules, "(I)Ljava/lang/String;"),
    NATIVE_METHOD(NativeCollation, getSortKey, "(ILjava/lang/String;)[B"),
    NATIVE_METHOD(NativeCollation, next, "(I)I"),
    NATIVE_METHOD(NativeCollation, openCollator, "(Ljava/lang/String;)I"),
    NATIVE_METHOD(NativeCollation, openCollatorFromRules, "(Ljava/lang/String;II)I"),
    NATIVE_METHOD(NativeCollation, previous, "(I)I"),
    NATIVE_METHOD(NativeCollation, reset, "(I)V"),
    NATIVE_METHOD(NativeCollation, safeClone, "(I)I"),
    NATIVE_METHOD(NativeCollation, setAttribute, "(III)V"),
    NATIVE_METHOD(NativeCollation, setOffset, "(II)V"),
    NATIVE_METHOD(NativeCollation, setText, "(ILjava/lang/String;)V"),
};
void register_libcore_icu_NativeCollation(JNIEnv* env) {
    jniRegisterNativeMethods(env, "libcore/icu/NativeCollation", gMethods, NELEM(gMethods));
}
