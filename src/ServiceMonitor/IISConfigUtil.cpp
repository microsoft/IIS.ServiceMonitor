// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include"stdafx.h"
#include <iostream>
using namespace std;

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

BOOL IISConfigUtil::FilterEnv(unordered_map<wstring, LPTSTR> filter, LPTSTR strEnvName, LPTSTR strEnvValue)
{
    LPTSTR   strFilterValue;
    wstring* strFilterName;
    _ASSERT(strEnvName != NULL);
    _ASSERT(strEnvValue != NULL);

    CharUpper(strEnvName);
    strFilterName = new wstring(strEnvName);
    auto value = filter.find(*strFilterName);
    if (value == filter.end())
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

HRESULT IISConfigUtil::BuildAppCmdCommand(unordered_map<wstring, wstring> envSet, WCHAR* pstrAppPoolName, wstring** pStrCmd, BOOL fAddCommand)
{
    HRESULT hr = S_OK;

    _ASSERT(strEnvName != NULL);
    _ASSERT(strEnvValue != NULL);
    _ASSERT(pstrAppPoolName != NULL);

    wstring* pstr = new wstring();
    if (pstr == NULL)
    {
        hr = ERROR_OUTOFMEMORY;
        goto Finished;
    }
	pstr->append(m_pstrSysDirPath);
	pstr->append(L"\\inetsrv\\appcmd.exe set config -section:system.applicationHost/applicationPools ");

	for (auto it = envSet.begin(); it != envSet.end(); ++it)
	{
		wstring strEnvName = it->first;
		wstring strEnvValue = it->second;
		if (fAddCommand)
		{
			pstr->append(L"/+\"[name='");
		}
		else
		{
			pstr->append(L"/-\"[name='");

		}
		pstr->append(pstrAppPoolName);
		pstr->append(L"'].environmentVariables.[name='");
		pstr->append(strEnvName);
		if (fAddCommand)
		{
			pstr->append(L"',value='");
			pstr->append(strEnvValue);
		}
		pstr->append(L"']\" ");
	}
	pstr->append(L" /commit:apphost");
	*pStrCmd = pstr;

	///////////////////////DEBUG//////////////////////////////
	wcout << *pstr << endl;
	wcout << endl;
	wcout << endl;
	///////////////////////DEBUG//////////////////////////////

Finished:
    return hr;
}


HRESULT IISConfigUtil::RunCommand(wstring * pstrCmd, BOOL fIgnoreError)
{
    HRESULT     hr       = S_OK;
    STARTUPINFO si       = { sizeof(STARTUPINFO) };
    DWORD       dwStatus = 0;
    PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL,
        (LPWSTR)pstrCmd->c_str(),
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

    if (pstrCmd != NULL)
    {
        delete pstrCmd;
        pstrCmd = NULL;
    }

    return hr;
}

HRESULT IISConfigUtil::UpdateEnvironmentVarsToConfig(WCHAR* pstrAppPoolName)
{
    HRESULT  hr           = S_OK;
    LPTCH    lpvEnv       = NULL;
    LPTSTR   lpszVariable = NULL;
    wstring* pstrAddCmd     = NULL;
    wstring* pstrRmCmd      = NULL;

	unordered_map<wstring, LPTSTR> filter;
	unordered_map<wstring, wstring> envSet;
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
            if (FilterEnv(filter, pstrName, pstrValue))
            {
                pEqualChar[0] = L'=';
                lpszVariable += lstrlen(lpszVariable) + 1;
                
                continue;
            }
			wstring * pStrTempName = new wstring();
			pStrTempName->append(pstrName);
			wstring * pStrTempValue = new wstring();
			pStrTempValue->append(pstrValue);

			envSet.insert(KV_WSTR(*pStrTempName, *pStrTempValue));
            
            pEqualChar[0] = L'=';

		}
        //
        // move to next environment variable
        //
        lpszVariable += lstrlen(lpszVariable) + 1;
    }

	hr = BuildAppCmdCommand(envSet, pstrAppPoolName, &pstrAddCmd, TRUE);
	if (FAILED(hr))
	{
		goto Finished;
	}

	hr = BuildAppCmdCommand(envSet, pstrAppPoolName, &pstrRmCmd, FALSE);
	if (FAILED(hr))
	{
		goto Finished;
	}

	//allow appcmd to fail if it is trying to remove environment variable
	RunCommand(pstrRmCmd, TRUE);
	//appcmd must success when add new environment variable
	hr = RunCommand(pstrAddCmd, FALSE);

	if (FAILED(hr))
	{
		goto Finished;
	}

Finished:
    if (lpvEnv != NULL)
    {
        FreeEnvironmentStrings(lpvEnv);
        lpvEnv = NULL;
    }

    return hr;
}