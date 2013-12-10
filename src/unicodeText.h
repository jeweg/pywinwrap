#ifndef UNICODE_TEXT_H_INCL
#define UNICODE_TEXT_H_INCL

#include <windows.h>

#if defined(_MSC_VER)
  #include <intrin.h>
  #define SWAP_BYTES(v) _byteswap_ushort(v)
#else
  unsigned short SWAP_BYTES(unsigned short v)
  {
    return ((v & 0xff) << 8) | ((v & 0xff00) >> 8);
  }
#endif

struct Encoding
{
  enum Value
  {
    Unknown,
    Utf8,
    Utf16le,
    Utf16be
  };
};

Encoding::Value getEncoding(const char *textBuffer, size_t textBytes,
  size_t *out_offsetBytes = 0)
{
  if (!memcmp(textBuffer, "\xef\xbb\xbf", 3))
  {
    if (out_offsetBytes)
      *out_offsetBytes = 3;
    return Encoding::Utf8;
  }
  if (!memcmp(textBuffer, "\xff\xfe", 2))
  {
    if (out_offsetBytes)
      *out_offsetBytes = 2;
    return Encoding::Utf16le;
  }
  if (!memcmp(textBuffer, "\xfe\xff", 2))
  {
    if (out_offsetBytes)
      *out_offsetBytes = 2;
    return Encoding::Utf16be;
  }

  if (out_offsetBytes)
    *out_offsetBytes = 0;

  // Try to detect UTF-16 without BOM.
  // We do this before UTF-8 non-BOM detection because that check
  // also recognizes UTF-16 non-BOM.
  // We do this by looking for a #(ansi 23) or a space(ansi 20).
  size_t i = 1;
  while (i < textBytes)
  {
    char b1 = textBuffer[i - 1];
    char b2 = textBuffer[i];
    i += 2;
    if (b1 == 0x23 && b2 == 0x00 || b1 == 0x20 && b2 == 0x00)
      return Encoding::Utf16le;
    else if (b1 == 0x00 && b2 == 0x23 || b1 == 0x00 && b2 == 0x20)
      return Encoding::Utf16be;
  }
  return Encoding::Utf8;
}

size_t decode(const char *textBuffer, size_t textBytes, Encoding::Value srcEncoding, wchar_t *outBuffer, size_t outBufferWChars)
{
  textBytes = textBytes / 2 * 2;

  short tmp = 1;
  bool platformIsBigEndian = *(char*)&tmp != 1;
  
  size_t wcharsWritten = 0;
  if (srcEncoding == Encoding::Utf8)
  {
    size_t wchars = MultiByteToWideChar(CP_UTF8, 0, textBuffer, textBytes,
      outBuffer, outBufferWChars - 1);
    outBuffer[wchars] = L'\0';
    return wchars;
  }
  else if (srcEncoding == Encoding::Utf16be && !platformIsBigEndian ||
            srcEncoding == Encoding::Utf16le && platformIsBigEndian)
  {
    size_t wcharsToWrite = textBytes / 2;
    if (outBufferWChars < wcharsToWrite + 1)
      wcharsToWrite = outBufferWChars - 1;

    const unsigned short *sp = reinterpret_cast<const unsigned short*>(textBuffer);
    unsigned short *dp = reinterpret_cast<unsigned short*>(outBuffer);
    for (size_t i = 0; i < wcharsToWrite; ++i)
      *dp++ = SWAP_BYTES(*sp++);

    *dp = (unsigned short)0;
    return wcharsToWrite;
  }
  else
  {
    memcpy(outBuffer, textBuffer, textBytes);
    outBuffer[textBytes] = L'\0';
    return textBytes / 2;
  }
  return 0;
}

#endif
