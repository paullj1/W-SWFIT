W-SWFIT
===

The Windows Software Fault Injection Tool (x64).  This tool implements the
[G-SWFIT technique](http://dx.doi.org/10.1109/TSE.2006.113) by [Dr. Joao Duraes](http://ieeexplore.ieee.org/search/searchresult.jsp?searchWithin=%22Authors%22:.QT.J.%20A.%20Duraes.QT.&newsearch=true) and [Dr. Henrique Madeira]
(http://ieeexplore.ieee.org/search/searchresult.jsp?searchWithin=%22Authors%22:.QT.H.%20S.%20Madeira.QT.&newsearch=true) from the University of Coimbra, Portugal.

Currently, it's statically configured to target the NTDSA.dll library loaded by
the lsass.exe executable.  However, this tool is designed in such a way, that
with minor modifications it can be altered to perform the G-SWFIT technique
against any x64 application in a Windows environment.

How To Run
---
This tool is built using Microsoft Visual Studio Professional 2013 and must be
run as System user which can be done using the PsExec tool included in the
[Sysinternals Suite](https://technet.microsoft.com/en-us/sysinternals/bb842062.aspx)
by Mark Russinovich:

`PsExec -i -s cmd.exe`

Then in the new console window:

`FaultInjection.exe`

Contact
---
[Paul Jordan](mailto:paul.jordan@afit.edu)
