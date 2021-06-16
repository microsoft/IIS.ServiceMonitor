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
                        map.insert(KV(L"PATHEXT",                 L".COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC")); \
                        map.insert(KV(L"PATH",                    NULL)); \
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
                        map.insert(KV(L"DRIVERDATA",              NULL)); \
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
    _ASSERT(strEnvName != NULL);
    _ASSERT(strEnvValue != NULL);

    auto value = filter.find(strEnvName);

    //
    // add this environment variable if the name does not match the block list
    //
    if (value == filter.end())
    {
        return FALSE;
    }
    
    strFilterValue = value->second;

    //
    //  filter out this environment variable if
    //  1. value match is not required (strFilterValue is NULL)
    //  2. require value match and value matches
    //
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
        //
        // failed to get System Directory info
        //
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

void IISConfigUtil::Replace(std::wstring& str, std::wstring oldValue, std::wstring newValue)
{
    size_t pos = 0;
    size_t oldValueLen = oldValue.length();
    size_t newValueLen = newValue.length();

    while ((pos = str.find(oldValue, pos)) != std::wstring::npos) {
        str.replace(pos, oldValueLen, newValue);
        pos += newValueLen;
    }
}

HRESULT IISConfigUtil::BuildAppCmdCommand(const vector<pair<wstring, wstring>>& vecSet, vector<pair<wstring, wstring>>::iterator& envVecIter, WCHAR* pstrAppPoolName, wstring& pStrCmd, APPCMD_CMD_TYPE appCmdType)
{
    HRESULT hr = S_OK;
    _ASSERT(pstrAppPoolName != NULL);

    pStrCmd.append(m_pstrSysDirPath);
    pStrCmd.append(L"\\inetsrv\\appcmd.exe set config -section:system.applicationHost/applicationPools ");

    for (; envVecIter != vecSet.end(); envVecIter++)
    {
        wstring strEnvName  = envVecIter->first;
        wstring strEnvValue = envVecIter->second;

        //
        // Handle values that have single and double quotes
        //
        Replace(strEnvName, L"'", L"''");
        Replace(strEnvName, L"\"", L"\"\"\"");
        Replace(strEnvValue, L"'", L"''");
        Replace(strEnvValue, L"\"", L"\"\"\"");

        if ((pStrCmd.length() + strEnvName.length() + strEnvValue.length()) > APPCMD_MAX_SIZE)
        {
            //
            // caller need to call again
            //

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
    }

    pStrCmd.append(L" /commit:apphost");

    return hr;
}


HRESULT IISConfigUtil::RunCommand(wstring& pstrCmd, BOOL fIgnoreError)
{
    HRESULT     hr       = S_OK;
    STARTUPINFO si       = { sizeof(STARTUPINFO) };
    DWORD       dwStatus = 0;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags |= STARTF_USESTDHANDLES;

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

    //
    // wait for at most 30 seconds to allow APPCMD finish
    //
    WaitForSingleObject(pi.hProcess, 30000);
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
    BOOL     fMoreData     = TRUE;

    unordered_map<wstring, LPTSTR> filter;
    vector<pair<wstring, wstring>> envVec;
    vector<pair<wstring, wstring>>::iterator envVecIter;

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
            wstring strNameCheck(pstrName);
            if (FilterEnv(filter, CharUpper((LPWSTR)strNameCheck.c_str()), pstrValue))
            {
                pEqualChar[0] = L'=';
                lpszVariable += lstrlen(lpszVariable) + 1;
                
                continue;
            }

            envVec.emplace_back(wstring(pstrName), wstring(pstrValue));

            pEqualChar[0] = L'=';

        }
        //
        // move to next environment variable
        //
        lpszVariable += lstrlen(lpszVariable) + 1;
    }
    
    envVecIter = envVec.begin();
    while (fMoreData)
    {

        pstrRmCmd.clear();
        fMoreData = FALSE;
        hr = BuildAppCmdCommand(envVec, envVecIter, pstrAppPoolName, pstrRmCmd, APPCMD_RM);

        if (hr != ERROR_MORE_DATA && FAILED(hr))
        {
                goto Finished;
        }

        if (hr == ERROR_MORE_DATA)
        {
            hr = S_OK;
            fMoreData = TRUE;
        }

        //
        //allow appcmd to fail if it is trying to remove environment variable
        //
        RunCommand(pstrRmCmd, TRUE);
    }

    fMoreData = TRUE;
    envVecIter = envVec.begin();
    while (fMoreData)
    {
        pstrAddCmd.clear();
        fMoreData = FALSE;
        hr = BuildAppCmdCommand(envVec, envVecIter, pstrAppPoolName, pstrAddCmd, APPCMD_ADD);

        if (hr != ERROR_MORE_DATA && FAILED(hr))
        {
            goto Finished;
        }

        if (hr == ERROR_MORE_DATA)
        {
            hr = S_OK;
            fMoreData = TRUE;
        }

        //
        //appcmd must succeed when add new environment variables
        //
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
