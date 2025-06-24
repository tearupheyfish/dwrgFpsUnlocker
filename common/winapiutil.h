//
// Created by Tofu on 25-6-24.
//

#ifndef DWRGFPSUNLOCKER_WINAPIUTIL_H
#define DWRGFPSUNLOCKER_WINAPIUTIL_H

#include <windows.h>
#include <TlHelp32.h>

#include <string>
#include <iostream>

inline std::wstring ConvertToWString(const char* str) {
    if (!str) return L"";

    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    if (len == 0) return L""; // 转换失败情况

    std::wstring wstr(len - 1, 0); // 不需要包含终止符
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &wstr[0], len);
    return wstr;
}


inline uintptr_t GetModuleBaseAddress(DWORD processId, const char* moduleName) {
    uintptr_t moduleBaseAddress = 0;
    std::wstring wmoduleName = ConvertToWString(moduleName);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32W);/** 2025.6.19 bugfix log：原来用MODULEENTRY32W填入的size得是MODULEENTRY32W 不兑，这不是理所当然的吗？*/

        if (Module32FirstW(hSnapshot, &moduleEntry)) {
            do {
//#ifdef _DEBUG
//                std::cerr<<moduleEntry.szModule<<'\t';
//#endif
                if (
                        wmoduleName == moduleEntry.szModule
                        ) {
                    moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
                    break;
                }
            } while (Module32NextW(hSnapshot, &moduleEntry));
        }
        else{
            DWORD error = GetLastError();
            std::cerr << "Module32FirstW failed with error: " << error << std::endl;
        }
        CloseHandle(hSnapshot);
    }
    return moduleBaseAddress;
}

inline uintptr_t GetProcAddressEx(HANDLE hProcess, uintptr_t moduleBase, const char* symbolName) {
    // 读取 DOS 头 (IMAGE_DOS_HEADER)
    IMAGE_DOS_HEADER dosHeader = { 0 };
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, (LPCVOID)moduleBase, &dosHeader, sizeof(IMAGE_DOS_HEADER), &bytesRead) || bytesRead != sizeof(IMAGE_DOS_HEADER)) {
        return 0;
    }

    // 验证 DOS 头
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        return 0;
    }

    // 读取 NT 头 (IMAGE_NT_HEADERS)
    IMAGE_NT_HEADERS ntHeaders = { 0 };
    uintptr_t ntHeaderAddr = moduleBase + dosHeader.e_lfanew;
    if (!ReadProcessMemory(hProcess, (LPCVOID)ntHeaderAddr, &ntHeaders, sizeof(IMAGE_NT_HEADERS), &bytesRead) || bytesRead != sizeof(IMAGE_NT_HEADERS)) {
        return 0;
    }

    // 验证 NT 头
    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
        return 0;
    }

    // 获取导出表的 RVA
    IMAGE_DATA_DIRECTORY exportDirectoryData = ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (exportDirectoryData.Size == 0) {
        return 0; // 没有导出表
    }

    // 读取导出表
    IMAGE_EXPORT_DIRECTORY exportDirectory = { 0 };
    uintptr_t exportTableAddr = moduleBase + exportDirectoryData.VirtualAddress;
    if (!ReadProcessMemory(hProcess, (LPCVOID)exportTableAddr, &exportDirectory, sizeof(IMAGE_EXPORT_DIRECTORY), &bytesRead) || bytesRead != sizeof(IMAGE_EXPORT_DIRECTORY)) {
        return 0;
    }

    // 遍历导出函数名称表
    DWORD nameCount = exportDirectory.NumberOfNames;
    DWORD nameTableRVA = exportDirectory.AddressOfNames;
    DWORD funcTableRVA = exportDirectory.AddressOfFunctions;
    DWORD nameOrdinalTableRVA = exportDirectory.AddressOfNameOrdinals;

    for (DWORD i = 0; i < nameCount; ++i) {
        // 读取导出名称的 RVA
        DWORD nameRVA = 0;
        if (!ReadProcessMemory(hProcess, (LPCVOID)(moduleBase + nameTableRVA + i * sizeof(DWORD)), &nameRVA, sizeof(DWORD), &bytesRead) || bytesRead != sizeof(DWORD)) {
            continue;
        }

        // 读取符号名称
        char symbolBuffer[256] = { 0 };
        if (!ReadProcessMemory(hProcess, (LPCVOID)(moduleBase + nameRVA), symbolBuffer, sizeof(symbolBuffer), &bytesRead)) {
            continue;
        }

        // 比较导出符号名称
        if (std::string(symbolBuffer) == symbolName) {
            // 符号匹配，获取对应函数地址的 RVA
            WORD ordinalIndex = 0;
            if (!ReadProcessMemory(hProcess, (LPCVOID)(moduleBase + nameOrdinalTableRVA + i * sizeof(WORD)), &ordinalIndex, sizeof(WORD), &bytesRead) || bytesRead != sizeof(WORD)) {
                return 0;
            }

            DWORD funcRVA = 0;
            if (!ReadProcessMemory(hProcess, (LPCVOID)(moduleBase + funcTableRVA + ordinalIndex * sizeof(DWORD)), &funcRVA, sizeof(DWORD), &bytesRead) || bytesRead != sizeof(DWORD)) {
                return 0;
            }

            // 计算函数的绝对地址
            return moduleBase + funcRVA;
        }
    }

    return 0; // 找不到符号
}

#endif //DWRGFPSUNLOCKER_WINAPIUTIL_H
