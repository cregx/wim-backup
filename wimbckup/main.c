/**
 * GUI application to perform backup and restore operations of a WIM file under WinPE.
 * Build with: Visual Studio 2010
 * 
 * The application was developed by Christoph Regner.
 * On the web: https://www.cregx.de
 * 
 * Copyright 2023 Christoph Regner
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For further information, please refer to the attached LICENSE.md
 */

/**
 * This application is based on a skeleton code from
 * https://www.codeproject.com/Articles/227831/A-Dialog-Based-Win32-C-Program-Step-by-Step
 * by (c) Rodrigo Cesar de Freitas Dias https://creativecommons.org/licenses/publicdomain/
 **/

/**
 * Important compilation note: This application must be compiled with the /MT option.
 * This will embed all the required parts of the C runtime library in the output file (Release / x64).
 * See for this (in VS 2010) under: Property pages / Configuration Properties / C/C++ / Code Generation / Runtime Library / Multi-Threaded (/MT).
 **/

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x400
#define _Win32_DCOM

#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <CommDlg.h>				// Dialogs
#include <Shlwapi.h>				// e.g. For example for shortening long paths. Needed: Shlwapi.lib static linked.
#include <strsafe.h>				// e.g. Safe string copy
#include <ShellAPI.h>				// e.g. SHELLEXECUTEINFO
#include <io.h>					// e.g. _access_s (file exists)
#include <Shobjidl.h>				// COM objects
#include <Objbase.h>

#include "resource.h"

// Enabling visual styles.
#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

// static link ComCtl32.lib.
#pragma comment(lib, "ComCtl32.lib")

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "propsys.lib")

typedef enum { BACKUP, RECOVERY } action_type;		// 0 for backup, 1 for restore
typedef enum BDE_Status { UNPROTECTED, PROTECTED, UNKNOWN } BDE_Status;

INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
void onAction(HWND, action_type);
void onClose(HWND);
void onInit(HWND, WPARAM);
void onClick_RadioRecovery(HWND);
void onClick_RadioBackup(HWND);
void onClick_SelectRecoveryFile(HWND, PTSTR, PTSTR, PTSTR, action_type);
void onClick_SelectBackupFile(HWND, PTSTR, PTSTR, PTSTR, action_type);
void onClick_ButtonLog(HWND);
void onClick_ButtonCmd(void);
void SelectFile(HWND, PTSTR, PTSTR, PTSTR, action_type);
void ShortenFilePath(HWND, PTSTR);
void InitializeFileDlg(HWND);
BOOL OpenFileDlg(HWND, PTSTR, PTSTR, action_type);
const TCHAR * GetOwnPath(void);
const TCHAR * GetBatchPath(TCHAR *);
void Action(const TCHAR *, PTSTR);
DWORD ActionEx(const TCHAR *, TCHAR *, ...);
BOOL FileExists(const wchar_t *);
void onChange_ComboBoxDrives(HWND, WORD);
void InitComboBox_LogicalDrives(HWND, HWND, HWND);
void GetVolumeInfo(LPCTSTR, LPTSTR, DWORD);
void GetDiskSpaces(LPCTSTR, LPTSTR, DWORD);

// Globals
static OPENFILENAME ofn;
static TCHAR g_szRecFileName[MAX_PATH];			// Full path name of the recovery file.
static TCHAR g_szRecTitleName[MAX_PATH];		// Name of the recovery file.
static TCHAR g_szRecFileNameShortened[MAX_PATH];	// Full path shortened name of the recovery file (e. g. C:\Data\...\recovery.wim)
static TCHAR g_szBckFileName[MAX_PATH];			// Full path name of the backup file.(e. g. E:\Backups\...\2021_02_backup.wim)
static TCHAR g_szBckTitleName[MAX_PATH];		// Name of the backup file.
static TCHAR g_szBckFileNameShortened[MAX_PATH];	// Full path shortened name of the backup file.
static TCHAR g_szBckVolumeName[16];			// Name of the volume whose data is to be backed up, e.g. C:\.
const TCHAR * g_pstrExePath;				// Full path to the exe.
const TCHAR * g_pstrBatchPath;				// Full path to the batch file.
BOOL g_bIsRecoveryBtnSelected;				// Which radio button ist selected?
action_type g_currentAction;				// Restore or backup action should be performed?

// Don't forget to increase the version number in the resource file (wimbckup.rc).
const LPCWSTR szAppVersion	= TEXT("App version 1.0.3 / 4th February 2023\nCopyright (c) 2023 Christoph Regner (https://github.com/cregx)\nWIM-Backup is licensed under the Apache License 2.0");

// Text constants
const LPCWSTR szRecoveryBtnText	= TEXT("Restore");
const LPCWSTR szRunRecoveryText = TEXT("Restore process");
const LPCWSTR szQuestRunRecText	= TEXT("Do you want to run the restore operation?");
const LPCWSTR szBackupBtnText	= TEXT("Backup");
const LPCWSTR szRunBackupText	= TEXT("Backup process");
const LPCWSTR szQuestRunBckText = TEXT("Do you want to run the backup operation?");
const LPCWSTR szBatchFileName	= TEXT("action.bat");
const LPCWSTR szDismLogFilePath = TEXT("%systemdrive%\\Tools\\dism.log");
const LPCWSTR szActionBckFin	= TEXT("The operation was completed successfully.\n\nThe backup was made to the following file: %s.\nCheck any errors that may have occurred in the log file.");
const LPCWSTR szActionRecFin	= TEXT("The process has been completed. Check any errors that may have occurred in the log file.");
const LPCWSTR szActionBckFailed = TEXT("The operation has failed. Check possible errors in the log file.");
const LPCWSTR szActionFailed	= TEXT("The operation has failed.\n\nError code: %lu\nCheck the log file for the cause of the error.");
const LPCWSTR szActionBackup	= TEXT("Backup");
const LPCWSTR szActionRecovery	= TEXT("Restore");
const LPCWSTR szFileNotFound	= TEXT("The file %s could not be found.");
const LPCWSTR szInfo			= TEXT("Information");

const DWORD RUN_ACTION_SHELLEX_FAILED	= 0xFFFFFFFFFFFFFFFF;		// dec => -1 (Function internal error, use GetLastError())
const DWORD RUN_ACTION_SUCCESSFUL	= 0x400;			// dec => 1024 (Successful processing of the batch file.)
const DWORD RUN_ACTION_CANCELLED	= 0xC000013A;			// dec => 3221225786 (Cancellation of the batch job by the user,
																	//		  e.g. because the user has clicked the X button.)

// Main function.
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE h0, LPTSTR lpCmdLine, int nCmdShow)
{
  HWND hDlg;
  MSG msg;
  BOOL ret;

  /**
   * For the correct execution of GetSaveFileName() (COM) we need CoInitializeEx()
   * when starting the application and CoUninitialize() when exiting.
   * See also under: https://learn.microsoft.com/en-us/troubleshoot/windows/win32/shell-functions-multithreaded-apartment
   **/
  HRESULT hrCoInitializeEx = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

  InitCommonControls();
  hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG), 0, DialogProc, 0);
  ShowWindow(hDlg, nCmdShow);

  while((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
    if(ret == -1)
      return -1;

    if(!IsDialogMessage(hDlg, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  
  CoUninitialize();
  return 0;
}

/**
 * Dialog box message procedure.
 */
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH hBrushStatic;
	static COLORREF colorRef[] = { RGB(255, 0, 0), RGB(0, 0, 139) };		// red, darkblue

	switch(uMsg)
	{
		case WM_INITDIALOG:
			onInit(hDlg, wParam);
			return TRUE;
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				// Click on the action button (backup or restore).
				case IDACTION:
					onAction(hDlg, g_currentAction);
					return TRUE;
				// Click on the select restore file button.
				case IDC_BUTTON_SELECT_BACKUP_FILE:
					onClick_SelectRecoveryFile(hDlg, g_szBckFileName, g_szBckFileNameShortened, g_szBckTitleName, BACKUP);
					return TRUE;
				// Click on the select backup file button.
				case IDC_BUTTON_SELECT_RECOVERY_FILE:
					onClick_SelectBackupFile(hDlg, g_szRecFileName, g_szRecFileNameShortened, g_szRecTitleName, RECOVERY);
					return TRUE;
				// Click on the recovery radio button.
				case IDC_RADIO_RECOVERY:
					onClick_RadioRecovery(hDlg);
					return TRUE;
				// Click on the backup radio button.
				case IDC_RADIO_BACKUP:
					onClick_RadioBackup(hDlg);
					return TRUE;
				// Click on the Log button.
				case IDC_BUTTON_LOG:
					onClick_ButtonLog(hDlg);
					return TRUE;
				// Click on the CMD button.
				case IDC_BUTTON_CMD:
					onClick_ButtonCmd();
					return TRUE;
				// Click on the combo box list.
				case IDC_COMBO_DRIVES:
					switch(HIWORD(wParam))
					{
						// Sent when the user changes the current selection in the list of the combo box.
						case CBN_SELCHANGE:
							// wParam: The LOWORD contains the control identifier of the combo box.
							onChange_ComboBoxDrives(hDlg, LOWORD(wParam));
							// Bug fix #3:
							// Forces the entire dialog to be redrawn. If you omit it, the information about the selected drive will not be redrawn properly.
							InvalidateRect(hDlg, NULL, TRUE);
							return TRUE;
					}
					break;
				// Click on the close button (X).
				case IDCANCEL:
					onClose(hDlg); 
					return TRUE;
			}
			break;
		case WM_CTLCOLORSTATIC:
			// I use a system brush (GetStockObject()) to display hint texts in a specific color.
			// This does not then have to be made explicitly free (DeleteObject()).
			// See also under: https://learn.microsoft.com/de-de/windows/win32/controls/wm-ctlcolorstatic
		
			// Color text note for warnings in red.
			if ((HWND) lParam == GetDlgItem(hDlg, IDC_STATIC_INFO_BACKUP) || (HWND) lParam == GetDlgItem(hDlg, IDC_STATIC_INFO_RECOVERY))
			{	
				hBrushStatic = (HBRUSH) GetStockObject(HOLLOW_BRUSH);	
				SetBkMode((HDC) wParam, TRANSPARENT);
				SetTextColor((HDC) wParam, colorRef[0]);
				return (INT_PTR) hBrushStatic;
			}

			// Colored text note for information that basically requires some attention, in blue.
			if ((HWND) lParam == GetDlgItem(hDlg, IDC_STATIC_VOLUME_NAME_VALUE) || (HWND) lParam == GetDlgItem(hDlg, IDC_STATIC_VOLUME_SPACE_VALUE))
			{
				hBrushStatic = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
				SetBkMode((HDC) wParam, TRANSPARENT);
				SetTextColor((HDC) wParam, colorRef[1]);
				return (INT_PTR) hBrushStatic;
			}
			break;
		case WM_CLOSE:
			// I'm not sure if I need to free up memory on an explicitly
			// allocated memory when I exit the program - so I just do that.
			if (g_pstrExePath != NULL)
			{
				free((void *) g_pstrExePath);
				g_pstrExePath = NULL;
			}
			
			if (g_pstrBatchPath != NULL)
			{
				free((void *) g_pstrBatchPath);
				g_pstrBatchPath = NULL;
			}

			// Close the application.
			DestroyWindow(hDlg);
			return TRUE;
		case WM_DESTROY:
			if (hBrushStatic != NULL)
			{
				DeleteObject(hBrushStatic);
			}
			PostQuitMessage(0);
			return TRUE;
	}
	return FALSE;
}

/**
 * Event that is called when the dialog window is initialized.
 */
void onInit(HWND hwndDlg, WPARAM wParam)
{
	HWND hwndActionBtn;
	HWND hwndBackupRadioBtn;
	g_currentAction = BACKUP;

	// Get the module path.
	g_pstrExePath = GetOwnPath();

	// Get the batch file path.
	g_pstrBatchPath = GetBatchPath((TCHAR *)g_pstrExePath);
		
	// Init controls.

	// Disable select recovery file button.
	EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_SELECT_RECOVERY_FILE), FALSE);

	// App version.
	SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_APP_INFO), szAppVersion);

	// Set the text from the central button to "Backup" and disable the button for now.
	hwndActionBtn = GetDlgItem(hwndDlg, IDACTION);

	if (hwndActionBtn != NULL)
	{
		SetDlgItemText(hwndDlg, IDACTION, TEXT("Backup"));
		EnableWindow(hwndActionBtn, FALSE);
	}

	// Mark the backup radio button as default (selected).
	// Get the window handle to the radio button
	hwndBackupRadioBtn = GetDlgItem(hwndDlg, IDC_RADIO_BACKUP);

	if (hwndBackupRadioBtn != NULL)
	{
		SendMessage(hwndBackupRadioBtn, BM_SETCHECK, 1, 0L);
	}

	// Initialize the ComboBox with system drives available in the system.
	InitComboBox_LogicalDrives(hwndDlg, GetDlgItem(hwndDlg, IDC_COMBO_DRIVES), GetDlgItem(hwndDlg, IDC_STATIC_VOLUME_NAME_VALUE));
}

/**
 * The event that is triggered when the dialog box is to be closed.
 */
void onClose(HWND hwnd)
{
	SendMessage(hwnd, WM_CLOSE, 0, 0);
}

/**
 * Event that occurs when the Backup radio button is clicked.
 */
void onClick_RadioBackup(HWND hDlg)
{
	// Should the action button be switch on?
	BOOL bEnableAction = FALSE;

	// Set action to backup.
	g_currentAction = BACKUP;

	// Set radio button global flag.
	g_bIsRecoveryBtnSelected = FALSE;

	// Enable the button select recovery file.
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SELECT_BACKUP_FILE), TRUE);

	// Disable the button select backup file.
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SELECT_RECOVERY_FILE), FALSE);

	// Change the label for the action button.
	SetWindowText(GetDlgItem(hDlg, IDACTION), szBackupBtnText);

	// Check if the backup file you need is already selected.
	// User must first select a backup file.
	bEnableAction = (g_szBckFileName == NULL || lstrlen(g_szBckFileName) == 0) ? FALSE : TRUE;
	EnableWindow(GetDlgItem(hDlg, IDACTION), bEnableAction);
}

/**
 * Event that occurs when the Recovery radio button is clicked.
 */
void onClick_RadioRecovery(HWND hDlg)
{
	// Should the action button be switch on?
	BOOL bEnableAction = FALSE;

	// Set action to backup.
	g_currentAction = RECOVERY;

	// Set radio button global flag.
	g_bIsRecoveryBtnSelected = TRUE;

	// Enable the button select backup file.
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SELECT_RECOVERY_FILE), TRUE);

	// Disable the button select recovery file.
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SELECT_BACKUP_FILE), FALSE);

	// Change the label for the action button.
	SetWindowText(GetDlgItem(hDlg, IDACTION), szRecoveryBtnText);

	// Check if the backup file you need is already selected.
	// User must first select a backup file.
	bEnableAction = (g_szRecFileName == NULL || lstrlen(g_szRecFileName) == 0) ? FALSE : TRUE;
	EnableWindow(GetDlgItem(hDlg, IDACTION), bEnableAction);
}

/**
 * Event fired when the action button (Backup/Recovery) is clicked.
 */
void onAction(HWND hDlg, action_type action)
{
	TCHAR szBuffer[1024];
	DWORD rcRunAction = 0;

	// Recovery action.
	if ((action == RECOVERY) && (g_bIsRecoveryBtnSelected == TRUE))
	{
		if (MessageBox(hDlg, szQuestRunRecText, szRunRecoveryText, MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
		{
			// Run action and wait until the task is finished.
			rcRunAction = ActionEx(g_pstrBatchPath, TEXT("%s %s %s"), szActionRecovery, g_szRecFileName, szDismLogFilePath);

			if (rcRunAction == RUN_ACTION_SUCCESSFUL)
			{
				MessageBox(hDlg, szActionRecFin, szRunRecoveryText, MB_ICONINFORMATION | MB_OK);
			}
			else 
			{
				// ActionEx() failed.
				_stprintf_s(szBuffer, 1024, szActionFailed, rcRunAction);
				MessageBox(hDlg, szBuffer, szRunRecoveryText, MB_ICONERROR | MB_OK);
			}
		}
	}

	// Backup action.
	if ((action == BACKUP) && (g_bIsRecoveryBtnSelected == FALSE))
	{
		if (MessageBox(hDlg, szQuestRunBckText, szRunBackupText, MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
		{
			// Run action and wait until the task is finished.
			rcRunAction = ActionEx(g_pstrBatchPath, TEXT("%s %s %s %s"), szActionBackup, g_szBckFileName, szDismLogFilePath, g_szBckVolumeName);
			
			if (rcRunAction == RUN_ACTION_SUCCESSFUL)
			{
				// Check if the backup was successful.
				if (FileExists(g_szBckFileName) == TRUE)
				{
					// The backup was probably created successfully because the backup file was created.
					_stprintf_s(szBuffer, 1024, szActionBckFin, g_szBckFileNameShortened);
					MessageBox(hDlg, szBuffer, szRunBackupText, MB_ICONINFORMATION | MB_OK);
				}
				else
				{
					// The backup action has failed.
					MessageBox(hDlg, szActionBckFailed, szRunBackupText, MB_ICONERROR | MB_OK);
				}
			}
			else
			{
				// ActionEx() failed.
				_stprintf_s(szBuffer, 1024, szActionFailed, rcRunAction);
				MessageBox(hDlg, szBuffer, szRunBackupText, MB_ICONERROR | MB_OK);
			}
		}
	}
}

/**
 * Initializes a standard file selection dialog with values.
 */
void InitializeFileDlg(HWND hwndOwner)
{
	static TCHAR szFilter[] = TEXT("Backup file (*.wim)\0*.wim\0*.*\0*.*\0\0");
	
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndOwner;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = NULL;		// Will be set when opening and closing the dialog.
	ofn.nMaxFile = MAX_PATH;	// 260 Chars
	ofn.lpstrFileTitle = NULL;		
	ofn.nMaxFileTitle = MAX_PATH;	// 260 Chars
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = 0;			// Will be set when opening and closing the dialog.
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("wim");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
}

/**
 * Function that handles the selection of the backup and restore file.
 */
void SelectFile(HWND hwndDlg, PTSTR pstrFileName, PTSTR pstrFileNameShortened, PTSTR pstrTitleName, action_type action)
{
	// Shows (1) that a file has been selected, otherwise 0.
	// This is the case if the dialog box has been canceled.
	BOOL bFileSelected = FALSE;
	HRESULT hResult = 0;
	HWND hwndStaticFilePath = NULL;
	int ctrlID_staticFilePath = 0;
	
	if (action == BACKUP)
	{
		ctrlID_staticFilePath = IDC_STATIC_BACKUP_FILE_PATH;
	}
	else 
	{
		ctrlID_staticFilePath = IDC_STATIC_RECOVERY_FILE_PATH;
	}

	hwndStaticFilePath = GetDlgItem(hwndDlg, ctrlID_staticFilePath);

	// Initialize file selection dialog.
	InitializeFileDlg(hwndDlg);

	// Call the file selection dialog and let the user select a *.wim file (file for the backup).
	bFileSelected = OpenFileDlg(hwndDlg, pstrFileName, pstrTitleName, action);

	// Output selected file in the label window.
	if (bFileSelected == TRUE)
	{
		// First reset the string to avoid appending a new string in the output window.
		*pstrFileNameShortened = '\0';

		//memset(pstrFileNameShortened, 0, sizeof(pstrFileNameShortened));

		// Copy the file path and compact it if needed.
		hResult = StringCchCat(pstrFileNameShortened, MAX_PATH, ofn.lpstrFile);
		
		// Shorten the file path and output it.
		if (hResult == S_OK)
		{
			ShortenFilePath(hwndStaticFilePath, pstrFileNameShortened);
			SetWindowText(hwndStaticFilePath, pstrFileNameShortened);
		}
		else
		{
			// Output the origin path.
			SetWindowText(hwndStaticFilePath, ofn.lpstrFile);
		}

		// Set global flag to indicate readiness for restore.
		g_currentAction = action;

		// and enable the action button.
		EnableWindow(GetDlgItem(hwndDlg, IDACTION), TRUE);
	}
}

/**
 * An event that occurs when the user clicks on the SelectRecoveryFile button.
 */
void onClick_SelectRecoveryFile(HWND hwnd, PTSTR pstrFileName, PTSTR pstrFileNameShortened, PTSTR pstrTitleName, action_type action)
{
	SelectFile(hwnd, pstrFileName, pstrFileNameShortened, pstrTitleName, action);	
}

/**
 * An event that occurs when the user clicks on the SelectBackupFile button.
 */
void onClick_SelectBackupFile(HWND hwnd, PTSTR pstrFileName, PTSTR pstrFileNameShortened, PTSTR pstrTitleName, action_type action)
{
	SelectFile(hwnd, pstrFileName, pstrFileNameShortened, pstrTitleName, action);
}

/**
 * An event that occurs when the user clicks on the Log button.
 */
void onClick_ButtonLog(HWND hwnd)
{
	#define MAX_BUFFER 512

	TCHAR szBuffer[MAX_BUFFER];
	const TCHAR * pLog = NULL;				// Helper pointer to the full path of the original DISM log file, e.g. "%systemdrive%\Tools\dism.log".
	TCHAR * pEnv = NULL;					// Helper pointer to the final obtained %-cleaned and zero terminated environment variable, e.g. "systemdrive".
	TCHAR * pszEnv = NULL;					// String with the name of the environment variable (null terminated).
	TCHAR * pszEnvVal = NULL;				// String with the value of the enviornment variable (null terminated).
	TCHAR * pszNewDismLogFilePath = NULL;	// String with the newly composed full path to the log file (null terminated).
		
	// Allocate needed memory. Don't forget to free the memory!
	pszEnv = (TCHAR *) calloc(MAX_BUFFER * 2, sizeof(TCHAR));
	pszEnvVal = (TCHAR *) calloc(MAX_BUFFER, sizeof(TCHAR));
	pszNewDismLogFilePath = (TCHAR *) calloc(MAX_BUFFER * 2, sizeof(TCHAR));
	
	// Check if pointers are valid. For allocated memory, whether allocation was possible.
	if (szDismLogFilePath == NULL || pszEnv == NULL || pszEnvVal == NULL || pszNewDismLogFilePath == NULL)
	{
		return;
	}

	/**
	 * Traversing the full path to the log file, checking if it contains an environment variable,
	 * then extracting it to determine its actual value.
	 **/
	pLog = szDismLogFilePath;
	pEnv = pszEnv;

	while (*pLog != '\0')
	{
		if (*pLog == '%')
		{
			pLog++;
			while(*pLog != '%')
			{
				*pEnv++ = *pLog++;
			}
			*pEnv = '\0';
		}
		pLog++;
		break;
	}

	// Get the value of the environment variable.
	// If the function fails, the return value is zero
	if (GetEnvironmentVariable(pszEnv, pszEnvVal, MAX_BUFFER) == 0)
	{
		// Either the buffer is too small or the environment variable is not present.
		// Build a new complete path to the log file.
		_stprintf_s(pszNewDismLogFilePath, MAX_BUFFER * 2, TEXT("%s"), szDismLogFilePath, pLog);
	}
	else
	{
		// Build a new complete path to the log file.
		_stprintf_s(pszNewDismLogFilePath, MAX_BUFFER * 2, TEXT("%s%s"), pszEnvVal, pLog);
	}
	
	// Check if the log file is present.
	if (FileExists(pszNewDismLogFilePath) == FALSE)
	{
		_stprintf_s(szBuffer, MAX_BUFFER, szFileNotFound, pszNewDismLogFilePath);
		MessageBox(hwnd, szBuffer, szInfo, MB_ICONINFORMATION | MB_OK);
	}
	else
	{
		// Launching Notepad with the current log file.
		ShellExecute(NULL, TEXT("open"), pszNewDismLogFilePath , NULL, NULL, SW_SHOWNORMAL);
	}	
	
	// Release memory that is no longer needed.
	if (pszEnv != NULL)
	{
		free((void *) pszEnv);
		pszEnv = NULL;
	}

	if (pszEnvVal != NULL)
	{
		free((void *) pszEnvVal);
		pszEnvVal = NULL;
	}

	if (pszNewDismLogFilePath != NULL)
	{
		free((void *) pszNewDismLogFilePath);
		pszNewDismLogFilePath = NULL;
	}
}

/**
 * An event that occurs when the user clicks on the CMD button.
 */
void onClick_ButtonCmd()
{
	// Launch a normal CMD shell (cmd.exe).
	ShellExecute(NULL, TEXT("open"), TEXT("cmd.exe"), NULL, NULL, SW_SHOWNORMAL);
}

/**
 * Opens a standard file selection dialog box to allow the user to select a file.
 * This function requires the Shlwapi.dll
 */
BOOL OpenFileDlg(HWND hwndOwner, PTSTR pstrFileName, PTSTR pstrTitleName, action_type action)
{
	ofn.hwndOwner = hwndOwner;
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrFileTitle = pstrTitleName;
	ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;

	if (action == BACKUP)
	{
		// Backup
		return GetSaveFileName(&ofn);
	}
	else
	{
		// Restore
		return GetOpenFileName(&ofn);
	}
}

/** 
 * This function shortens the path of a file for use in a control.
 * For example, C:\ProgramFiles\MyFile.wim becomes C:\Program...\MyFile.wim.
 * Thus the path can be theoretically arbitrarily long.
 */
void ShortenFilePath(HWND hwndDlgCtrl, PTSTR pstrShortenFileName)
{
	// Attention: This function is actually obsolete, because you can
	// achieve the same result via the PathEllipsis control property without
	// having to program a line of code.

	// We need the right font, which can be inserted into the DC.
	// You can get a DC for a control with GetDC(), but possibly 
	// (a matter of luck) the right font will not be selected in that DC.
	// Therefore, we get the font via WM_GETFONT and temporarily insert it into the DC.
	// Then use PathCompactPath and clean up again afterwards.
	
	RECT clientRect;
	int iWidth = 0;

	// We need first the DC handle from the control,
	HDC hDC = GetDC(hwndDlgCtrl);
	
	// then we need the current font and insert it into the DC.
	HFONT hFont = (HFONT)SendMessage(hwndDlgCtrl, WM_GETFONT, 0, 0);
	HFONT hFontOld = (HFONT)SelectObject(hDC, hFont);

	// Get the window rect.
	// We need this to calculate the width of the control. 
	GetWindowRect(hwndDlgCtrl, &clientRect);
	iWidth = clientRect.right - clientRect.left;
	
	// Now get the new shortened path.
	PathCompactPath(hDC, pstrShortenFileName, iWidth);

	// Clean it up.
	SelectObject(hDC, hFontOld);
	DeleteObject(hFontOld);
	ReleaseDC(hwndDlgCtrl, hDC);
}

/*
 * Returns the absolute path to this module (exe), e.g. c:\programfiles\tool\myapp.exe
 */
const TCHAR * GetOwnPath()
{
	// Allocate memory for the path string. Don't forget to free the memory!
	TCHAR * ptrOwnPath = (TCHAR *) calloc(MAX_PATH, sizeof(TCHAR));

	// If NULL is passed, the handle to the current module (exe) is supplied.
	HMODULE hModule = GetModuleHandle(NULL);
	
	if (hModule != NULL)
	{
		// Get the whole path.
		GetModuleFileName(hModule, ptrOwnPath, MAX_PATH * sizeof(TCHAR));
	}
	return ptrOwnPath;
}

/*
 * Returns an absolute path to the action batch file. Based on the data from GetOwnPath().
 */
const TCHAR * GetBatchPath(TCHAR * szModulePath)
{
	TCHAR * pszModulePath = szModulePath;
	TCHAR * ptrBatchPath = (TCHAR *) calloc(MAX_PATH, sizeof(TCHAR));	// Don't forget to free the memory!
	TCHAR * pTmp = ptrBatchPath;
	TCHAR * pBackSlash = pTmp;
	HRESULT hResult = 0;

	// Go through the entire module path string until it ends.
	while (*pszModulePath != '\0')
	{

		*pTmp = *pszModulePath;
		// Check if char is a backslash (remember it).
		if (*pTmp == '\\')
		{
			// Note the last backslash as a pointer address.
			pBackSlash = pTmp;
		}
		pTmp++;
		pszModulePath++;
	}

	// Terminate the still temporary batch path string at the last backslash.
	*++pBackSlash = '\0';
	
	// Concatenate the batch file name at the end of the batch string and return it.
	hResult = StringCchCat(ptrBatchPath, MAX_PATH * sizeof(TCHAR), szBatchFileName); 
	return ptrBatchPath;
}

/*
 * This function executes the actual batch job and waits until it is finished.
 */
void Action(const TCHAR * szActionFile, PTSTR szParameters)
{
	SHELLEXECUTEINFO sei = {0};

	ZeroMemory(&sei, sizeof(sei));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	sei.lpFile = szActionFile;
	sei.lpParameters = szParameters;
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOW;
	sei.hInstApp = NULL;
	
	// Execute the action job.
	ShellExecuteEx(&sei);

	// Wait until the action process is finished. 
	WaitForSingleObject(sei.hProcess, INFINITE);
	CloseHandle(sei.hProcess);
}

/*
 * This function executes the actual batch job and waits until it is finished.
 * This function corresponds to the Action() function with the 
 * difference that it takes dynamic parameter lists as last parameter
 * and returns a run code.
 * 
 * Return: If everything was okay,
 * i.e. the batch ran to the end, the return code should be 0x400.
 * Other values always signal an error code.
 * The value SHELL_EXECUTE_EX_FAILED means
 * that an error occurred during the execution of ShellExecuteEx().
 */
DWORD ActionEx(const TCHAR * szActionFile, TCHAR * szFormat, ...)
{
	SHELLEXECUTEINFO sei = {0};
	DWORD exitCode = 0;
	const int SHELL_EXECUTE_EX_FAILED = RUN_ACTION_SHELLEX_FAILED;

	// Dynamic parameter list...
	// See also: https://docs.microsoft.com/de-de/cpp/c-runtime-library/reference/vsnprintf-s-vsnprintf-s-vsnprintf-s-l-vsnwprintf-s-vsnwprintf-s-l?view=msvc-160
	// See also: See also: Book by Charles Petzold, 5th edition Windows Programming, page 42 (SCRNSIZE.C). 
	TCHAR szBuffer[2048];
	va_list pArgList;

	va_start (pArgList, szFormat);
	_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), _TRUNCATE, szFormat, pArgList); 
	va_end (pArgList);
	
	ZeroMemory(&sei, sizeof(sei));

	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	sei.lpFile = szActionFile;
	sei.lpParameters = szBuffer;
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOW;
	sei.hInstApp = NULL;
		
	// Execute the action job.
	if (ShellExecuteEx(&sei) == TRUE)
	{
		// Wait until the action process is finished. 
		WaitForSingleObject(sei.hProcess, INFINITE);

		// The process was finished.
		// Get the value of %errorlevel% from the batch.
		// If everything was okay, i.e. the batch ran to the end, then we get a value of 0x400.
		GetExitCodeProcess(sei.hProcess, &exitCode);
		CloseHandle(sei.hProcess);
		
		return exitCode;
	}
	return SHELL_EXECUTE_EX_FAILED;
}

/*
 * Checks if a file exists and returns TRUE in this case else FALSE.
 */
BOOL FileExists(const wchar_t * szFile)
{
	errno_t err = 0;

	// Check for existence.
	if ((err = _taccess_s(szFile, 0)) == 0)
	{
		return TRUE;
	}
	return FALSE;
}

/*
 * Initializes the ComboBox with drives available in the system.
 *
 */
void InitComboBox_LogicalDrives(HWND hwndDlg, HWND hwndComboBox, HWND hwndStaticAdditionalInfo)
{
	DWORD drives = GetLogicalDrives();
	int i = 0;
	TCHAR driveLetter[16];
	DWORD iSelectedItem = 0;


	// Go through all the drive letters and find which drives are available.
	for(i = 0; i < 26; i++)
	{
		// Explanation:
		// H G F E D C B A
		// 0 0 0 0 1 1 0 0
		// 1 << 0 = 1 (2 ^ 0)
		// 1 << 1 = 2 (2 ^ 1)
		// 1 << 2 = 4 (2 ^ 2)
		// 1 << 3 = 8 (2 ^ 3)
		if( (drives & ( 1 << i )) != 0)
		{
			// Compose the drive letter.
			_stprintf_s(driveLetter, sizeof(driveLetter)/sizeof(TCHAR), TEXT("%c:\\"), TEXT('A') + i);
			
			// Add the letter into the comob box.
			SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)driveLetter);
		}
	}

	// Select the first Item (0) in the combobox.
	SendMessage(hwndComboBox, CB_SETCURSEL, (WPARAM)iSelectedItem, (LPARAM)0);
	
	// Triggering the "combo box selection changed" event.
	onChange_ComboBoxDrives(hwndDlg, IDC_COMBO_DRIVES);
}

/*
 * An event that occurs when the user selects a new element in the ComboBox.
 *
 */
void onChange_ComboBoxDrives(HWND hwndDlg, WORD iID_ComboBox)
{
	TCHAR driveLetter[16];
	TCHAR driveInfo[MAX_PATH + 1];
	TCHAR spaceInfo[MAX_PATH + 1];
	
	// Get the element selected in the combo box.
	LPARAM iSelectedItem = SendMessage(GetDlgItem(hwndDlg, iID_ComboBox), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

	// Get the string (volume like e. g. c:\) from the list of a combo box.
	SendMessage(GetDlgItem(hwndDlg, iID_ComboBox), CB_GETLBTEXT, (WPARAM)iSelectedItem, (LPARAM)driveLetter);

	// Save the selection (as value) in a global variable.
	_tcscpy_s(g_szBckVolumeName, sizeof(g_szBckVolumeName)/sizeof(TCHAR), driveLetter);

	// Get the volume information by the selected drive.
	GetVolumeInfo(driveLetter, driveInfo, MAX_PATH + 1);

	// Show the drive information in the static window. 
	SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_VOLUME_NAME_VALUE), driveInfo);

	// Show the space informtion in the static window.
	GetDiskSpaces(driveLetter, spaceInfo, MAX_PATH + 1);
	SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_VOLUME_SPACE_VALUE), spaceInfo);
}

/*
 * Returns formatted volume information such as drive type,
 * file system type and drive label based on the drive letter in an output string (pDriveInfoBuffer).
 */
void GetVolumeInfo(LPCTSTR pDriveLetter, LPTSTR pDriveInfoBuffer, DWORD nDriveInfoSize)
{
	TCHAR volumeName[MAX_PATH+1];
	TCHAR fileSysName[MAX_PATH+1];
	DWORD serialNumber = 0;
	DWORD maxCompLength = 0;
	DWORD fileSysFlags = 0;

	BOOL bGetVolInf = FALSE;
	UINT iDrvType = 0;
	TCHAR driveType[MAX_PATH];
	size_t size = sizeof(driveType)/sizeof(TCHAR);

	// Get drive information.
	bGetVolInf = GetVolumeInformation(pDriveLetter, volumeName, sizeof(volumeName)/sizeof(TCHAR),
						&serialNumber, &maxCompLength, &fileSysFlags, fileSysName, sizeof(fileSysName)/sizeof(TCHAR));
	
	// Identify the drive type.
	iDrvType = GetDriveType(pDriveLetter);
	
	switch(iDrvType)
	{
		case 0: 
			_stprintf_s(driveType, size, TEXT("%s"), TEXT("unknown"));
			break;
		case 1: 
			_stprintf_s(driveType, size, TEXT("%s"), TEXT("path?"));
			break;
		case 2: 
			_stprintf_s(driveType, size, TEXT("%s"), TEXT("external"));
			break;
		case 3:
			_stprintf_s(driveType, size, TEXT("%s"), TEXT("local disk"));
			break;
		case 4:
			_stprintf_s(driveType, size, TEXT("%s"), TEXT("network"));
			break;
		case 5:
			_stprintf_s(driveType, size, TEXT("%s"), TEXT("CD-ROM"));
			break;
		case 6: 
			_stprintf_s(driveType, size, TEXT("%s"), TEXT("RAM"));
			break;
		default:
			_stprintf_s(driveType, size, TEXT("%s"), TEXT("unknown"));
			break;
	}

	// Output default value if there is no label for the drive.
	if (_tcslen(volumeName) == 0)
	{
		_stprintf_s(volumeName, sizeof(volumeName)/sizeof(TCHAR), TEXT("%s"), TEXT("No label"));
	}

	// Compose additional info about the drive.
	_stprintf_s(pDriveInfoBuffer, nDriveInfoSize, TEXT("%.15s | %s (%s)"), volumeName, driveType, fileSysName);
}

/*
 * Provides formatted capacity information (free space of maximum memory) for a drive in an output string (pSpaceInfoBuffer).
 */
void GetDiskSpaces(LPCTSTR pDriveLetter, LPTSTR pSpaceInfoBuffer, DWORD nSpaceInfoBufferSize)
{
	unsigned __int64 i64TotalNumberOfBytes;
	unsigned __int64 i64TotalNumberOfFreeBytes;
	unsigned __int64 i64FreeBytesAvailableToCaller;
	BOOL bGetDiskFreeSpaceEx = FALSE;

	// Get memory space data.
	bGetDiskFreeSpaceEx = GetDiskFreeSpaceEx(pDriveLetter, (PULARGE_INTEGER)&i64FreeBytesAvailableToCaller, (PULARGE_INTEGER)&i64TotalNumberOfBytes, (PULARGE_INTEGER)&i64TotalNumberOfFreeBytes);

	if (bGetDiskFreeSpaceEx == TRUE)
    {
		// Compose memory space info.
		_stprintf_s(pSpaceInfoBuffer, nSpaceInfoBufferSize, TEXT("Capacity: %I64u GB (free) / %I64u GB"), i64TotalNumberOfFreeBytes / (1024*1024*1024), i64TotalNumberOfBytes / (1024*1024*1024));
    }
}