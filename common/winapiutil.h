//
// Created by Tofu on 25-6-24.
//

#ifndef DWRGFPSUNLOCKER_WINAPIUTIL_H
#define DWRGFPSUNLOCKER_WINAPIUTIL_H

#include <windows.h>
#include <TlHelp32.h>

#include <string>
#include <iostream>
#include <cstdint>

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

uintptr_t getProcAddressEx(HANDLE hProcess, uintptr_t moduleBase, const char* symbolName);
uintptr_t getProcAddressExBuffered(HANDLE hProcess, uintptr_t moduleBase, const char* symbolName);

bool ModifyMappedData(const std::string& filePath, uintptr_t targetAddress, uintptr_t newValue);


#endif //DWRGFPSUNLOCKER_WINAPIUTIL_H
