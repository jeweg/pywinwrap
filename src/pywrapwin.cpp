#ifndef UNICODE
#  define UNICODE
#endif
#ifndef _UNICODE
#  define _UNICODE
#endif
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#pragma comment(lib, "Shlwapi.lib")

#include "unicodeText.h"
#include <windows.h>
#include <Shlwapi.h>
#include <stdio.h>

const wchar_t* PythonScriptSuffix = L"-script.py";

const int ErrorGetModuleFileName     = 10000;
const int ErrorCreateProcess         = 10001;
const int ErrorSetConsoleCtrlHandler = 10002;
const int ErrorGetExitCodeProcess    = 10003;

BOOL WINAPI interruptHandler(DWORD code)
{
  return TRUE;
}

void error(int errorCode, const wchar_t *msg)
{
  DWORD lastError = GetLastError();
  if (lastError != ERROR_SUCCESS)
  {
    const size_t MsgBufferMaxSize = 500;
    wchar_t errMsgBuffer[MsgBufferMaxSize];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, lastError, 0, (LPTSTR)&errMsgBuffer, MsgBufferMaxSize, NULL);
    wprintf(L"Error [%d] %s: %s", errorCode, msg, errMsgBuffer);
  }
  else
    wprintf(L"Error [%d] %s\n", errorCode, msg);
  ExitProcess(errorCode);
}


wchar_t *skipPastThis(wchar_t *commandLine)
{
  BOOL quoted;
  wchar_t c;
  wchar_t *result = commandLine;

  // Find the first whitespace or the 2nd quote if commandLine
  // starts with a quote.
  quoted = commandLine[0] == L'\"';
  if (!quoted)
    c = L' ';
  else {
    c = L'\"';
    ++result;
  }
  result = wcschr(result, c);
  if (result == NULL)
    return L"";
  else
  {
    // Skip past space or closing quote.
    ++result;
    // Skip past whitespace
    while (*result && iswspace(*result))
      ++result;
    return result;
  }
}


int wmain()
{
  // Determine full path to this executable without extension.
  wchar_t myPath[MAX_PATH];
  if (!GetModuleFileName(NULL, myPath, MAX_PATH))
    error(ErrorGetModuleFileName, L"GetModuleFileName failed");
  PathRemoveExtension(myPath);

  // Read the Python file and try get an interpreter path from the shebang.

  wchar_t *pythonExe = L"python.exe";

  size_t len = wcslen(myPath) + wcslen(PythonScriptSuffix) + 1;
  wchar_t *buff = new wchar_t[len];
  buff[0] = '\0';
  wcscat_s(buff, len, myPath);
  wcscat_s(buff, len, PythonScriptSuffix);

  FILE *fp;
  errno_t err = _wfopen_s(&fp, buff, L"rb");
  if (err)
    error(-1, L"Python script");

  const int BufferSize = 500;
  char buffer[BufferSize];
  size_t bytesRead = fread(buffer, 1, BufferSize, fp);
  fclose(fp);
  size_t textBytesOffset;
  Encoding::Value encoding = getEncoding(buffer, bytesRead, &textBytesOffset);

  wchar_t wbuffer[BufferSize];
  size_t wcharCount = decode(buffer + textBytesOffset, bytesRead - textBytesOffset,
    encoding, wbuffer, BufferSize);
  wchar_t *firstLineStart = wbuffer;
  if (wcharCount > 2)
  {
    if (firstLineStart[0] == L'#' && firstLineStart[1] == L'!')
    {
      firstLineStart += 2;

      // Look for line ending.
      wchar_t *firstLineEnd = wcschr(firstLineStart, L'\n');
      if (firstLineEnd)
      {
        if (firstLineEnd > firstLineStart && *(firstLineEnd - 1) == L'\r')
          --firstLineEnd;
        *firstLineEnd = L'\0';
      }
      else
        firstLineEnd = firstLineStart + wcharCount;

      // Got a shebang line. Interpret as path.
      errno_t err = _wfopen_s(&fp, firstLineStart, L"rb");
      fclose(fp);
      if (err == 0)
        pythonExe = firstLineStart;
    }
  }

  // Build a command line.

  // Remove ourselves from the command line.
  wchar_t *arguments = skipPastThis(GetCommandLine());
  size_t commandLineSize = wcslen(pythonExe) +
                           wcslen(myPath) +
                           wcslen(PythonScriptSuffix) +
                           wcslen(arguments) +
                           3; // 2 spaces and terminator.
  
  // No need to delete this because the process will end anyway.
  wchar_t *commandLine = new wchar_t[commandLineSize];
  commandLine[0] = '\0';

  wcscat_s(commandLine, commandLineSize, pythonExe);
  wcscat_s(commandLine, commandLineSize, L" ");
  wcscat_s(commandLine, commandLineSize, myPath);
  wcscat_s(commandLine, commandLineSize, PythonScriptSuffix);
  wcscat_s(commandLine, commandLineSize, L" ");
  wcscat_s(commandLine, commandLineSize, arguments);

  // For the purpose of transparency we disable Ctrl-c
  // keyboard interrupt for this wrapper process,
  // so keyboard interrrupts only occur in the called python.
  BOOL okay = SetConsoleCtrlHandler(interruptHandler, TRUE);
  if (!okay)
     error(ErrorSetConsoleCtrlHandler, L"SetConsoleCtrlHandler failed");

  // Run the command.
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  okay = CreateProcess(NULL, commandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
  if (!okay)
    error(ErrorCreateProcess, L"CreateProcess failed");

  CloseHandle(pi.hThread);
  WaitForSingleObjectEx(pi.hProcess, INFINITE, FALSE);

  DWORD returnCode;
  okay = GetExitCodeProcess(pi.hProcess, &returnCode);
  if (!okay)
    error(ErrorGetExitCodeProcess, L"GetExitCodeProcess failed");

  ExitProcess(returnCode);
}
