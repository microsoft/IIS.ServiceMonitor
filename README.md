# Microsoft IIS Service Monitor

**ServiceMonitor** is a Windows executable designed to be used as the entrypoint
process when running IIS inside a Windows Server container.

ServiceMonitor monitors the status of the `w3svc` service and will exit when the
service state changes from `SERVICE_RUNNING` to either one of `SERVICE_STOPPED`,
`SERVICE_STOP_PENDING`, `SERVICE_PAUSED` or `SERVICE_PAUSE_PENDING`.

Additionally, ServiceMonitor will promote environment variables from process
environment it's own process environment block to the DefaultAppPool. We achieve
this by naively copying all variables in our process environment block except
for those Environment variable / value pairs present in this list below.

### Environment variable exclusion list

| Environment Variable    | Value                                                                                                                                                                      |
|-------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| TMP                     | "C:\\Users\\ContainerAdministrator\\AppData\\Local\\Temp"                                                                                                                  |
| TEMP                    | "C:\\Users\\ContainerAdministrator\\AppData\\Local\\Temp"                                                                                                                  |
| USERNAME                | "ContainerAdministrator"                                                                                                                                                   |
| USERPROFILE             | "C:\\Users\\ContainerAdministrator"                                                                                                                                        |
| APPDATA                 | "C:\\Users\\ContainerAdministrator\\AppData\\Roaming"                                                                                                                      |
| LOCALAPPDATA            | "C:\\Users\\ContainerAdministrator\\AppData\\Local"                                                                                                                        |
| PROGRAMDATA             | "C:\\ProgramData"                                                                                                                                                          |
| PSMODULEPATH            | "%ProgramFiles%\\WindowsPowerShell\\Modules;C:\\Windows\\system32\\WindowsPowerShell\\v1.0\\Modules"                                                                       |
| PUBLIC                  | "C:\\Users\\Public"                                                                                                                                                        |
| USERDOMAIN              | "User Manager"                                                                                                                                                             |
| ALLUSERSPROFILE         | "C:\\ProgramData"                                                                                                                                                          |
| PATH                    | "C:\\Windows\\system32;C:\\Windows;C:\\Windows\\System32\\Wbem;C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\;C:\\Users\\ContainerAdministrator\\Microsoft\\WindowsApps" |
| PATHEXT                 | ".COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC"                                                                                                                    |
| COMPUTERNAME            | *                                                                                                                                                                          |
| COMSPEC                 | *                                                                                                                                                                          |
| OS                      | *                                                                                                                                                                          |
| PROCESSOR_IDENTIFIER    | *                                                                                                                                                                          |
| PROCESSOR_LEVEL         | *                                                                                                                                                                          |
| PROCESSOR_REVISION      | *                                                                                                                                                                          |
| PROGRAMFILES            | *                                                                                                                                                                          |
| PROGRAMFILES(X86)       | *                                                                                                                                                                          |
| PROGRAMW6432            | *                                                                                                                                                                          |
| SYSTEMDRIVE             | *                                                                                                                                                                          |
| WINDIR                  | *                                                                                                                                                                          |
| NUMBER_OF_PROCESSORS    | *                                                                                                                                                                          |
| PROCESSOR_ARCHITECTURE  | *                                                                                                                                                                          |
| SYSTEMROOT              | *                                                                                                                                                                          |
| COMMONPROGRAMFILES      | *                                                                                                                                                                          |
| COMMONPROGRAMFILES(X86) | *                                                                                                                                                                          |
| COMMONPROGRAMW6432      | *                                                                                                                                                                          |

## Build

```
.\build.cmd
```

## Usage

```
.\ServiceMonitor.exe w3svc
```

ServiceMonitor is currently distributed as part of the[IIS](https://github.com/microsoft/iis-docker),
[ASP.NET](https://github.com/microsoft/aspnet-docker), and [WCF](https://github.com/microsoft/wcf-docker) images on DockerHub. We recommend layering your project on top of those official images as running
ServiceMonitor directly in your Dockerfile. 

## Contributing

This project welcomes contributions and suggestions.  Most contributions require
you to agree to a Contributor License Agreement (CLA) declaring that you have
the right to, and actually do, grant us the rights to use your contribution. For
details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether
you need to provide a CLA and decorate the PR appropriately (e.g., label,
comment). Simply follow the instructions provided by the bot. You will only need
to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any
additional questions or comments.


