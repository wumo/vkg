// (‑●‑●)> released under the WTFPL v2 license, by Gregory Pakosz (@gpakosz)

// in case you want to #include "whereami.c" in a larger compilation unit
#if !defined(WHEREAMI_H)
#include "whereami.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(WAI_MALLOC) || !defined(WAI_FREE) || !defined(WAI_REALLOC)
#include <stdlib.h>
#endif

#if !defined(WAI_MALLOC)
#define WAI_MALLOC(size) malloc(size)
#endif

#if !defined(WAI_FREE)
#define WAI_FREE(p) free(p)
#endif

#if !defined(WAI_REALLOC)
#define WAI_REALLOC(p, size) realloc(p, size)
#endif

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#if defined(_MSC_VER)
#pragma warning(push, 3)
#endif
#include <windows.h>
#include <intrin.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#ifndef WAI_NOINLINE
#ifdef _MSC_VER
#define WAI_NOINLINE __declspec(noinline)
#endif
#endif

static int WAI_PREFIX(getModulePath_)(HMODULE module, char* out, int capacity, int* dirname_length)
{
  wchar_t buffer1[MAX_PATH];
  wchar_t buffer2[MAX_PATH];
  wchar_t* path = NULL;
  int length = -1;

  for (;;)
  {
    DWORD size;
    int length_;

    size = GetModuleFileNameW(module, buffer1, sizeof(buffer1) / sizeof(buffer1[0]));

    if (size == 0)
      break;
    else if (size == (DWORD)(sizeof(buffer1) / sizeof(buffer1[0])))
    {
      DWORD size_ = size;
      do
      {
        size_ *= 2;
        path = (wchar_t*)WAI_REALLOC(path, sizeof(wchar_t) * size_);
        size = GetModuleFileNameW(NULL, path, size_);
      } while (size == size_);
    }
    else
      path = buffer1;

    _wfullpath(buffer2, path, MAX_PATH);
    length_ = WideCharToMultiByte(CP_UTF8, 0, buffer2, -1, out, capacity, NULL, NULL);

    if (length_ == 0)
      length_ = WideCharToMultiByte(CP_UTF8, 0, buffer2, -1, NULL, 0, NULL, NULL);
    if (length_ == 0)
      break;

    if (length_ <= capacity && dirname_length)
    {
      int i;

      for (i = length_ - 1; i >= 0; --i)
      {
        if (out[i] == '\\')
        {
          *dirname_length = i;
          break;
        }
      }
    }

    length = length_;

    break;
  }

  if (path != buffer1)
    WAI_FREE(path);

  return length;
}

WAI_NOINLINE
WAI_FUNCSPEC
int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length)
{
  return WAI_PREFIX(getModulePath_)(NULL, out, capacity, dirname_length);
}

WAI_NOINLINE
WAI_FUNCSPEC
int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length)
{
  HMODULE module;
  int length = -1;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4054)
#endif
  if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)_ReturnAddress(), &module))
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
  {
    length = WAI_PREFIX(getModulePath_)(module, out, capacity, dirname_length);
  }

  return length;
}

#elif defined(__linux__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

char *realpath(const char * pathname, char * resolved_path);

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#if !defined(WAI_PROC_SELF_EXE)
#define WAI_PROC_SELF_EXE "/proc/self/exe"
#endif

#ifndef WAI_NOINLINE
#ifdef __GNUC__
#define WAI_NOINLINE __attribute__((noinline))
#endif
#endif

WAI_FUNCSPEC
int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length)
{
  char buffer[PATH_MAX];
  char* resolved = NULL;
  int length = -1;

  for (;;)
  {
    resolved = realpath(WAI_PROC_SELF_EXE, buffer);
    if (!resolved)
      break;

    length = (int)strlen(resolved);
    if (length <= capacity)
    {
      memcpy(out, resolved, length);

      if (dirname_length)
      {
        int i;

        for (i = length - 1; i >= 0; --i)
        {
          if (out[i] == '/')
          {
            *dirname_length = i;
            break;
          }
        }
      }
    }

    break;
  }

  return length;
}

#if !defined(WAI_PROC_SELF_MAPS_RETRY)
#define WAI_PROC_SELF_MAPS_RETRY 5
#endif

#if !defined(WAI_PROC_SELF_MAPS)
#define WAI_PROC_SELF_MAPS "/proc/self/maps"
#endif

WAI_NOINLINE
WAI_FUNCSPEC
int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length)
{
  int length = -1;
  FILE* maps = NULL;
  int i;

  for (i = 0; i < WAI_PROC_SELF_MAPS_RETRY; ++i)
  {
    maps = fopen(WAI_PROC_SELF_MAPS, "r");
    if (!maps)
      break;

    for (;;)
    {
      char buffer[PATH_MAX < 1024 ? 1024 : PATH_MAX];
      uint64_t low, high;
      char perms[5];
      uint64_t offset;
      uint32_t major, minor;
      char path[PATH_MAX];
      uint32_t inode;

      if (!fgets(buffer, sizeof(buffer), maps))
        break;

      if (sscanf(buffer, "%" PRIx64 "-%" PRIx64 " %s %" PRIx64 " %x:%x %u %s\n", &low, &high, perms, &offset, &major, &minor, &inode, path) == 8)
      {
        uint64_t addr = (uint64_t)(uintptr_t) __builtin_extract_return_addr(__builtin_return_address(0));
        if (low <= addr && addr <= high)
        {
          char* resolved;

          resolved = realpath(path, buffer);
          if (!resolved)
            break;

          length = (int)strlen(resolved);
          if (length <= capacity)
          {
            memcpy(out, resolved, length);

            if (dirname_length)
            {
              int i;

              for (i = length - 1; i >= 0; --i)
              {
                if (out[i] == '/')
                {
                  *dirname_length = i;
                  break;
                }
              }
            }
          }

          break;
        }
      }
    }

    fclose(maps);

    if (length != -1)
      break;
  }

  return length;
}

#elif defined(__APPLE__)

#define _DARWIN_BETTER_REALPATH
#include <mach-o/dyld.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#ifndef WAI_NOINLINE
#ifdef __GNUC__
#define WAI_NOINLINE __attribute__((noinline))
#endif
#endif

WAI_FUNCSPEC
int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length)
{
  char buffer1[PATH_MAX];
  char buffer2[PATH_MAX];
  char* path = buffer1;
  char* resolved = NULL;
  int length = -1;

  for (;;)
  {
    uint32_t size = (uint32_t)sizeof(buffer1);
    if (_NSGetExecutablePath(path, &size) == -1)
    {
      path = (char*)WAI_MALLOC(size);
      if (!_NSGetExecutablePath(path, &size))
        break;
    }

    resolved = realpath(path, buffer2);
    if (!resolved)
      break;

    length = (int)strlen(resolved);
    if (length <= capacity)
    {
      memcpy(out, resolved, length);

      if (dirname_length)
      {
        int i;

        for (i = length - 1; i >= 0; --i)
        {
          if (out[i] == '/')
          {
            *dirname_length = i;
            break;
          }
        }
      }
    }

    break;
  }

  if (path != buffer1)
    WAI_FREE(path);

  return length;
}

WAI_NOINLINE
WAI_FUNCSPEC
int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length)
{
  char buffer[PATH_MAX];
  char* resolved = NULL;
  int length = -1;

  for(;;)
  {
    Dl_info info;

    if (dladdr(__builtin_extract_return_addr(__builtin_return_address(0)), &info))
    {
      resolved = realpath(info.dli_fname, buffer);
      if (!resolved)
        break;

      length = (int)strlen(resolved);
      if (length <= capacity)
      {
        memcpy(out, resolved, length);

        if (dirname_length)
        {
          int i;

          for (i = length - 1; i >= 0; --i)
          {
            if (out[i] == '/')
            {
              *dirname_length = i;
              break;
            }
          }
        }
      }
    }

    break;
  }

  return length;
}

#elif defined(__QNXNTO__)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#ifndef WAI_NOINLINE
#ifdef __GNUC__
#define WAI_NOINLINE __attribute__((noinline))
#endif
#endif

#if !defined(WAI_PROC_SELF_EXE)
#define WAI_PROC_SELF_EXE "/proc/self/exefile"
#endif

WAI_FUNCSPEC
int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length)
{
  char buffer1[PATH_MAX];
  char buffer2[PATH_MAX];
  char* resolved = NULL;
  FILE* self_exe = NULL;
  int length = -1;

  for (;;)
  {
    self_exe = fopen(WAI_PROC_SELF_EXE, "r");
    if (!self_exe)
      break;

    if (!fgets(buffer1, sizeof(buffer1), self_exe))
      break;

    resolved = realpath(buffer1, buffer2);
    if (!resolved)
      break;

    length = (int)strlen(resolved);
    if (length <= capacity)
    {
      memcpy(out, resolved, length);

      if (dirname_length)
      {
        int i;

        for (i = length - 1; i >= 0; --i)
        {
          if (out[i] == '/')
          {
            *dirname_length = i;
            break;
          }
        }
      }
    }

    break;
  }

  fclose(self_exe);

  return length;
}

WAI_FUNCSPEC
int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length)
{
  char buffer[PATH_MAX];
  char* resolved = NULL;
  int length = -1;

  for(;;)
  {
    Dl_info info;

    if (dladdr(__builtin_extract_return_addr(__builtin_return_address(0)), &info))
    {
      resolved = realpath(info.dli_fname, buffer);
      if (!resolved)
        break;

      length = (int)strlen(resolved);
      if (length <= capacity)
      {
        memcpy(out, resolved, length);

        if (dirname_length)
        {
          int i;

          for (i = length - 1; i >= 0; --i)
          {
            if (out[i] == '/')
            {
              *dirname_length = i;
              break;
            }
          }
        }
      }
    }

    break;
  }

  return length;
}

#endif

#ifdef __cplusplus
}
#endif
