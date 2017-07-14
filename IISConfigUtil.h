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
    HRESULT RunCommand(std::wstring * pstrCmd);
    HRESULT BuildAppCmdCommand(WCHAR*  strEnvName, WCHAR* strEnvValue, WCHAR* pstrAppPoolName, std::wstring** pStrCmd, BOOL fAddCommand);
    BOOL    FilterEnv(std::unordered_map<std::wstring, LPTSTR> filter, LPTSTR strEnvName, LPTSTR strEnvValue);
    TCHAR*  m_pstrSysDirPath;
};