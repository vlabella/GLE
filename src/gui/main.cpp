/***********************************************************************************
 * QGLE - A Graphical Interface to GLE                                             *
 * Copyright (C) 2006  A. S. Budden & J. Struyf                                    *
 *                                                                                 *
 * This program is free software; you can redistribute it and/or                   *
 * modify it under the terms of the GNU General Public License                     *
 * as published by the Free Software Foundation; either version 2                  *
 * of the License, or (at your option) any later version.                          *
 *                                                                                 *
 * This program is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   *
 * GNU General Public License for more details.                                    *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License               *
 * along with this program; if not, write to the Free Software                     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. *
 *                                                                                 *
 * Also add information on how to contact you by electronic and paper mail.        *
 ***********************************************************************************/

/******************************************************************************
 * main.cpp: Contains the main function.  As with all QT main functions, this *
 * is very sparse: create the application object and show the main window.    *
 ******************************************************************************/

#include <QApplication>
#include "mainwindow.h"
#include "dialogues.h"

#ifdef Q_OS_WIN32
	#include <windows.h>
#endif

#include <locale.h>

#ifdef _M_AMD64
#define CONTEXT_EIP context->Rip
#define CCONTEXT_EIP Context->Rip
#define CONTEXT_EBP context->Rbp
#define CCONTEXT_EBP Context->Rbp
#define CONTEXT_ESP context->Rsp
#define CCONTEXT_ESP Context->Rsp
#else
#define CONTEXT_EIP context->Eip
#define CCONTEXT_EIP Context->Eip
#define CONTEXT_EBP context->Ebp
#define CCONTEXT_EBP Context->Ebp
#define CONTEXT_ESP context->Esp
#define CCONTEXT_ESP Context->Esp
#endif

void findDependencies(int argc, char** argv, int argi) {
	GLEInterface* gleInterface = GLEGetInterfacePointer();
	SoftwareLocateDialogue dial(NULL, gleInterface, 0);
	dial.setWindowTitle("GLE - Finding Dependencies");
	if (gleInterface->initializeGLE("qgle", argc, argv)) {
		if (argi+1 < argc) {
			QString file = QString::fromUtf8(argv[argi+1]);
			QFileInfo fileinfo(file);
			if (fileinfo.exists()) {
				if (fileinfo.isDir()) {
					dial.setRootDir(file);
				} else {
					int ret = QMessageBox::question(&dial, dial.tr("QGLE"),
                                dial.tr("Import settings from previous GLE installation?"),
                                QMessageBox::Yes | QMessageBox::No);
					if (ret == QMessageBox::Yes) {
						dial.setRootDir(file);
					}
				}
			}
		}
		dial.searchAuto();
		dial.exec();
		gleInterface->saveRCFile();
	} else {
		dial.disableAll();
		dial.enableOK(true);
		dial.exec();
	}
}

void doQGLECrashRecover(QString file) {
	CrashRecoverDialogue dial(file);
	dial.exec();
	QDir dir;
	dir.remove(file);
}

#define QGLE_SCRIPT_NAME_BACKUP_SIZE 1000
char QGLEScriptNameBackup[QGLE_SCRIPT_NAME_BACKUP_SIZE+1];

void SetQGLEScriptNameBackup(QString name) {
	QByteArray arr = name.toUtf8();
	strncpy(QGLEScriptNameBackup, arr.constData(), QGLE_SCRIPT_NAME_BACKUP_SIZE);
	QGLEScriptNameBackup[QGLE_SCRIPT_NAME_BACKUP_SIZE] = 0;
}

#ifdef Q_OS_WIN32

#include <dbghelp.h>

typedef BOOL (__stdcall *pStackWalk)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME StackFrame,
 									 PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
									 PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
									 PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
									 PTRANSLATE_ADDRESS_ROUTINE TranslateAddress);

void PrintStackTrace(CONTEXT* context, HANDLE hLogFile, char* szOutBuffer) {
	DWORD NumBytes;
   char szModuleName[MAX_PATH+1];
	DWORD dwRes = GetModuleFileNameA(NULL, szModuleName, MAX_PATH);
	if (dwRes == 0) {
		sprintf(szOutBuffer, "   Error: GetModuleFileName fails.\n");
		WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
		return;
	}
   char* backSlash = strrchr(szModuleName, '\\');
   if (backSlash != 0) {
      backSlash[1] = 0;
   }
   strcat(szModuleName, "dbghelp.dll");
	HMODULE dbgHelp = LoadLibraryA(szModuleName);
	if (dbgHelp == NULL) {
		sprintf(szOutBuffer, "   Error: %s not found.\n", szModuleName);
		WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
      // try without path
      dbgHelp = LoadLibraryA("dbghelp.dll");
      if (dbgHelp == NULL) {
         sprintf(szOutBuffer, "   Error: dbghelp.dll not found.\n");
         WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
         return;
      }
	}
	pStackWalk stackWalk = (pStackWalk)GetProcAddress(dbgHelp, "StackWalk");
	if (stackWalk == NULL) {
		sprintf(szOutBuffer, "   Error: dbghelp.dll does not include StackWalk.\n");
		WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
		return;
	}
	PFUNCTION_TABLE_ACCESS_ROUTINE funcTable = (PFUNCTION_TABLE_ACCESS_ROUTINE)GetProcAddress(dbgHelp, "SymFunctionTableAccess");
	PGET_MODULE_BASE_ROUTINE moduleBase = (PGET_MODULE_BASE_ROUTINE)GetProcAddress(dbgHelp, "SymGetModuleBase");
	if (funcTable == NULL || moduleBase == NULL) {
		sprintf(szOutBuffer, "   Error: dbghelp.dll does not include the required functions.\n");
		WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
		return;
	}
	STACKFRAME frame;
	memset(&frame, 0, sizeof(frame));
	frame.AddrPC.Offset = CONTEXT_EIP;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = CONTEXT_EBP;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = CONTEXT_ESP;
	frame.AddrStack.Mode = AddrModeFlat;
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	char szBuf[MAX_PATH*2];
	MEMORY_BASIC_INFORMATION MemInfo;
	while (true) {
		DWORD imageType = IMAGE_FILE_MACHINE_I386;
		if (!stackWalk(imageType, hProcess, hThread, &frame, context, NULL, funcTable, moduleBase, NULL)) {
			sprintf(szOutBuffer, "   StackWalk end offset: %08lx - error code = %ld\n", frame.AddrPC.Offset, GetLastError());
			WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
			break;
		}
		if (frame.AddrPC.Offset == frame.AddrReturn.Offset)	{
			sprintf(szOutBuffer, "   Infinite call stack - offset: %08lx\n", frame.AddrPC.Offset);
			WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
			break;
		}
		if (frame.AddrPC.Offset == 0) {
			break;
		}
		sprintf(szOutBuffer, "   Call: 0x%08lx", frame.AddrPC.Offset);
		WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
		if (VirtualQuery((void*)frame.AddrPC.Offset, &MemInfo, sizeof(MemInfo)) != 0) {
			GetModuleFileNameA((HINSTANCE)MemInfo.AllocationBase, szBuf, sizeof(szBuf)-2);
			char* fName = strrchr(szBuf, '\\');
			if (fName != NULL) fName++;
			else fName = szBuf;
			sprintf(szOutBuffer, " (module %s)", fName);
			WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
		}
		sprintf(szOutBuffer, "\n");
		WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
	}
}

LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS *info) {
	// Crash report code (intentionally does not use Qt)
	static bool bFirstTime = true;
	if (!bFirstTime) {
		// Recursive call means this routine crashed
		return EXCEPTION_CONTINUE_SEARCH;
	}
	bFirstTime = false;
	// Get temporary path name
	TCHAR lpPathBuffer[MAX_PATH+1];
	DWORD dwRetVal = GetTempPath(MAX_PATH, lpPathBuffer);
	if (dwRetVal > MAX_PATH || dwRetVal == 0) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	// Get temporary file name
	TCHAR szTempName[MAX_PATH+1];
	UINT uRetVal = GetTempFileName(lpPathBuffer, TEXT("gle"), 0, szTempName);
	if (uRetVal == 0) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	// Create crash log file
	HANDLE hLogFile = CreateFile(szTempName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hLogFile == INVALID_HANDLE_VALUE) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	// Find out module in which crash occurred
	CONTEXT* Context = info->ContextRecord;
	char szCrashModulePathName[MAX_PATH*2];
	strcpy(szCrashModulePathName, "Unknown");
	MEMORY_BASIC_INFORMATION MemInfo;
	if (VirtualQuery((void*)CCONTEXT_EIP, &MemInfo, sizeof(MemInfo)) != 0) {
		GetModuleFileNameA((HINSTANCE)MemInfo.AllocationBase, szCrashModulePathName, sizeof(szCrashModulePathName)-2);
	}
	// Print error code to log file
	DWORD NumBytes;
	char szOutBuffer[5000];
	char* szCrashModuleName = strrchr(szCrashModulePathName, '\\');
	if (szCrashModuleName != NULL) szCrashModuleName++;
	else szCrashModuleName = szCrashModulePathName;
	sprintf(szOutBuffer, "Error code 0x%08x in module %s at %04x:%08x.\n", (unsigned int)info->ExceptionRecord->ExceptionCode, szCrashModuleName, (unsigned int)Context->SegCs, (unsigned int)CCONTEXT_EIP);
	WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
	if (info->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION && info->ExceptionRecord->NumberParameters >= 2) {
		if (info->ExceptionRecord->ExceptionInformation[0]) {
			sprintf(szOutBuffer, "Write to location %08x caused an access violation.\n", (unsigned int)info-> ExceptionRecord->ExceptionInformation[1]);
		} else {
			sprintf(szOutBuffer, "Read from location %08x caused an access violation.\n", (unsigned int)info-> ExceptionRecord->ExceptionInformation[1]);
		}
	}
	WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
	sprintf(szOutBuffer, "\nStack trace:\n");
	WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
	PrintStackTrace(Context, hLogFile, szOutBuffer);
	sprintf(szOutBuffer, "\nGLE file: '%s'\n", QGLEScriptNameBackup);
	WriteFile(hLogFile, szOutBuffer, strlen(szOutBuffer), &NumBytes, 0);
	CloseHandle(hLogFile);
	// Restart in crash recover mode
	TCHAR szModuleName[MAX_PATH+1];
	DWORD dwRes = GetModuleFileName(NULL, szModuleName, MAX_PATH);
	if (dwRes == 0) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	TCHAR szCommandLine[2*MAX_PATH];
	lstrcpy(szCommandLine, szModuleName);
	lstrcat(szCommandLine, " -crashrecover \"");
	lstrcat(szCommandLine, szTempName);
	lstrcat(szCommandLine, "\"");
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	if (CreateProcess(NULL,	szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		// suppress the standard crash dialog
		exit(-1);
	} else {
		// the standard crash dialog appear
		return EXCEPTION_CONTINUE_SEARCH;
	}
}
#endif

int main(int argc, char *argv[]) {
	#ifdef Q_OS_WIN32
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
	#endif
	#ifdef Q_OS_LINUX
	Q_IMPORT_PLUGIN(QWaylandIntegrationPlugin)
	#endif
	QLocale curLocale("C");
	QLocale::setDefault(curLocale);
	QApplication app(argc,argv);
	Q_INIT_RESOURCE(qgle);
	// setlocale is required to make sure that GLE uses decimal a point instead of a comma
	// this is very important, otherwise its PostScript output may contain commas in floating point numbers
	// the call to setlocale should come after the initialization of QApplication
	setlocale(LC_NUMERIC, "C");
	for(int i = 0; i < argc; i++) {
		if (QString(argv[i]) == "-finddeps") {
			findDependencies(argc, argv, i);
			return 0;
		}
		if (QString(argv[i]) == "-crashrecover" && i+1 < argc) {
			doQGLECrashRecover(QString(argv[i+1]));
			return 0;
		}
	}
	SetQGLEScriptNameBackup(QString("?"));
	#ifdef Q_OS_WIN32
		SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	#endif
	GLEMainWindow mainWin(argc,argv);
	mainWin.show();
	setlocale(LC_NUMERIC, "C");
	return app.exec();
}
