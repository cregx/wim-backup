# WIM-Backup (this document is not yet ready)
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
<img alt="wim-backup UI" src="https://user-images.githubusercontent.com/14788832/208285320-7b9b63a8-2d20-41b7-98e7-139b15d1b47d.png" width="50%" height="50%" />
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

### How to use wim-backup (step-by-step instructions)

> The instructions described below are necessary because the solution requires the use of a WinPE-based boot medium. However, this must not be made available for licensing reasons. 

> In short: I am not allowed to provide a bootable ISO file with the solution. You will have to create it yourself.

1. [Download](https://github.com/cregx/wim-backup/releases) a current release version of **wim-backup** (or compile your own customised version).
2. You will need the Deployment and Imaging Tools from Microsoft. These can be found in the [Windows ADK](https://learn.microsoft.com/en-us/windows-hardware/get-started/adk-install) (Windows Assessment and Deployment Kit). Be sure to download the appropriate ADK version for your development environment (your Windows version). Make sure that you have selected the **Deployment Tools** in the features.
<p align="center" width="100%">
<img alt="adk-installation-screenshot" src="https://user-images.githubusercontent.com/14788832/208288059-db11b50d-09d8-4fab-9339-f8a2c17ce7ab.PNG" width="60%" height="60%" />
</p>

3. In addition, you need the **Windows Assessment and Deployment Kit Windows Preinstallation Environment Add-ons** (see download link from ADK). These are installed following the ADK setup.
<p align="center" width="100%">
<img alt="winpe-addons-installation" src="https://user-images.githubusercontent.com/14788832/208288374-c8305c4e-2863-48eb-a88c-6775a41d7c9d.PNG" width="60%" height="60%" />
</p>

4. Assuming that the processing of the previous steps was successful, the next step is to create a WinPE-enabled boot medium. This contains the WIM-Backup solution. To do this, start the **Deployment and Imaging Tools Environment** with **administrative** privileges.

5. Copy the **amd64** directory to a folder that does not yet exist, for example **C:\temp\media**.

```
copype amd64 c:\temp\media
```

<p align="center" width="100%">
<img alt="copype-to-media" src="https://user-images.githubusercontent.com/14788832/208670949-73eac611-35a0-4772-8572-006f87844d15.png" width="60%" height="60%" />

6. Then mount the **boot.wim** image from the previously copied **amd64** directory to the newly created **c:\temp\media\mount** directory.

```
dism /mount-image /imagefile:C\temp\media\media\source\boot.wim /mountdir:C:\temp\media\mount /index:1
```

7. Create the folder **Tools** in the directory **C:\Temp\media\mount**.

8. Now copy the following release files into this folder:
* **wimbckup.exe**
* **action.bat**
* **diskpart.txt**

9. You also need to copy the **winpkeshl.ini** file to **C:\Temp\media\mount\Windows\System32**.

10. Finally, the mounted image must be **un-mounted** and the **WinPEMedia ISO file** must be created.

```
dism /unmount-image /mountdir:C:\temp\media\mount /commit
...
cd ..
cd "Windows Preinstallation Environment"
MakeWinPEMedia.cmd /iso C:\temp\media C:\temp\wim-backup-100.iso
```

11. After having created the **WinPE-ISO** file, we can now use [Rufus](https://rufus.ie/) to transfer it to a **USB flash drive** to boot from and create a WIM-based backup or restore an existing one.

<p align="center" width="100%">
<img alt="rufus-create-bootable-drive" src="https://user-images.githubusercontent.com/14788832/208671977-82813a19-1fb7-4526-9e0b-263ab906aa79.PNG" width="30%" height="30%" />

## WIM-Backup in action

The animation below shows WIM-Backup in action.



## FAQ

❓ How much time should I expect to spend trying out the solution in my environment?

It is very difficult to give a general answer to this question. I estimate the time required to work through the instruction steps at around 30 to 60 minutes.

❓ Why is no bootable, i.e. ready-to-use ISO image incl. the WIM backup solution offered?

The issue is with the license terms. According to this, I am not allowed to provide a pre-built image based on WinPE.
That is the reason why every developer has to create his own ISO image. The internal use of this image (within the own team) should then not be a problem.

## Code of Conduct

Please refer to the [Code of Conduct](https://github.com/cregx/wim-backup/blob/main/CODE_OF_CONDUCT.md) for this repository.

## Disclaimer

This program code is provided "as is", without warranty or guarantee as to its usability or effects on systems. It may be used, distributed and modified in any manner, provided that the parties agree and acknowledge that the author(s) assume(s) no responsibility or liability for the results obtained by the use of this code.

[![ForTheBadge built-with-love](http://ForTheBadge.com/images/badges/built-with-love.svg)](https://www.cregx.de)
