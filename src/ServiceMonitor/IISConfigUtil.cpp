// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include"stdafx.h"
#include <iostream>
using namespace std;

#define APPCMD_MAX_SIZE 30000
#define KV(a,b) pair<wstring, LPTSTR>(a,b)
#define KV_WSTR(a,b) pair<wstring, wstring>(a,b)
#define POPULATE(map) do                          \
                    {                             \
                        map.insert(KV(L"TMP",                     L"C:\\Users\\ContainerAdministrator\\AppData\\Local\\Temp")); \
                        map.insert(KV(L"TEMP",                    L"C:\\Users\\ContainerAdministrator\\AppData\\Local\\Temp")); \
                        map.insert(KV(L"USERNAME",                L"ContainerAdministrator")); \
                        map.insert(KV(L"USERPROFILE",             L"C:\\Users\\ContainerAdministrator")); \
                        map.insert(KV(L"APPDATA",                 L"C:\\Users\\ContainerAdministrator\\AppData\\Roaming")); \
                        map.insert(KV(L"LOCALAPPDATA",            L"C:\\Users\\ContainerAdministrator\\AppData\\Local")); \
                        map.insert(KV(L"PROGRAMDATA",             L"C:\\ProgramData")); \
                        map.insert(KV(L"PSMODULEPATH",            L"%ProgramFiles%\\WindowsPowerShell\\Modules;C:\\Windows\\system32\\WindowsPowerShell\\v1.0\\Modules")); \
                        map.insert(KV(L"PUBLIC",                  L"C:\\Users\\Public")); \
                        map.insert(KV(L"USERDOMAIN",              L"User Manager")); \
                        map.insert(KV(L"ALLUSERSPROFILE",         L"C:\\ProgramData")); \
                        map.insert(KV(L"PATH",                    L"C:\\Windows\\system32;C:\\Windows;C:\\Windows\\System32\\Wbem;C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\;C:\\Users\\ContainerAdministrator\\AppData\\Local\\Microsoft\\WindowsApps")); \
                        map.insert(KV(L"PATHEXT",                 L".COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC")); \
                        map.insert(KV(L"COMPUTERNAME",            NULL)); \
                        map.insert(KV(L"COMSPEC",                 NULL)); \
                        map.insert(KV(L"OS",                      NULL)); \
                        map.insert(KV(L"PROCESSOR_IDENTIFIER",    NULL)); \
                        map.insert(KV(L"PROCESSOR_LEVEL",         NULL)); \
                        map.insert(KV(L"PROCESSOR_REVISION",      NULL)); \
                        map.insert(KV(L"PROGRAMFILES",            NULL)); \
                        map.insert(KV(L"PROGRAMFILES(X86)",       NULL)); \
                        map.insert(KV(L"PROGRAMW6432",            NULL)); \
                        map.insert(KV(L"SYSTEMDRIVE",             NULL)); \
                        map.insert(KV(L"WINDIR",                  NULL)); \
                        map.insert(KV(L"NUMBER_OF_PROCESSORS",    NULL)); \
                        map.insert(KV(L"PROCESSOR_ARCHITECTURE",  NULL)); \
                        map.insert(KV(L"SYSTEMROOT",              NULL)); \
                        map.insert(KV(L"COMMONPROGRAMFILES",      NULL)); \
                        map.insert(KV(L"COMMONPROGRAMFILES(X86)", NULL)); \
                        map.insert(KV(L"COMMONPROGRAMW6432",      NULL)); \
                    } while(0)

IISConfigUtil::IISConfigUtil():m_pstrSysDirPath(NULL)
{
}

IISConfigUtil::~IISConfigUtil()
{
    if (m_pstrSysDirPath != NULL)
    {
        delete m_pstrSysDirPath;
        m_pstrSysDirPath = NULL;
    }
}

BOOL IISConfigUtil::FilterEnv(const unordered_map<wstring, LPTSTR>& filter, LPCTSTR strEnvName, LPCTSTR strEnvValue)
{
    LPTSTR   strFilterValue;
    wstring strFilterName;
    _ASSERT(strEnvName != NULL);
    _ASSERT(strEnvValue != NULL);

    strFilterName = strEnvName;
    auto value = (filter).find(strFilterName);
    if (value == (filter).end())
    {
        return FALSE;
    }
    
    strFilterValue = value->second;

    //don't need to match value or value match
    if ((strFilterValue == NULL ) || (lstrcmpi(strEnvValue, strFilterValue) == 0))
    {
        return TRUE;
    }

    return FALSE;
}

HRESULT IISConfigUtil::Initialize()
{
    HRESULT hr        = S_OK;
    TCHAR*  pBuffer   = NULL;
    DWORD   dwBufSize = 0;
    //
    // resolve system drive
    //
    dwBufSize = GetSystemDirectory(NULL, 0);
    if (dwBufSize == 0)
    {
        // failed to get System Directory info
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Finished;
    }

    pBuffer = (TCHAR*)malloc(dwBufSize * sizeof(TCHAR));
    if (pBuffer == NULL) 
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto Finished;
    }

    if (GetSystemDirectory(pBuffer, dwBufSize) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Finished;
    }

    m_pstrSysDirPath = pBuffer;
    pBuffer = NULL;

Finished:
    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }
    return hr;
}

HRESULT IISConfigUtil::BuildAppCmdCommand(vector<pair<wstring, wstring>> vecSet, WCHAR* pstrAppPoolName, wstring& pStrCmd, APPCMD_CMD_TYPE appCmdType, int* beginIndex)
{
    HRESULT hr = S_OK;
    _ASSERT(pstrAppPoolName != NULL);

    //wstring* pstrCmd = new wstring();
    //if (pstrCmd == NULL)
    //{
    //    hr = ERROR_OUTOFMEMORY;
    //    goto Finished;
    //}
    
    pStrCmd.append(m_pstrSysDirPath);
    pStrCmd.append(L"\\inetsrv\\appcmd.exe set config -section:system.applicationHost/applicationPools ");

    for (int i = *beginIndex; i < vecSet.size(); i++)
    {
        wstring strEnvName = vecSet[i].first;
        wstring strEnvValue = vecSet[i].second;

        if ((pStrCmd.length() + strEnvName.length() + strEnvValue.length()) > APPCMD_MAX_SIZE)
        {
            //set the begin index for next iteration
            *beginIndex = i;
            hr = ERROR_MORE_DATA;
            break;
        }

        if (appCmdType == APPCMD_ADD)
        {
            pStrCmd.append(L"/+\"[name='");
        }
        else
        {
            pStrCmd.append(L"/-\"[name='");

        }
        pStrCmd.append(pstrAppPoolName);
        pStrCmd.append(L"'].environmentVariables.[name='");
        pStrCmd.append(strEnvName);
        if (appCmdType == APPCMD_ADD)
        {
            pStrCmd.append(L"',value='");
            pStrCmd.append(strEnvValue);
        }
        pStrCmd.append(L"']\" ");

        if (i + 1 == vecSet.size())
        {
            //set to indicate it's done
            *beginIndex = -1;
        }
    }

    pStrCmd.append(L" /commit:apphost");
    //*pStrCmd = pstrCmd;

Finished:
    return hr;
}


HRESULT IISConfigUtil::RunCommand(wstring pstrCmd, BOOL fIgnoreError)
{
    HRESULT     hr       = S_OK;
    STARTUPINFO si       = { sizeof(STARTUPINFO) };
    DWORD       dwStatus = 0;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    //si.dwFlags |= STARTF_USESTDHANDLES;
    wcout << pstrCmd << endl;
    if (!CreateProcess(NULL,
        (LPWSTR)pstrCmd.c_str(),
        NULL,
        NULL,
        false,
        0,
        NULL,
        NULL,
        &si,
        &pi))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Finished;
    }

    // wait for at most 5 seconds to allow APPCMD finish
    WaitForSingleObject(pi.hProcess, 5000);
    if ((!GetExitCodeProcess(pi.hProcess, &dwStatus) || dwStatus != 0) && (!fIgnoreError))
    {
        //
        // appcmd command failed
        //
        _tprintf(L"\nAPPCMD failed with error code %d\n", dwStatus);
        hr = E_FAIL;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

Finished:

    return hr;
}

HRESULT IISConfigUtil::UpdateEnvironmentVarsToConfig(WCHAR* pstrAppPoolName)
{
    HRESULT  hr           = S_OK;
    LPTCH    lpvEnv       = NULL;
    LPTSTR   lpszVariable = NULL;
    wstring  pstrAddCmd;
    wstring  pstrRmCmd;
    int      beginIndex    = 0;
    BOOL     fMoreData     = TRUE;

    unordered_map<wstring, LPTSTR> filter;
    vector<pair<wstring, wstring>> envVec;
    POPULATE(filter) ;

    lpvEnv = GetEnvironmentStrings();
    if (lpvEnv == NULL)
    {
        _tprintf(L"Failed to call GetEnvironmentStrings! \n");
        hr = E_FAIL;
        goto Finished;
    }

    lpszVariable = (LPTSTR)lpvEnv;
    while (*lpszVariable)
    {
        LPTSTR pEqualChar = wcschr(lpszVariable, L'=');
        if (pEqualChar != lpszVariable)
        {
            DWORD dwStatus = 0;
            LPTSTR pstrValue = pEqualChar + 1;
            LPTSTR pstrName = lpszVariable;


            pEqualChar[0] = L'\0';
            if (FilterEnv(filter, CharUpper(pstrName), pstrValue))
            {
                pEqualChar[0] = L'=';
                lpszVariable += lstrlen(lpszVariable) + 1;
                
                continue;
            }

            /*
            wstring pStrTempName;
            pStrTempName.append(pstrName);
            wstring pStrTempValue;;
            pStrTempValue.append(pstrValue);
            */

            envVec.emplace_back(wstring(pstrName), wstring(pstrValue));

            pEqualChar[0] = L'=';

        }
        //
        // move to next environment variable
        //
        lpszVariable += lstrlen(lpszVariable) + 1;
    }
    
    while (fMoreData)
    {

        pstrRmCmd.clear();
        fMoreData = FALSE;
        hr = BuildAppCmdCommand(envVec, pstrAppPoolName, pstrRmCmd, APPCMD_RM, &beginIndex);

        _tprintf(L" RM COMMAND \n");

        if (hr != ERROR_MORE_DATA && FAILED(hr))
        {
                goto Finished;
        }

        if (hr == ERROR_MORE_DATA)
        {
            hr = S_OK;
            fMoreData = TRUE;
        }
        //allow appcmd to fail if it is trying to remove environment variable
        RunCommand(pstrRmCmd, TRUE);
    }

    fMoreData = TRUE;
    beginIndex = 0;
    while (fMoreData)
    {
        _tprintf(L" ADD COMMAND \n");
        pstrAddCmd.clear();
        fMoreData = FALSE;
        hr = BuildAppCmdCommand(envVec, pstrAppPoolName, pstrAddCmd, APPCMD_ADD, &beginIndex);

        if (hr != ERROR_MORE_DATA && FAILED(hr))
        {
            goto Finished;
        }

        if (hr == ERROR_MORE_DATA)
        {
            hr = S_OK;
            fMoreData = TRUE;
        }

        //appcmd must succeed when add new environment variables
        hr = RunCommand(pstrAddCmd, FALSE);

        if (FAILED(hr))
        {
            goto Finished;
        }
    }

Finished:
    if (lpvEnv != NULL)
    {
        FreeEnvironmentStrings(lpvEnv);
        lpvEnv = NULL;
    }

    return hr;
}