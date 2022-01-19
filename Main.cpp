#undef UNICODE
#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include <inttypes.h>
#include <process.h>
#include <wingdi.h>
#include "TBuffer.h"

#pragma warning (disable:4996) // что бы е появлялась ошибка 
// This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS.

BOOL CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);


bool Terminate = false;

HANDLE handle;


HWND hMainWnd = 0;

static TBuffer Buffer = TBuffer();
//https://www.informit.com/articles/article.aspx?p=328647&seqNum=3

LRESULT CALLBACK GraphWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rect;

		int midProcess = Buffer.GetMidProcess();
		int maxProcess = Buffer.GetMaxProcess();
		char midStr[50];
		char maxStr[50];

		sprintf(midStr, "%.2d%%", midProcess);
		sprintf(maxStr, "%.2d%%", maxProcess);

		SetDlgItemText(hMainWnd, IDC_MID, midStr);
		SetDlgItemText(hMainWnd, IDC_MAX, maxStr);


		GetClientRect(hWnd, &rect);
		Buffer.Draw(hdc, rect);
		EndPaint(hWnd, &ps);
		ReleaseDC(hWnd, hdc);

		return 0; // ziņojums ir apstrādāts

	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = GraphWndProc;
	wc.lpszClassName = "GRAPH";
	RegisterClass(&wc);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, (DLGPROC)MainWndProc);
	return 0;
}



BOOL RunNotepad(HWND hWnd) { // создаём процесс
	char processName[50]; // идёт вместо "Notepad.exe"
	int processNameLength = sizeof(processName);
	if (!GetDlgItemText(hWnd, IDC_COMMANDLINE, processName, processNameLength)) // читает что мы написали в строку и открывает прогу
		return 0;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si)); // избегаем любых нежелательных эффектов
	si.cb = sizeof(si);


	if (!CreateProcess(
		NULL,
		processName, // командная строка
		NULL,
		NULL,
		FALSE,
		0,   // флажки создание
		NULL,
		NULL,
		&si, // информация предустановки
		&pi) // информация процесса
		)return 0;

	if (handle != 0) CloseHandle(handle);
	CloseHandle(pi.hThread);
	handle = pi.hProcess;
	return 1; // return true

}

DWORD FileTimeToMilliseconds(FILETIME ft)
{
	ULARGE_INTEGER i;
	i.LowPart = ft.dwLowDateTime;
	i.HighPart = ft.dwHighDateTime;
	return i.QuadPart / 10000;
}


DWORD WINAPI ProcessThread(LPVOID lpParameter) {

	char filename[MAX_PATH] = "";

	GetDlgItemText(hMainWnd, IDC_COMMANDLINE, filename, MAX_PATH);
	Terminate = false;
	Buffer.Clear();

	if (RunNotepad(hMainWnd)) {

		SetTimer(hMainWnd, NULL, 125, NULL);

		EnableWindow(GetDlgItem(hMainWnd, IDC_START), FALSE);
		EnableWindow(GetDlgItem(hMainWnd, IDC_TERMINATE), TRUE);


		FILETIME creation_time, exit_time, kernel_time, kernel_time1, user_time, user_time1;

		while (!Terminate) {
			GetProcessTimes(handle, &creation_time, &exit_time, &kernel_time, &user_time);

			DWORD tu1 = FileTimeToMilliseconds(user_time);
			DWORD tk1 = FileTimeToMilliseconds(kernel_time);
			DWORD tr1 = GetTickCount();

			Sleep(100);

			GetProcessTimes(handle, &creation_time, &exit_time, &kernel_time1, &user_time1);

			DWORD tu2 = FileTimeToMilliseconds(user_time1);
			DWORD tk2 = FileTimeToMilliseconds(kernel_time1);
			DWORD tr2 = GetTickCount();

			int tu = 100 * (tu2 - tu1) / (tr2 - tr1);
			int tk = 100 * (tk2 - tk1) / (tr2 - tr1);

			Buffer.Put(TBufItem{ tu,tk });

			DWORD exitCode;
			GetExitCodeProcess(handle, &exitCode);

			if (exitCode == NULL) {
				Terminate = true;
				DWORD exitCode = 0;
				if (handle != 0) {
					Terminate = true;
					TerminateProcess(handle, exitCode);
					EnableWindow(GetDlgItem(hMainWnd, IDC_START), FALSE);
					CloseHandle(handle);

				}

				break;
			}

		}

		KillTimer(hMainWnd, NULL);

	}

	return 0;
}



bool BrowseFileName(HWND Wnd, char* FileName) {
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn); // определяет строку в БАЙТАХ !!! 
	ofn.hwndOwner = Wnd;
	ofn.lpstrFilter = "Executable Files (*.exe)\0*.exe\0" // информационная строка, которая описывает фильтр
		"All Files(*.*)\0 * .*\0"; //  определяет модель фильтра
	ofn.lpstrFile = FileName; // Указатель на буфер, который содержит имя файла
	ofn.nMaxFile = MAX_PATH; // Размер буфера
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "exe";

	return GetOpenFileName(&ofn); // возращаем диалоговое окно в котором находится открываемый диск, имя файла...
}



BOOL CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

	switch (Msg) {
	case WM_INITDIALOG:
		hMainWnd = hWnd;
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			DestroyWindow(hWnd);
			return TRUE;
		case IDC_BROWSE:
		{ // чтобы не пропускала этот case так как мы создаём переменную
			char filename[MAX_PATH] = "notepad.exe";
			if (BrowseFileName(hWnd, filename)) {
				SetDlgItemText(hWnd, IDC_COMMANDLINE, filename);
			}
		}
			return TRUE;
		case IDC_START:
			CloseHandle(CreateThread(NULL, 0, ProcessThread, NULL, 0, NULL));
			return TRUE;
		case IDC_TERMINATE:
		{
			DWORD exit = 0;
			if (handle != 0) {
				Terminate = true;
				TerminateProcess(handle, exit);
				EnableWindow(GetDlgItem(hMainWnd, IDC_TERMINATE), TRUE);
				CloseHandle(handle);

			}
		}
			return TRUE;
		}
		return FALSE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	case WM_TIMER:
		InvalidateRect(GetDlgItem(hWnd, IDC_GRAPH), NULL, FALSE);
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return TRUE;



	}
	return FALSE;
}