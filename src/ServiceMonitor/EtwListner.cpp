#include"stdafx.h"
#include"EtwListner.h"



#define SERVICE_MONITOR_TRACE  L"SV-IISEventTraceSession2"
#define WCHAR_BYTES sizeof(WCHAR);
#define CHAR_BYTES sizeof(CHAR);
#define USHORT_BYTES sizeof(USHORT);
#define ULONG_BYTESE sizeof(ULONG);
#define ULONGLONG_BYTES sizeof(ULONGLONG);



VOID WINAPI
EvtRecCallback(
	__in PEVENT_RECORD pEvtRecord
)
{
	USHORT      UserDataLength = pEvtRecord->UserDataLength;
	PVOID       UserData = pEvtRecord->UserData;
	PVOID       userDataEnd = (char *)UserData + UserDataLength;

	WCHAR *     date = ((WCHAR *)UserData + 2);
	WCHAR *     time = ((WCHAR *)date + wcslen(date) + 1);
	WCHAR *     clientIP = ((WCHAR *)time + wcslen(time) + 1);
	WCHAR *     userName = ((WCHAR *)clientIP + wcslen(clientIP) + 1);
	WCHAR *     siteName = ((WCHAR *)userName + wcslen(userName) + 1);
	WCHAR *     computerName = ((WCHAR *)siteName + wcslen(siteName) + 1);
	WCHAR *     serverIP = ((WCHAR *)computerName + wcslen(computerName) + 1);
	CHAR *      method = ((CHAR *)serverIP + 2 * wcslen(serverIP) + 2);
	WCHAR *     uriStem = (WCHAR *)(method + strlen(method) + 1);
	CHAR *      uriQuery = ((CHAR *)uriStem + 2 * wcslen(uriStem) + 2);
	USHORT *    protocolStatus = (USHORT *)(uriQuery + strlen(uriQuery) + 1);
	ULONG *     win32Status = (ULONG *)((CHAR *)protocolStatus + sizeof(USHORT));
	ULONGLONG * bytesSent = (ULONGLONG *)((CHAR *)win32Status + sizeof(ULONG));
	ULONGLONG * bytesReceived = (ULONGLONG *)((CHAR *)bytesSent + sizeof(ULONGLONG));
	ULONGLONG * timeTaken = (ULONGLONG *)((CHAR *)bytesReceived + sizeof(ULONGLONG));
	USHORT *    port = (USHORT *)((CHAR *)timeTaken + sizeof(ULONGLONG));
	CHAR *      userAgent = ((CHAR *)port + sizeof(USHORT));
	CHAR *      cookie = ((CHAR *)userAgent + strlen(userAgent) + 1);
	CHAR *      referer = ((CHAR *)cookie + strlen(cookie) + 1);
	WCHAR *     protocolVer = (WCHAR *)(referer + strlen(referer) + 1);
	CHAR *      host = (CHAR *)protocolVer + 2 * wcslen(protocolVer) + 2;
	USHORT *    protocolSubStatus = (USHORT *)(host + strlen(host) + 1);
	WCHAR *     customFields = (WCHAR *)((CHAR *)protocolSubStatus + sizeof(USHORT));

	/*
	if ((customFields + wcslen(customFields)) <= userDataEnd)
	{
		_tprintf(L"\INVALID DATA RECEIVED n\n");
		return;
	}
	*/

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
	return;

}

DWORD WINAPI
ProcessIISEtwThreadProc(
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
		std::wcout << "PROCESS ETW TRACE FAILED" << std::endl;
		return NULL;
	}

	return NULL;
}

//0x7E8AD27F - 0xB271 - 0x4EA2 - 0xA783 - A47BDE29143B
static const GUID ProviderGuid =
{ 0x7E8AD27F, 0xB271, 0x4EA2,{ 0xA7, 0x83, 0xA4, 0x7B, 0xDE, 0x29, 0x14, 0x3B } };

#define HTTP_LOG_EVENT_HANDLER_SESSION_BUFFER_SIZE sizeof(EVENT_TRACE_PROPERTIES) + sizeof(SERVICE_MONITOR_TRACE)
EtwListner::~EtwListner()
{
}

EtwListner::EtwListner()
	{
	ULONG   uStatus = ERROR_SUCCESS;

	EVENT_TRACE_PROPERTIES stsessionProperties;
	stsessionProperties.Wnode.BufferSize = 1000;
	StopTrace(NULL, SERVICE_MONITOR_TRACE, &stsessionProperties);

	if (uStatus != ERROR_SUCCESS)
	{
		_tprintf(L"UNABLE TO STOP OLD TRACE %i\n", uStatus);
	}

	EVENT_TRACE_PROPERTIES stp;
	ZeroMemory(&stp, sizeof(EVENT_TRACE_PROPERTIES));

	stp.Wnode.BufferSize = HTTP_LOG_EVENT_HANDLER_SESSION_BUFFER_SIZE;
	stp.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	stp.Wnode.ClientContext = 1;
	stp.LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING;
	stp.FlushTimer = 1;
	stp.MaximumFileSize = 1024;
	stp.LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
	stp.LogFileNameOffset = 0;
	stp.BufferSize = 1024;
	stp.MinimumBuffers = 20;
	stp.MaximumBuffers = 30;

	TRACEHANDLE handle;

	TRACEHANDLE hTrace;
	EVENT_TRACE_LOGFILE eventTraceLog = { 0 };

	uStatus = StartTrace((PTRACEHANDLE)&handle, SERVICE_MONITOR_TRACE, &stp);
	if (uStatus != ERROR_SUCCESS)
	{
		_tprintf(L"START TRACE FAIL %i\n", uStatus);
		goto finish;
	}

	uStatus = EnableTraceEx2(handle,
		(LPCGUID)&ProviderGuid,
		EVENT_CONTROL_CODE_ENABLE_PROVIDER,
		TRACE_LEVEL_INFORMATION,
		0,
		0,
		0,
		NULL
	);

	if (uStatus != ERROR_SUCCESS)
	{
		_tprintf(L"ENABLE TRACe FAIL \n");
		goto finish;

	}

	ZeroMemory(&eventTraceLog, sizeof(EVENT_TRACE_LOGFILE));

	eventTraceLog.LogFileName = NULL;

	eventTraceLog.LoggerName = SERVICE_MONITOR_TRACE;
	eventTraceLog.CurrentTime = 0;
	eventTraceLog.BuffersRead = 0;

	eventTraceLog.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_REAL_TIME;

	eventTraceLog.BufferCallback = NULL;
	eventTraceLog.BufferSize = 0;
	eventTraceLog.Filled = 0;
	eventTraceLog.EventsLost = 0;

	eventTraceLog.EventRecordCallback = EvtRecCallback;
	eventTraceLog.Context = NULL;

	hTrace = OpenTrace(&eventTraceLog);
	if (hTrace == INVALID_PROCESSTRACE_HANDLE)
	{
		uStatus = GetLastError();
		std::wcout << "OPEN TRACE FAIL" << std::endl;
		goto finish;
	}


	DWORD dwThreadId;
	HANDLE  pHandle;
	pHandle = CreateThread(NULL,
		0x2000000,
		ProcessIISEtwThreadProc,
		(LPVOID)&hTrace,
		0,
		&dwThreadId);
	if (pHandle == NULL)
	{
		uStatus = GetLastError();
		std::wcout << "START THREAD FAIL" << std::endl;
		goto finish;
	}
	std::wcout << "BYE" << std::endl;


finish:
	if (uStatus != ERROR_SUCCESS)
		return;

	}




