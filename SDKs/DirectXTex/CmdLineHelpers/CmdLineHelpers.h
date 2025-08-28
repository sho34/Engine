//--------------------------------------------------------------------------------------
// File: CmdLineHelpers.h
//
// Command-line tool shared functions
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------

#pragma once

#if __cplusplus < 201703L
//#error Requires C++17 (and /Zc:__cplusplus with MSVC)
#endif

#include <algorithm>
#include <cstdio>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <set>
#include <string>

#ifndef TOOL_VERSION
#error Define TOOL_VERSION before including this header
#endif
#include <dxgiformat.h>
#include <handleapi.h>
#include <cassert>

namespace Helpers
{
    struct handle_closer { void operator()(HANDLE h) { if (h) CloseHandle(h); } };

    using ScopedHandle = std::unique_ptr<void, handle_closer>;

    inline HANDLE safe_handle(HANDLE h) noexcept { return (h == INVALID_HANDLE_VALUE) ? nullptr : h; }

    struct find_closer { void operator()(HANDLE h) noexcept { assert(h != INVALID_HANDLE_VALUE); if (h) FindClose(h); } };

    using ScopedFindHandle = std::unique_ptr<void, find_closer>;

#ifdef _PREFAST_
#pragma prefast(disable : 26018, "Only used with static internal arrays")
#endif

    struct SConversion
    {
        std::wstring szSrc;
        std::wstring szFolder;
    };

    template<typename T>
    struct SValue
    {
        const wchar_t*  name;
        T               value;
    };

    template<typename T>
    T LookupByName(const wchar_t _In_z_ *pName, const SValue<T> *pArray)
    {
        while (pArray->name)
        {
            if (!_wcsicmp(pName, pArray->name))
                return pArray->value;

            pArray++;
        }

        return static_cast<T>(0);
    }

    template<typename T>
    const wchar_t* LookupByValue(T value, const SValue<T> *pArray)
    {
        while (pArray->name)
        {
            if (value == pArray->value)
                return pArray->name;

            pArray++;
        }

        return L"";
    }

    void PrintFormat(DXGI_FORMAT Format, const SValue<DXGI_FORMAT>* pFormatList);

    void PrintFormat(DXGI_FORMAT Format, const SValue<DXGI_FORMAT>* pFormatList1, const SValue<DXGI_FORMAT>* pFormatList2);

    template<typename T>
    void PrintList(size_t cch, const SValue<T> *pValue)
    {
        while (pValue->name)
        {
            const size_t cchName = wcslen(pValue->name);

            if (cch + cchName + 2 >= 80)
            {
                wprintf(L"\n      ");
                cch = 6;
            }

            wprintf(L"%ls ", pValue->name);
            cch += cchName + 2;
            pValue++;
        }

        wprintf(L"\n");
    }

    void PrintLogo(bool versionOnly, _In_z_ const wchar_t* name, _In_z_ const wchar_t* desc);

    void SearchForFiles(const std::filesystem::path& path, std::list<SConversion>& files, bool recursive, _In_opt_z_ const wchar_t* folder);

    void ProcessFileList(std::wifstream& inFile, std::list<SConversion>& files);

    const wchar_t* GetErrorDesc(HRESULT hr);
}
