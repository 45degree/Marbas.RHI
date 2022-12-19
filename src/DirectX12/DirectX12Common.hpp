#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <winerror.h>

#include <cwchar>
#include <exception>
#include <iostream>
#include <string>

namespace Marbas {

inline void
ThrowIfFailed(HRESULT hr) {
  if (FAILED(hr)) {
    // Set a breakpoint on this line to catch DirectX API errors
    throw std::exception();
  }
}

}  // namespace Marbas
   //

inline std::ostream&
operator<<(std::ostream& out, const wchar_t* str) {
  size_t len = wcsrtombs(nullptr, &str, 0, nullptr);
  char* buf = static_cast<char*>(malloc(len + 1));
  buf[len] = 0;
  wcsrtombs(buf, &str, len, nullptr);
  out << buf;
  free(buf);
  return out;
}

inline std::ostream&
operator<<(std::ostream& out, const std::wstring& str) {
  return operator<<(out, str.c_str());
}
