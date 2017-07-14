#pragma once

#include <Windows.h>

class Service_Monitor
{

public:
    Service_Monitor() :_hSCManager(NULL)
    {
        InitializeSRWLock(&_srwLock);
    }

    ~Service_Monitor()
    {
        if (_hSCManager != NULL)
        {
            CloseServiceHandle(_hSCManager);
        }
    }

    HRESULT EnsureInitialized();
    HRESULT StartServiceByName(LPCTSTR pServiceName, DWORD dwTimeOutSeconds = 20);
    HRESULT StopServiceByName(LPCTSTR pServiceName, DWORD dwTimeOutSeconds = 20);
    HRESULT MonitoringService(LPCTSTR pServiceName, DWORD dwStatus, HANDLE hStopEvent);
    static VOID  CALLBACK  NotifyCallBack(PVOID parameter);

private:
    HRESULT GetServiceHandle(LPCTSTR pServiceName, SC_HANDLE* pHandle);
    SC_HANDLE _hSCManager;
    BOOL      _fInitialized;
    SRWLOCK   _srwLock;
};
