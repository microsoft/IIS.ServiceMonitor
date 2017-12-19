// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <iostream>
#include <evntrace.h>
#include <Evntcons.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <TCHAR.h>
#include <windows.h>


class EtwListner
{

public:
    EtwListner();
    ~EtwListner();
	void StartListenIISEvent();
    void StartListen(LPWSTR pStrSessionName, LPCGUID pTraceGUID);
};