/**
 * Copyright 2022.11.4 45degree
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

namespace Marbas {

#ifdef _MSC_VER_  // for MSVC
#define FORCE_INLINE __forceinline
#elif defined __GNUC__  // for gcc on Linux/Apple OS X
#define FORCE_INLINE __inline__ __attribute__((always_inline))
#else
#define FORCE_INLINE
#endif

}  // namespace Marbas
