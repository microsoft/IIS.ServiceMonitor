// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include"stdafx.h"

HANDLE g_hStopEvent = INVALID_HANDLE_VALUE;

VOID CtrlHandle(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        _tprintf(L"\nTerminating the process due to CTRL signal was recevied.\n");
        SetEvent(g_hStopEvent);
        g_hStopEvent = INVALID_HANDLE_VALUE;
        break;
    default:
        return;
    }
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr = S_OK;
    Service_Monitor sm = Service_Monitor();

    if (argc <= 1)
    {
        _tprintf(L"\nUSAGE: %s [windows service name]\n", argv[0]);
        goto Finished;
    }

    // iis only allow monitor on w3svc
    if (_wcsicmp(argv[1], L"w3svc") == 0)
    {
        hr = sm.StopServiceByName(L"w3svc");
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
        if( FAILED(hr = configHelper.Initialize()) ||
            FAILED(hr = configHelper.UpdateEnvironmentVarsToConfig(L"DefaultAppPool")))
        {
            _tprintf(L"\nFailed to update IIS configuration\n");
            goto Finished;
        }
    }

    g_hStopEvent = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        argv[1]             // object name
    );

    if (g_hStopEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Finished;
    }

    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandle, TRUE))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        _tprintf(L"\nERROR: Failed to set control handle with error [%x]\n", hr);
        goto Finished;
    }

    if (g_hStopEvent != INVALID_HANDLE_VALUE)
    {
        hr = sm.StartServiceByName(argv[1]);
		EtwListner sl = EtwListner();
    }

    if (SUCCEEDED(hr))
    {
        if (g_hStopEvent != INVALID_HANDLE_VALUE)
        {
            //
            // will stop monitoring once the stop event is set
            //
            hr = sm.MonitoringService(argv[1],
                SERVICE_NOTIFY_STOPPED | SERVICE_NOTIFY_STOP_PENDING | SERVICE_NOTIFY_PAUSED | SERVICE_NOTIFY_PAUSE_PENDING,
                g_hStopEvent);
        }
    }
    //
    // don't capture the stop error
    //
    sm.StopServiceByName(argv[1]);

Finished:
    if (g_hStopEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hStopEvent);
        g_hStopEvent = INVALID_HANDLE_VALUE;
    }
    return hr;
}