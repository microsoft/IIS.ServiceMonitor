// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "stdafx.h"
#include "Version.h"

HANDLE g_hStopEvent = INVALID_HANDLE_VALUE;

BOOL WINAPI CtrlHandle(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        _tprintf(L"\nCTRL signal received. The process will now terminate.\n");
        SetEvent(g_hStopEvent);
        g_hStopEvent = INVALID_HANDLE_VALUE;
        break;

    default:
        break;
    }

    return TRUE;
}

bool isNumber(LPCWSTR strpointer) {

    int index;

    for (index = 0; index < wcslen(strpointer); index++) {
        if (iswdigit(strpointer[index]) == false) {
            return false;
        }
    }

    return true;
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr = S_OK;
    Service_Monitor sm = Service_Monitor();
    int argIndex;
    int serviceTimeout = 20; //default value - in seconds
    int appcmdTimeout = 5; //default value - in seconds
    bool invalidSyntax = false;
    int nextPositionalArgument = 1;

    WCHAR buffDrive[3], buffDirName[1024], buffFileName[1024], buffExt[25];
    _wsplitpath_s(argv[0], buffDrive, 3, buffDirName, 1024, buffFileName, 1024, buffExt, 25);

    WCHAR fileName[1024];
    wcscpy_s(fileName, buffFileName);
    wcscat_s(fileName, 1024, buffExt);

    WCHAR* pstrServiceName = L"w3svc";           //default service name
    WCHAR* pstrAppPoolName = L"DefaultAppPool"; // default application pool name

    for (argIndex = 1; argIndex < argc; argIndex++) {
        if ((argIndex == 1) && (
            (_wcsicmp(argv[argIndex], L"-?") == 0) ||
            (_wcsicmp(argv[argIndex], L"/?") == 0) ||
            (_wcsicmp(argv[argIndex], L"--help") == 0)
            )) {
            invalidSyntax = true;
            break;
        }
        else if (_wcsicmp(argv[argIndex], L"-st") == 0) {
            if (argIndex + 1 >= argc) {
                invalidSyntax = true;
            }
            else if (isNumber(argv[argIndex + 1])) {
                serviceTimeout = _wtoi(argv[argIndex + 1]);
                argIndex++;
            }
            else {
                invalidSyntax = true;
            }
        }
        else if (_wcsicmp(argv[argIndex], L"-at") == 0) {
            if (argIndex + 1 >= argc) {
                invalidSyntax = true;
            }
            else if (isNumber(argv[argIndex + 1])) {
                appcmdTimeout = _wtoi(argv[argIndex + 1]);
                argIndex++;
            }
            else {
                invalidSyntax = true;
            }
        }
        else if (nextPositionalArgument == 1) {
            pstrServiceName = argv[argIndex];
            nextPositionalArgument++;
        }
        else if (nextPositionalArgument == 2) {
            pstrAppPoolName = argv[argIndex];
            nextPositionalArgument++;
        }
        else {
            invalidSyntax = true;
        }
    }

    if (invalidSyntax)
    {
        _tprintf(L"\nServiceMonitor Version %d.%d.%d.%d", SM_MAJORNUMBER, SM_MINORNUMBER, SM_BUILDNUMBER, SM_BUILDMINORVERSION);
        _tprintf(L"\n\nUSAGE:");
        _tprintf(L"\n       %s [<windows_service_name> [application_pool]] [-st <service_timeout>] [-at <appcmd_timeout>]", fileName);
        _tprintf(L"\n\nOptions:");
        _tprintf(L"\n    windows_service_name    Name of the Windows service to monitor");
        _tprintf(L"\n    application_pool        Name of the application pool to monitor; defaults to DefaultAppPool");
        _tprintf(L"\n    service_timeout         The time (in seconds) it is willing to wait for the service to start or stop");
        _tprintf(L"\n    appcmd_timeout          The time (in seconds) it is willing to wait for the appcmd command to execute");
        _tprintf(L"\n\nExamples:");
        _tprintf(L"\n       %s", fileName);
        _tprintf(L"\n       %s w3svc", fileName);
        _tprintf(L"\n       %s -st 60", fileName);
        _tprintf(L"\n       %s w3svc ApplicationPool", fileName);
        _tprintf(L"\n       %s -at 30", fileName);
        _tprintf(L"\n       %s w3svc MyAppPool -st 60 -at 30", fileName);

        goto Finished;
    }

    _tprintf(L"Executing %s: %s [timeout: %d], ApplicationPool: %s [timeout: %d]", fileName, pstrServiceName, serviceTimeout, pstrAppPoolName, appcmdTimeout);

    // iis only allow monitor on w3svc
    if (_wcsicmp(pstrServiceName, L"w3svc") == 0)
    {
        hr = sm.StopServiceByName(L"w3svc", serviceTimeout);
        if (FAILED(hr))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Finished;
        }

        //
        // iis scenario, update the environment variable
        // we hardcode this behavior for now. We can add an input switch later if needed
        //
        IISConfigUtil configHelper = IISConfigUtil();
        if (FAILED(hr = configHelper.Initialize()) ||
            FAILED(hr = configHelper.UpdateEnvironmentVarsToConfig(pstrAppPoolName, appcmdTimeout)))
        {
            _tprintf(L"\nFailed to update IIS configuration\n");
            goto Finished;
        }
    }

    g_hStopEvent = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        pstrServiceName     // object name
    );

    if (g_hStopEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Finished;
    }

    if (!SetConsoleCtrlHandler(CtrlHandle, TRUE))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        _tprintf(L"\nERROR: Failed to set control handle with error [%x]\n", hr);
        goto Finished;
    }

    if (g_hStopEvent != INVALID_HANDLE_VALUE)
    {
        hr = sm.StartServiceByName(pstrServiceName, serviceTimeout);
    }

    if (SUCCEEDED(hr))
    {
        if (g_hStopEvent != INVALID_HANDLE_VALUE)
        {
            //
            // will stop monitoring once the stop event is set
            //
            hr = sm.MonitoringService(pstrServiceName,
                SERVICE_NOTIFY_STOPPED | SERVICE_NOTIFY_STOP_PENDING | SERVICE_NOTIFY_PAUSED | SERVICE_NOTIFY_PAUSE_PENDING,
                g_hStopEvent);
        }
    }
    //
    // don't capture the stop error
    //
    sm.StopServiceByName(pstrServiceName, serviceTimeout);

Finished:
    if (g_hStopEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hStopEvent);
        g_hStopEvent = INVALID_HANDLE_VALUE;
    }
    return hr;
}
