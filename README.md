# WIM-Backup
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Visual Studio](https://badgen.net/badge/icon/visualstudio?icon=visualstudio&label)](https://visualstudio.microsoft.com)
[![GitHub issues](https://img.shields.io/github/issues/cregx/wim-backup)](https://github.com/cregx/wim-backup/issues)
[![GitHub closed issues](https://img.shields.io/github/issues-closed/cregx/wim-backup)](https://github.com/cregx/wim-backup/issues?q=is%3Aissue+is%3Aclosed)
[![Code-Signed](https://img.shields.io/badge/code--signed%20exe-Yes-green)](https://github.com/cregx/wim-backup/releases)
[![Wiki available](https://img.shields.io/badge/wiki-Yes-green)](https://github.com/cregx/wim-backup/wiki)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/cregx/wim-backup)](https://github.com/cregx/wim-backup/releases)
[![Commits since release](https://img.shields.io/github/commits-since/cregx/wim-backup/latest/main)](https://github.com/cregx/wim-backup/commits/main)
[![GitHub stars](https://img.shields.io/github/stars/cregx/wim-backup)](https://github.com/cregx/wim-backup/stargazers)
[![Github All Releases](https://img.shields.io/github/downloads/cregx/wim-backup/total.svg)](https://github.com/cregx/wim-backup/releases)

The UI application "WIM-Backup" offers the possibility to create offline backups of Windows partitions (e.g. hard disk C:) in a WinPE environment. The Windows Imaging Format (WIM) is used for this purpose.

<p align="center" width="100%">
<img alt="WIM-Backup-Social-Media-Logo" src="https://user-images.githubusercontent.com/14788832/213690768-80f35b94-76b6-4a64-b9b9-cc9ee3c9516d.png" width="100%" height="100%" />
</p>

WIM-Backup is a Win32 application created in (Microsoft) C and Visual Studio 2010. Of course, you can also compile the project using a newer version of Visual Studio, such as 2019, or you can use an already compiled release version.

## The advantage of WIM-Backup

WIM-Backup offers a free backup option for a Windows-based system. So you don't always need a commercial software.

## The story behind the solution

In the process of troubleshooting and fixing bugs on Windows 10-based systems, I repeatedly had to perform new installations of the operating system. This process proved to be very time-consuming in some cases. So I came up with the idea of backing up the Windows image so that it could be restored without much effort if necessary. This was the birth of WIM-Backup.

<p align="center" width="100%">
<img alt="wim-backup UI" src="https://user-images.githubusercontent.com/14788832/212529178-de7ed094-d73a-482f-9cc9-3912ddc8ed8d.PNG" width="50%" height="50%" />
</p>

#### Components of the solution
The solution consists of the following four components:

- **wimbckup.exe**: GUI-based application of the WIM-Backup solution
- **action.bat**: batch script responsible for executing the backup (creating the WIM file) and restoring the system from a WIM file
- **diskpart.txt**: parameter file with instructions for diskpart, which is needed for the restore process
- **winpeshl.ini**: template file for the execution of WIM-Backup (wimbckup.exe) within the WinPE environment

#### Rough flowchart

The following sketch roughly illustrates the essential relationships.

<p align="center" width="100%">
<img alt="wim-backup rough flowchar" src="https://user-images.githubusercontent.com/14788832/208229659-e2b77a70-f128-4320-a91e-1e2d8ac69626.png" width="70%" height="70%" />
</p>

### Brief summary

- WIM-Backup always requires an external bootable media such as a USB flash drive.
- From this drive WinPE is booted to perform a backup or restore to or from an external medium (e.g. a USB hard drive).
- On the bootable USB flash drive the WinPE must be set up before (is documented illustrated in the readme).
- After completion of the respective operation, a status message is displayed whether the operation was successful or failed.
- After restoring a backup, you can boot normally from the destination drive.
- Both the backup and restore process are relatively simple (not "rocket science").
- To set up the solution, you need about 30 minutes time in the best case due to the necessary downloads (e .g. ADK)
- Last but not least: it has a permissive license (non-proprietary) and is open source.

## How to use WIM-Backup (step-by-step instructions)

> The instructions described below are necessary because the solution requires the use of a WinPE-based boot medium. However, this must not be made available for licensing reasons. 

> In short: I am not allowed to provide a bootable ISO file with the solution. You will have to create it yourself.

1. [Download](https://github.com/cregx/wim-backup/releases) a current release version of **WIM-Backup** (or compile your own customised version).
2. You will need the Deployment and Imaging Tools from Microsoft. These can be found in the [Windows ADK](https://learn.microsoft.com/en-us/windows-hardware/get-started/adk-install) (Windows Assessment and Deployment Kit). Be sure to download the appropriate ADK version for your development environment (your Windows version). Make sure that you have selected the **Deployment Tools** in the features.
<p align="center" width="100%">
<img alt="adk-installation-screenshot" src="https://user-images.githubusercontent.com/14788832/208288059-db11b50d-09d8-4fab-9339-f8a2c17ce7ab.PNG" width="60%" height="60%" />
</p>

3. In addition, you need the **Windows Assessment and Deployment Kit Windows Preinstallation Environment Add-ons** (see download link from ADK). These are installed following the ADK setup.
<p align="center" width="100%">
<img alt="winpe-addons-installation" src="https://user-images.githubusercontent.com/14788832/208288374-c8305c4e-2863-48eb-a88c-6775a41d7c9d.PNG" width="60%" height="60%" />
</p>

4. Assuming that the processing of the previous steps was successful, the next step is to create a WinPE-enabled boot medium. This contains the WIM-Backup solution. To do this, start the **Deployment and Imaging Tools Environment** with **administrative** privileges.

5. Copy the **amd64** directory to a folder that **does not yet exist**, for example **C:\Temp\media**.

> Tip: Copy the desired commands to the clipboard (to the right of the respective command), this way you can speed up the creation process considerably.

```
copype amd64 c:\Temp\media
```

<p align="center" width="100%">
<img alt="copype-to-media" src="https://user-images.githubusercontent.com/14788832/208670949-73eac611-35a0-4772-8572-006f87844d15.png" width="60%" height="60%" />

6. Then mount the **boot.wim** image from the previously copied **amd64** directory to the newly created **c:\temp\media\mount** directory.

```
dism /mount-image /imagefile:C:\Temp\media\media\sources\boot.wim /mountdir:C:\Temp\media\mount /index:1
```

7. Create the folder **Tools** in the directory **C:\Temp\media\mount**.

```
mkdir C:\Temp\media\mount\Tools
```

8. Now copy the following release files into this folder:
* **wimbckup.exe**
* **action.bat**
* **diskpart.txt**

9. You also need to copy the **winpkeshl.ini** file to **C:\Temp\media\mount\Windows\System32**.

10. Finally, the mounted image must be **un-mounted** and the **WinPEMedia ISO file** must be created.
  
> Do not forget to close all windows/files/applications that are open on the mounted directory. Otherwise, DISM will complain that it cannot complete the unmounting process.  

```
dism /unmount-image /mountdir:C:\Temp\media\mount /commit
```

```
cd ..
cd "Windows Preinstallation Environment"
MakeWinPEMedia.cmd /iso C:\Temp\media C:\temp\wim-backup-102.iso
```

11. After having created the **WinPE-ISO** file, we can now use **Rufus** [on Github](https://github.com/pbatard/rufus) / [Official Website](https://rufus.ie/) to transfer it to a **USB flash drive** to boot from and create a WIM-based backup or restore an existing one.

<p align="center" width="100%">
<img alt="rufus-create-bootable-drive" src="https://user-images.githubusercontent.com/14788832/208671977-82813a19-1fb7-4526-9e0b-263ab906aa79.PNG" width="50%" height="50%" />

## WIM-Backup in action

The following animation shows WIM-Backup in action when creating a backup.

> More information on how to use WIM-Backup can be found in the [Wiki](https://github.com/cregx/wim-backup/wiki).

<p align="center" width="100%">
<img alt="wim-backup-animation-optimized" src="https://user-images.githubusercontent.com/14788832/209192953-c536cee4-1518-422d-b4d3-bfca5b6ee420.gif" width="75%" height="75%" />
</p>

## FAQ

### :question: How much time should I expect to spend trying out the solution in my environment?

It is very difficult to give a general answer to this question. I estimate the time required to work through the instruction steps at around 30 to 60 minutes.

### :question: Why is no bootable, i.e. ready-to-use ISO image incl. the WIM backup solution offered?

The issue is with the license terms. According to this, I am not allowed to provide a pre-built image based on WinPE.
That is the reason why every developer has to create his own ISO image. The internal use of this image (within the own team) should then not be a problem.

### :question: How are errors that occurred during backup or restore reported?

WIM-Backup uses the dism.log file to identify possible errors. To do this, this file is checked for the occurrence of the pattern "Error" and a general error message is issued after a backup/restore operation is completed.

You can find the dism.log file under the following file path: *%systemdrive%\Tools\dism.log*. Here, the *%systemdrive%* environment variable in WinPE normally points to the x:\ drive.

### :question: Are there any known limitations to using WIM-Backup?

Yes, the list below shows the currently known restrictions:

- If you want to backup a BitLocker-encrypted drive, you must first stop active BitLocker protection (BDE) in Windows. However, this does not mean that you have to decrypt the encrypted drive first. Once you exit BDE protection and restart the PC to boot from WinPE into the solution, WIM-Backup can access the drive. However, if you forget this step, WIM-Backup will not recognize the BDE-encrypted drive.
- During the restore process, you can restore an existing WIM backup image on the C: drive only. Restoring to other drives is not supported.
- WIM-Backup can only create **offline backups**. Attempting to backup a system while it is running, that is, the operating system has booted properly, results in an error.
- An overview of the operating systems tested with WIM-Backup can be found in the [Wiki](https://github.com/cregx/wim-backup/wiki/Requirements#tested-windows-versions).

## Stargazers, Forkers & other users

Thanks to all for using WIM-Backup.

### Stargazers

[![Stargazers repo roster for @cregx/wim-backup](https://reporoster.com/stars/cregx/wim-backup)](https://github.com/cregx/wim-backup/stargazers)

### Forkers
[![Forkers repo roster for @cregx/wim-backup](https://reporoster.com/forks/cregx/wim-backup)](https://github.com/cregx/wim-backup/network/members)

## Code of Conduct

Please refer to the [Code of Conduct](https://github.com/cregx/wim-backup/blob/main/CODE_OF_CONDUCT.md) for this repository.

## Disclaimer

This program code is provided "as is", without warranty or guarantee as to its usability or effects on systems. It may be used, distributed and modified in any manner, provided that the parties agree and acknowledge that the author(s) assume(s) no responsibility or liability for the results obtained by the use of this code.

[![ForTheBadge built-with-love](http://ForTheBadge.com/images/badges/built-with-love.svg)](https://www.cregx.de)
