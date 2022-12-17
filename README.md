# WIM-Backup (coming soon - is in the preparation process)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Visual Studio](https://badgen.net/badge/icon/visualstudio?icon=visualstudio&label)](https://visualstudio.microsoft.com)
[![GitHub issues](https://img.shields.io/github/issues/cregx/wim-backup)](https://github.com/cregx/wim-backup/issues)
[![GitHub closed issues](https://img.shields.io/github/issues-closed/cregx/wim-backup)](https://github.com/cregx/wim-backup/issues?q=is%3Aissue+is%3Aclosed)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/cregx/wim-backup)](https://github.com/cregx/wim-backup/releases)
[![GitHub stars](https://img.shields.io/github/stars/cregx/wim-backup)](https://github.com/cregx/wim-backup/stargazers)
[![Github All Releases](https://img.shields.io/github/downloads/cregx/wim-backup/total.svg)]()
[![Code-Signed](https://img.shields.io/badge/code--signed%20exe-Yes-green)](https://github.com/cregx/wim-backup/releases)


The UI application "WIM-Backup" offers the possibility to create backups of hard disks (partitions) in a WinPE environment. The Windows Imaging Format (WIM) is used for this purpose.

WIM-Backup is a Win32 application created in (Microsoft) C and Visual Studio 2010. Of course, you can also compile the project using a newer version of Visual Studio, such as 2019, or you can use an already compiled release version.

## The story behind WIM-Backup

In the process of troubleshooting and fixing bugs on Windows 10-based systems, I repeatedly had to perform new installations of the operating system. This process proved to be very time-consuming in some cases. So I came up with the idea of backing up the Windows image so that it could be restored without much effort if necessary. This was the birth of WIM-Backup.

<p align="center" width="100%">
<img alt="wim-backup UI" src="https://user-images.githubusercontent.com/14788832/208148759-8b61ad62-2a00-4ebf-aeef-5ffca6030afd.png" width="50%" height="50%" />
</p>

#### Components of the solution
The solution consists of the following four components:

- **wimbckup.exe**: GUI-based application of the wim-backup solution
- **action.bat**: batch script responsible for executing the backup (creating the WIM file) and restoring the system from a WIM file
- **diskpart.txt**: parameter file with instructions for diskpart, which is needed for the restore process
- **winpeshl.ini**: template file for the execution of wim-backup (wimbckup.exe) within the WinPE environment

#### Rough flowchart

The following sketch roughly illustrates the essential relationships.

<p align="center" width="100%">
<img alt="wim-backup rough flowchar" src="https://user-images.githubusercontent.com/14788832/208229659-e2b77a70-f128-4320-a91e-1e2d8ac69626.png" width="70%" height="70%" />
</p>

## Code of Conduct

Please refer to the [Code of Conduct](https://github.com/cregx/wim-backup/blob/main/CODE_OF_CONDUCT.md) for this repository.

## Disclaimer

This program code is provided "as is", without warranty or guarantee as to its usability or effects on systems. It may be used, distributed and modified in any manner, provided that the parties agree and acknowledge that the author(s) assume(s) no responsibility or liability for the results obtained by the use of this code.

[![ForTheBadge built-with-love](http://ForTheBadge.com/images/badges/built-with-love.svg)](https://www.cregx.de)
