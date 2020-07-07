//******************************************************************************
// Copyright (c) Microsoft Corporation.  All rights reserved.
// spgeterrormsg.cpp
//
//******************************************************************************

#include "ttscomp.h"

#pragma warning (disable : 4786) 
#include <map>

typedef std::map<HRESULT, LPCSTR> HRMAP;
typedef HRMAP::value_type HRPAIR;

void InitSpErrorMsg(HRMAP& rhrmap);

inline LPCSTR SpGetErrorMsg(HRESULT hr)
{
	static char szMessageBuffer[MAX_PATH];
    static bool s_fInit;
    static HRMAP s_hrmap;

	if(!s_fInit) 
    {
        InitSpErrorMsg(s_hrmap);
        s_fInit = true;
    }

    HRMAP::iterator it = s_hrmap.find(hr);
    if(it != s_hrmap.end())
    {
        return it->second;
    }
    else if(FAILED(hr)) // FormatMessage will treat SUCCEEDED(hr) as Win32 error code
    {
		DWORD cbWrite = ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), szMessageBuffer, 256, NULL);
		if(cbWrite > 0)
		{
			// truncate the ending CRLF
			szMessageBuffer[strlen(szMessageBuffer) - 2] = 0;
			return szMessageBuffer;
		}
    }

    sprintf(szMessageBuffer, "0x%08x", hr);
    return szMessageBuffer;
}

inline void InitSpErrorMsg(HRMAP& rhrmap)
{
#define ADD_HR_PAIR(hr) rhrmap.insert(HRPAIR(hr, #hr))
#define ADD_WIN32_PAIR(errno) rhrmap.insert(HRPAIR(SpHrFromWin32(errno), "SpHrFromWin32("#errno")"))
    ADD_HR_PAIR(S_OK);
    ADD_HR_PAIR(S_FALSE);

    // If we have two names for the same hr, the first one inserted will count.
    // So the SAPI names are put before standard names.

	// The following error codes are based on sperror.h#10
    // Note: Please update the version number above when new codes added.
    ADD_HR_PAIR(SPERR_UNINITIALIZED);
    ADD_HR_PAIR(SPERR_ALREADY_INITIALIZED);
    ADD_HR_PAIR(SPERR_UNSUPPORTED_FORMAT);
    ADD_HR_PAIR(SPERR_INVALID_FLAGS);
    ADD_HR_PAIR(SP_END_OF_STREAM);
    ADD_HR_PAIR(SPERR_DEVICE_BUSY);
    ADD_HR_PAIR(SPERR_DEVICE_NOT_SUPPORTED);
    ADD_HR_PAIR(SPERR_DEVICE_NOT_ENABLED);
    ADD_HR_PAIR(SPERR_NO_DRIVER);
    ADD_HR_PAIR(SPERR_FILE_MUST_BE_UNICODE);
    ADD_HR_PAIR(SP_INSUFFICIENT_DATA);
    ADD_HR_PAIR(SPERR_INVALID_PHRASE_ID);
    ADD_HR_PAIR(SPERR_BUFFER_TOO_SMALL);
    ADD_HR_PAIR(SPERR_FORMAT_NOT_SPECIFIED);
    ADD_HR_PAIR(SPERR_AUDIO_STOPPED);
    ADD_HR_PAIR(SP_AUDIO_PAUSED);
    ADD_HR_PAIR(SPERR_RULE_NOT_FOUND);
    ADD_HR_PAIR(SPERR_TTS_ENGINE_EXCEPTION);
    ADD_HR_PAIR(SPERR_TTS_NLP_EXCEPTION);
    ADD_HR_PAIR(SPERR_ENGINE_BUSY);
    ADD_HR_PAIR(SP_AUDIO_CONVERSION_ENABLED);
    ADD_HR_PAIR(SP_NO_HYPOTHESIS_AVAILABLE);
    ADD_HR_PAIR(SPERR_CANT_CREATE);
    ADD_HR_PAIR(SP_ALREADY_IN_LEX);
    ADD_HR_PAIR(SPERR_NOT_IN_LEX);
    ADD_HR_PAIR(SP_LEX_NOTHING_TO_SYNC);
    ADD_HR_PAIR(SPERR_LEX_VERY_OUT_OF_SYNC);
    ADD_HR_PAIR(SPERR_UNDEFINED_FORWARD_RULE_REF);
    ADD_HR_PAIR(SPERR_EMPTY_RULE);
    ADD_HR_PAIR(SPERR_GRAMMAR_COMPILER_INTERNAL_ERROR);
    ADD_HR_PAIR(SPERR_RULE_NOT_DYNAMIC);
    ADD_HR_PAIR(SPERR_DUPLICATE_RULE_NAME);
    ADD_HR_PAIR(SPERR_DUPLICATE_RESOURCE_NAME);
    ADD_HR_PAIR(SPERR_TOO_MANY_GRAMMARS);
    ADD_HR_PAIR(SPERR_CIRCULAR_REFERENCE);
    ADD_HR_PAIR(SPERR_INVALID_IMPORT);
    ADD_HR_PAIR(SPERR_INVALID_WAV_FILE);
    ADD_HR_PAIR(SP_REQUEST_PENDING);
    ADD_HR_PAIR(SPERR_ALL_WORDS_OPTIONAL);
    ADD_HR_PAIR(SPERR_INSTANCE_CHANGE_INVALID);
    ADD_HR_PAIR(SPERR_RULE_NAME_ID_CONFLICT);
    ADD_HR_PAIR(SPERR_NO_RULES);
    ADD_HR_PAIR(SPERR_CIRCULAR_RULE_REF);
    ADD_HR_PAIR(SP_NO_PARSE_FOUND);
    ADD_HR_PAIR(SPERR_INVALID_HANDLE);
    ADD_HR_PAIR(SPERR_REMOTE_CALL_TIMED_OUT);
    ADD_HR_PAIR(SPERR_AUDIO_BUFFER_OVERFLOW);
    ADD_HR_PAIR(SPERR_NO_AUDIO_DATA);
    ADD_HR_PAIR(SPERR_DEAD_ALTERNATE);
    ADD_HR_PAIR(SPERR_HIGH_LOW_CONFIDENCE);
    ADD_HR_PAIR(SPERR_INVALID_FORMAT_STRING);
    ADD_HR_PAIR(SP_UNSUPPORTED_ON_STREAM_INPUT);
    ADD_HR_PAIR(SPERR_APPLEX_READ_ONLY);
    ADD_HR_PAIR(SPERR_NO_TERMINATING_RULE_PATH);
    ADD_HR_PAIR(SP_WORD_EXISTS_WITHOUT_PRONUNCIATION);
    ADD_HR_PAIR(SPERR_STREAM_CLOSED);
    ADD_HR_PAIR(SPERR_NO_MORE_ITEMS);
    ADD_HR_PAIR(SPERR_NOT_FOUND);
    ADD_HR_PAIR(SPERR_INVALID_AUDIO_STATE);
    ADD_HR_PAIR(SPERR_GENERIC_MMSYS_ERROR);
    ADD_HR_PAIR(SPERR_MARSHALER_EXCEPTION);
    ADD_HR_PAIR(SPERR_NOT_DYNAMIC_GRAMMAR);
    ADD_HR_PAIR(SPERR_AMBIGUOUS_PROPERTY);
    ADD_HR_PAIR(SPERR_INVALID_REGISTRY_KEY);
    ADD_HR_PAIR(SPERR_INVALID_TOKEN_ID);
    ADD_HR_PAIR(SPERR_XML_BAD_SYNTAX);
    ADD_HR_PAIR(SPERR_XML_RESOURCE_NOT_FOUND);
    ADD_HR_PAIR(SPERR_TOKEN_IN_USE);
    ADD_HR_PAIR(SPERR_TOKEN_DELETED);
    ADD_HR_PAIR(SPERR_MULTI_LINGUAL_NOT_SUPPORTED);
    ADD_HR_PAIR(SPERR_EXPORT_DYNAMIC_RULE);
    ADD_HR_PAIR(SPERR_STGF_ERROR);
    ADD_HR_PAIR(SPERR_WORDFORMAT_ERROR);
    ADD_HR_PAIR(SPERR_STREAM_NOT_ACTIVE);
    ADD_HR_PAIR(SPERR_ENGINE_RESPONSE_INVALID);
    ADD_HR_PAIR(SPERR_SR_ENGINE_EXCEPTION);
    ADD_HR_PAIR(SPERR_STREAM_POS_INVALID);
    ADD_HR_PAIR(SP_RECOGNIZER_INACTIVE);
    ADD_HR_PAIR(SPERR_REMOTE_CALL_ON_WRONG_THREAD);
    ADD_HR_PAIR(SPERR_REMOTE_PROCESS_TERMINATED);
    ADD_HR_PAIR(SPERR_REMOTE_PROCESS_ALREADY_RUNNING);
    ADD_HR_PAIR(SPERR_LANGID_MISMATCH);
    ADD_HR_PAIR(SP_PARTIAL_PARSE_FOUND);
    ADD_HR_PAIR(SPERR_NOT_TOPLEVEL_RULE);
    ADD_HR_PAIR(SP_NO_RULE_ACTIVE);
    ADD_HR_PAIR(SPERR_LEX_REQUIRES_COOKIE);
    ADD_HR_PAIR(SP_STREAM_UNINITIALIZED);
    ADD_HR_PAIR(SPERR_UNSUPPORTED_LANG);
    ADD_HR_PAIR(SPERR_VOICE_PAUSED);
    ADD_HR_PAIR(SPERR_AUDIO_BUFFER_UNDERFLOW);
    ADD_HR_PAIR(SPERR_AUDIO_STOPPED_UNEXPECTEDLY);
    ADD_HR_PAIR(SPERR_NO_WORD_PRONUNCIATION);
    ADD_HR_PAIR(SPERR_ALTERNATES_WOULD_BE_INCONSISTENT);

    // These are standard error codes:
    ADD_HR_PAIR(E_NOTIMPL);
    ADD_HR_PAIR(E_UNEXPECTED);
    ADD_HR_PAIR(E_OUTOFMEMORY);
    ADD_HR_PAIR(E_INVALIDARG);
    ADD_HR_PAIR(E_NOINTERFACE);
    ADD_HR_PAIR(E_POINTER);
    ADD_HR_PAIR(E_HANDLE);
    ADD_HR_PAIR(E_ABORT);
    ADD_HR_PAIR(E_FAIL);
    ADD_HR_PAIR(E_ACCESSDENIED);
    ADD_HR_PAIR(REGDB_E_CLASSNOTREG);
    ADD_HR_PAIR(REGDB_E_IIDNOTREG);

    ADD_WIN32_PAIR(ERROR_BAD_EXE_FORMAT);
    ADD_WIN32_PAIR(ERROR_RESOURCE_DATA_NOT_FOUND);
    ADD_WIN32_PAIR(ERROR_RESOURCE_TYPE_NOT_FOUND);
    ADD_WIN32_PAIR(ERROR_RESOURCE_NAME_NOT_FOUND);
    ADD_WIN32_PAIR(ERROR_RESOURCE_LANG_NOT_FOUND);
#undef ADD_HR_PAIR
#undef ADD_WIN32_PAIR
}