#include "unicodeText.h"
#include <wchar.h>
#include <stdio.h>

template <typename T>
bool expectEqual(wchar_t *msg, T v1, T v2)
{
  if (!(v1 == v2))
  {
    wprintf(L"    Failed %s.\n", msg);
    return false;
  }
  return true;
}

void test(wchar_t *path,
          Encoding::Value expectedEncoding,
          size_t expectedOffset)
{
  wprintf(L"Testing \"%s\":\n", path);

  const int BufferSize = 1000;
  char buffer[BufferSize];

  FILE *fp;
  errno_t err = _wfopen_s(&fp, path, L"rb");
  size_t bytesRead = fread(buffer, sizeof(char), BufferSize - 1, fp);
  fclose(fp);
  
  size_t offsetBytes = 666;
  Encoding::Value enc = getEncoding(buffer, bytesRead, &offsetBytes);
  if (!expectEqual(L"encoding detection", expectedEncoding, enc))
    return;
  if (!expectEqual(L"offset detection", expectedOffset, offsetBytes))
    return;

  wchar_t decodedBuffer[BufferSize];
  size_t x = decode(buffer + offsetBytes, bytesRead - offsetBytes,
    enc, decodedBuffer, BufferSize);

  const wchar_t *ReferenceText = L"#!Lorem Ipsum Dolor Sit Amet.";
  if (!expectEqual(L"text compare", 0, wmemcmp(decodedBuffer, ReferenceText, wcslen(ReferenceText))))
    return;

  wprintf(L"    Success\n");
}

int wmain(int argc, wchar_t *argv[])
{
  if (argc < 2)
    SetCurrentDirectory(L"../testdata");
  else
    SetCurrentDirectory(argv[1]);

  test(L"bom\\utf8", Encoding::Utf8, 3);
  test(L"bom\\utf16le", Encoding::Utf16le, 2);
  test(L"bom\\utf16be", Encoding::Utf16be, 2);
  test(L"nonbom\\utf8", Encoding::Utf8, 0);
  test(L"nonbom\\ansi", Encoding::Utf8, 0);
  test(L"nonbom\\utf16le", Encoding::Utf16le, 0);
  test(L"nonbom\\utf16be", Encoding::Utf16be, 0);
}
