// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <Windows.h>
#include <unordered_map>


class IISConfigUtil
{
public:

    IISConfigUtil();
    ~IISConfigUtil();
    HRESULT Initialize();
    HRESULT UpdateEnvironmentVarsToConfig(WCHAR* pstrAppPoolName);

private:
    HRESULT RunCommand(std::wstring * pstrCmd, BOOL fIgnoreError);
    HRESULT BuildAppCmdCommand(std::vector<std::pair<std::wstring, std::wstring>> vecSet, WCHAR* pstrAppPoolName, std::wstring** pStrCmd, BOOL fAddCommand, int* beginIndex);
    BOOL    FilterEnv(std::unordered_map<std::wstring, LPTSTR> filter, LPTSTR strEnvName, LPTSTR strEnvValue);
    TCHAR*  m_pstrSysDirPath;
};