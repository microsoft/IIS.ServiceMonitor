// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

# Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# ServiceMonitor

ServiceMonitor service the purpose as the foreground process when runnin IIS in container. It monitors the w3svc service, when the service state is changes from SERVICE_RUNNING to SERVICE_STOPPED | SERVICE_STOP_PENDING | SERVICE_PAUSED | SERVICE_PAUSE_PENDING. ServiceMonitor will exit. As a result, the container will exit.

## Add Envirormental Variables to ApplicationHost.config

ServiceMonitor will also iterate through the envirormental variable and add them to ApplicationHost.config using appcmd one by one. ServiceMonitor also define of a a set of environment variables that should not be added to ApplicationHost.config. First, try to lookup for the name. If the name is in the list, try to look for it's value, only filter environment variables out when both name and value match or name matches with NULL value. 

### Environment Variables to be Filter out

[a,b]
[d,e]
[f,NULL]
...

## How to build

.\build.cmd


## How to Run.
.\ServiceMonitor.exe [ServiceName]
eg. .\ServiceMonitor.exe w3svc