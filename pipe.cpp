#include <cstring>
#include <windows.h>
#include "base2.h"
#include "pipe.h"

inline void ParseDir(char* szDir, const char* szPath) {
	char* lpSeparator;
	strcpy(szDir, szPath);
	lpSeparator = strrchr(szDir, PATH_SEPARATOR);
	if (lpSeparator == NULL) {
		szDir[0] = '\0';
	}
	else {
		*lpSeparator = '\0';
	}
}

void PipeStruct::Open(const char* szProcFile) {
	DWORD dwMode;
	HANDLE hStdinRead, hStdinWrite, hStdoutRead, hStdoutWrite;
	SECURITY_ATTRIBUTES sa;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char szDir[PATH_MAX_CHAR], szCurDir[PATH_MAX_CHAR];

	nEof = 0;
	if (szProcFile == NULL) {
		hInput = GetStdHandle(STD_INPUT_HANDLE);
		hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		bConsole = GetConsoleMode(hInput, &dwMode);
	}
	else {
		GetCurrentDirectory(PATH_MAX_CHAR, szCurDir);
		ParseDir(szDir, szProcFile);
		SetCurrentDirectory(szDir);

		memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0);
		CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0);
		memset(&si, 0, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = hStdinRead;
		si.hStdOutput = hStdoutWrite;
		si.hStdError = hStdoutWrite;
		if (CreateProcess(NULL, (LPSTR)szProcFile, NULL, NULL, TRUE, DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP | IDLE_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else {
			nEof = 1;
		}
		CloseHandle(hStdinRead);
		CloseHandle(hStdoutWrite);
		hInput = hStdoutRead;
		hOutput = hStdinWrite;
		bConsole = FALSE;

		SetCurrentDirectory(szCurDir);
	}
	if (bConsole) {
		SetConsoleMode(hInput, dwMode & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
		FlushConsoleInputBuffer(hInput);
	}
	nBytesLeft = 0;
	nReadEnd = 0;
}

void PipeStruct::Close(void) const {
	CloseHandle(hInput);
	CloseHandle(hOutput);
}

void PipeStruct::ReadInput(void) {
	DWORD dwBytes;
	if (ReadFile(hInput, szBuffer + nReadEnd, LINE_INPUT_MAX_CHAR - nReadEnd, &dwBytes, NULL)) {
		nReadEnd += dwBytes;
		if (nBytesLeft > 0) {
			nBytesLeft -= dwBytes;
		}
	}
	else {
		nEof = 1;
	}
}

bool PipeStruct::CheckInput(void) {
	DWORD dwEvents, dwBytes;
	if (bConsole) { // a tty, or an un-redirected handle
		GetNumberOfConsoleInputEvents(hInput, &dwEvents);
		return dwEvents > 1;
	}
	else if (nBytesLeft > 0) { // a pipe with remainder data
		return true;
	}
	else if (PeekNamedPipe(hInput, NULL, 0, NULL, &dwBytes, NULL)) { // a pipe without remainder data 
		nBytesLeft = dwBytes;
		return nBytesLeft > 0;
	}
	else { // a file, always true
		return true;
	}
}

void PipeStruct::LineOutput(const char* szLineStr) const {
	DWORD dwBytes;
	int nStrLen;
	char szWriteBuffer[LINE_INPUT_MAX_CHAR];
	nStrLen = strlen(szLineStr);
	memcpy(szWriteBuffer, szLineStr, nStrLen);
	szWriteBuffer[nStrLen] = '\r';
	szWriteBuffer[nStrLen + 1] = '\n';
	WriteFile(hOutput, szWriteBuffer, nStrLen + 2, &dwBytes, NULL);
}

bool PipeStruct::GetBuffer(char* szLineStr) {
	char* lpFeedEnd;
	int nFeedEnd;
	lpFeedEnd = (char*)memchr(szBuffer, '\n', nReadEnd);
	if (lpFeedEnd == NULL) {
		return false;
	}
	else {
		nFeedEnd = lpFeedEnd - szBuffer;
		memcpy(szLineStr, szBuffer, nFeedEnd);
		szLineStr[nFeedEnd] = '\0';
		nFeedEnd++;
		nReadEnd -= nFeedEnd;
		memcpy(szBuffer, szBuffer + nFeedEnd, nReadEnd);
		lpFeedEnd = (char*)strchr(szLineStr, '\r');
		if (lpFeedEnd != NULL) {
			*lpFeedEnd = '\0';
		}
		return true;
	}
}

bool PipeStruct::LineInput(char* szLineStr) {
	if (GetBuffer(szLineStr)) {
		return true;
	}
	else if (CheckInput()) {
		ReadInput();
		if (GetBuffer(szLineStr)) {
			return true;
		}
		else if (nReadEnd == LINE_INPUT_MAX_CHAR) {
			memcpy(szLineStr, szBuffer, LINE_INPUT_MAX_CHAR - 1);
			szLineStr[LINE_INPUT_MAX_CHAR - 1] = '\0';
			szBuffer[0] = szBuffer[LINE_INPUT_MAX_CHAR - 1];
			nReadEnd = 1;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}
