W-SWFIT
===

**This repository has been made private pending Public Affairs release**

The Windows Software Fault Injection Tool (x64).  This tool implements the
[G-SWFIT technique](http://dx.doi.org/10.1109/TSE.2006.113) by [Dr. Joao Duraes](http://ieeexplore.ieee.org/search/searchresult.jsp?searchWithin=%22Authors%22:.QT.J.%20A.%20Duraes.QT.&newsearch=true) and [Dr. Henrique Madeira]
(http://ieeexplore.ieee.org/search/searchresult.jsp?searchWithin=%22Authors%22:.QT.H.%20S.%20Madeira.QT.&newsearch=true) from the University of Coimbra, Portugal.

W-SWFIT is designed to perform the G-SWFIT technique against any x64 application in 
a Windows environment.  It has not been tested against 32-bit applications, but in 
theory should work.

How To Run
---
This tool is built using Microsoft Visual Studio Professional 2013.  If the target
application is a kernel mode application (such as lsass.exe), then W-SWFIT must be
run as System user which can be done using the PsExec tool included in the
[Sysinternals Suite](https://technet.microsoft.com/en-us/sysinternals/bb842062.aspx)
by Mark Russinovich as follows:

`PsExec -i -s cmd.exe`

Then in the new console window:

`FaultInjection.exe`

Contact
---
[Paul Jordan](mailto:paullj1@gmail.com)

Disclaimer
---
The views expressed herein are solely those of the authors and do not reflect
the official policy or position of the U.S. Air Force, the Department of
Defense, or the U.S. Government.
