// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include"stdafx.h"
#include"EtwListner.h"

#define IIS_LOG_TRACE L"SV-IISEventTraceSession"
#define WCHAR_BYTES sizeof(WCHAR)
#define CHAR_BYTES sizeof(CHAR)
#define USHORT_BYTES sizeof(USHORT)
#define ULONG_BYTESE sizeof(ULONG)
#define ULONGLONG_BYTES sizeof(ULONGLONG)
#define WCHAR_NULL_BYTES sizeof(WCHAR)
#define CHAR_NULL_BYTES sizeof(CHAR)
#define NULL_BYTES 1

// IIS ETW GUID Provider
// 0x7E8AD27F - 0xB271 - 0x4EA2 - 0xA783 - A47BDE29143B
static const GUID IISProviderGuid =
{ 0x7E8AD27F, 0xB271, 0x4EA2,{ 0xA7, 0x83, 0xA4, 0x7B, 0xDE, 0x29, 0x14, 0x3B } };

VOID WINAPI
IISLoggingEvtRecCallback(
    __in PEVENT_RECORD pEvtRecord
)
{
    std::wcout << "AAAAAAAAAAAAAAAAAAAA" << std::endl;
    USHORT      UserDataLength = pEvtRecord->UserDataLength;
    PVOID       UserData = pEvtRecord->UserData;
    PVOID       userDataEnd = (char *)UserData + UserDataLength;

    //
    // Read Raw Data
    //

    WCHAR *     date = ((WCHAR *)UserData + 2);
    WCHAR *     time = ((WCHAR *)date + wcslen(date) + NULL_BYTES);
    WCHAR *     clientIP = ((WCHAR *)time + wcslen(time) + NULL_BYTES);
    WCHAR *     userName = ((WCHAR *)clientIP + wcslen(clientIP) + NULL_BYTES);
    WCHAR *     siteName = ((WCHAR *)userName + wcslen(userName) + NULL_BYTES);
    WCHAR *     computerName = ((WCHAR *)siteName + wcslen(siteName) + NULL_BYTES);
    WCHAR *     serverIP = ((WCHAR *)computerName + wcslen(computerName) + NULL_BYTES);
    CHAR *      method = ((CHAR *)serverIP + WCHAR_BYTES * wcslen(serverIP) + WCHAR_NULL_BYTES);
    WCHAR *     uriStem = (WCHAR *)(method + strlen(method) + CHAR_NULL_BYTES);
    CHAR *      uriQuery = ((CHAR *)uriStem + WCHAR_BYTES * wcslen(uriStem) + WCHAR_NULL_BYTES);
    USHORT *    protocolStatus = (USHORT *)(uriQuery + strlen(uriQuery) + CHAR_NULL_BYTES);
    ULONG *     win32Status = (ULONG *)((CHAR *)protocolStatus + USHORT_BYTES);
    ULONGLONG * bytesSent = (ULONGLONG *)((CHAR *)win32Status + ULONG_BYTESE);
    ULONGLONG * bytesReceived = (ULONGLONG *)((CHAR *)bytesSent + ULONGLONG_BYTES);
    ULONGLONG * timeTaken = (ULONGLONG *)((CHAR *)bytesReceived + ULONGLONG_BYTES);
    USHORT *    port = (USHORT *)((CHAR *)timeTaken + ULONGLONG_BYTES);
    CHAR *      userAgent = ((CHAR *)port + USHORT_BYTES);
    CHAR *      cookie = ((CHAR *)userAgent + strlen(userAgent) + NULL_BYTES);
    CHAR *      referer = ((CHAR *)cookie + strlen(cookie) + NULL_BYTES);
    WCHAR *     protocolVer = (WCHAR *)(referer + strlen(referer) + CHAR_NULL_BYTES);
    CHAR *      host = (CHAR *)protocolVer + WCHAR_BYTES * wcslen(protocolVer) + WCHAR_NULL_BYTES;
    USHORT *    protocolSubStatus = (USHORT *)(host + strlen(host) + CHAR_NULL_BYTES);
    WCHAR *     customFields = (WCHAR *)((CHAR *)protocolSubStatus + USHORT_BYTES);

    //
    // Print the data out, try to match the order in event viewer
    //

    _tprintf(L""
        "date %s "
        "time %s "
        "s-sitename %s "
        "s-computername %s "
        "s-ip %s "
        , date, time, siteName, computerName, serverIP
    );

    printf(""
        "cs-method %s "
        , method
    );

    _tprintf(L""
        "cs-uri-stem %s "
        , uriStem
    );

    printf(""
        "cs-uri-query %s "
        "port %d "
        , uriQuery, *port
    );

    _tprintf(L""
        "cs-user-name %s "
        "c-ip %s "
        "cs-version %s "
        , userName, clientIP, protocolVer
    );

    printf(""
        "cs(User-Agent) %s "
        "cs(Cookie) %s "
        "cs(Referer) %s "
        "cs-host %s "
        "sc-status %d "
        "sc-substatus %d "
        "sc-win32-status %d "
        "sc-bytes %I64u "
        "cs-bytess %I64u "
        "time-taken %I64u "
        , userAgent, cookie, referer, host, *protocolStatus, *protocolSubStatus, *win32Status, *bytesSent, *bytesReceived, *timeTaken
    );

    _tprintf(L""
        "%s"
        , customFields
    );

    _tprintf(L"\n\n");
    //fflush(stdout);
    return;
}

DWORD WINAPI
ProcessEtwThreadProc(
    LPVOID             pValue
)
{
    HRESULT hr = S_OK;
    ULONG   uStatus = ERROR_SUCCESS;

    uStatus = ProcessTrace((TRACEHANDLE *)pValue,
        1,
        NULL,
        NULL
    );

    if (uStatus != ERROR_SUCCESS)
    {
        _tprintf(L"ProcessTrace FAIL %i\n", uStatus);
        return NULL;
    }

    return NULL;
}


EtwListner::~EtwListner()
{
}

EtwListner::EtwListner()
{
}

void EtwListner::StartListenIISEvent()
{
    ULONG hr = StartListen(IIS_LOG_TRACE, &IISProviderGuid);
    if (hr != ERROR_SUCCESS)
    {
        _tprintf(L"FAILED TO LISTEN TO IIS ETW LOGS %i\n", hr);
    }
}

ULONG EtwListner::StartListen(LPWSTR pStrSessionName, LPCGUID pTraceGUID)
{
    ULONG     uStatus = ERROR_SUCCESS;
    DWORD	  dwThreadId;
    HANDLE    pThreadHandle;
    EVENT_TRACE_PROPERTIES closeSessionPropt;
    EVENT_TRACE_PROPERTIES iisLogSessionPropt;

    //TRACEHANDLE handle;

    TRACEHANDLE hTrace;

    //
    // stop the trace if the trace already started
    //

    closeSessionPropt.Wnode.BufferSize = 1000;
    StopTrace(NULL, pStrSessionName, &closeSessionPropt);

    if (uStatus != ERROR_SUCCESS)
    {
        _tprintf(L"UNABLE TO STOP OLD TRACE %i\n", uStatus);
    }

    ZeroMemory(&iisLogSessionPropt, sizeof(EVENT_TRACE_PROPERTIES));

    iisLogSessionPropt.Wnode.BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + wcslen(pStrSessionName) * WCHAR_BYTES + WCHAR_NULL_BYTES;

    iisLogSessionPropt.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    iisLogSessionPropt.Wnode.ClientContext = 1;
    iisLogSessionPropt.LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING;
    iisLogSessionPropt.FlushTimer = 1;
    iisLogSessionPropt.MaximumFileSize = 1024;
    iisLogSessionPropt.LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    iisLogSessionPropt.LogFileNameOffset = 0;
    iisLogSessionPropt.BufferSize = 1024;
    iisLogSessionPropt.MinimumBuffers = 20;
    iisLogSessionPropt.MaximumBuffers = 30;

    EVENT_TRACE_LOGFILE eventTraceLog = { 0 };

    uStatus = StartTrace((PTRACEHANDLE)&hTrace, pStrSessionName, &iisLogSessionPropt);
    if (uStatus != ERROR_SUCCESS)
    {
        _tprintf(L"START TRACE FAIL %i\n", uStatus);
        goto finish;
    }

    uStatus = EnableTraceEx2(hTrace,
        (LPCGUID)pTraceGUID,
        EVENT_CONTROL_CODE_ENABLE_PROVIDER,
        TRACE_LEVEL_INFORMATION,
        0,
        0,
        0,
        NULL
    );

    if (uStatus != ERROR_SUCCESS)
    {
        _tprintf(L"ENABLE TRACE FAIL \n");
        goto finish;
    }

    ZeroMemory(&eventTraceLog, sizeof(EVENT_TRACE_LOGFILE));

    eventTraceLog.LogFileName = NULL;

    eventTraceLog.LoggerName = pStrSessionName;
    eventTraceLog.CurrentTime = 0;
    eventTraceLog.BuffersRead = 0;

    eventTraceLog.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_REAL_TIME;

    eventTraceLog.BufferCallback = NULL;
    eventTraceLog.BufferSize = 0;
    eventTraceLog.Filled = 0;
    eventTraceLog.EventsLost = 0;

    eventTraceLog.EventRecordCallback = IISLoggingEvtRecCallback;
    eventTraceLog.Context = NULL;

    //
    // reuse the hTrace handle variable
    //

    hTrace = OpenTrace(&eventTraceLog);
    if (hTrace == INVALID_PROCESSTRACE_HANDLE)
    {
        uStatus = GetLastError();
        _tprintf(L"OPEN TRACE FAIL %i\n", uStatus);
        goto finish;
    }

    pThreadHandle = CreateThread(NULL,
        0x2000000,
        ProcessEtwThreadProc,
        (LPVOID)&hTrace,
        0,
        &dwThreadId);

    if (pThreadHandle == NULL)
    {
        uStatus = GetLastError();
        _tprintf(L"START THREAD FAIL %i\n", uStatus);
        goto finish;
    }

    _tprintf(L"Start Listen to IIS Etw Logs...\n");
    //fflush(stdout);

finish:

    return uStatus;
}