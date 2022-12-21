/**
 * Copyright 2022.12.21 45degree
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
