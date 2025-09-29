//
// Created by Tofu on 25-6-24.
//
#include "winapiutil.h"
#include <vector>

uintptr_t getProcAddressExBuffered(HANDLE hProcess, uintptr_t moduleBase, const char* symbolName) {
    SIZE_T bytesRead;
    IMAGE_DOS_HEADER dosHeader = {};
    if (!ReadProcessMemory(hProcess, (LPCVOID)moduleBase, &dosHeader, sizeof(dosHeader), &bytesRead))
        return 0;
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
        return 0;

    IMAGE_NT_HEADERS ntHeaders = {};
    uintptr_t ntHeaderAddr = moduleBase + dosHeader.e_lfanew;
    if (!ReadProcessMemory(hProcess, (LPCVOID)ntHeaderAddr, &ntHeaders, sizeof(ntHeaders), &bytesRead))
        return 0;
    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE)
        return 0;

    auto& expData = ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (expData.VirtualAddress == 0 || expData.Size == 0)
        return 0;

    std::vector<uint8_t> buffer(expData.Size);
    uintptr_t exportBase = moduleBase + expData.VirtualAddress;
    if (!ReadProcessMemory(hProcess, (LPCVOID)exportBase, buffer.data(), buffer.size(), &bytesRead))
        return 0;

    auto* exportDir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(buffer.data());
    DWORD* nameRVAs = reinterpret_cast<DWORD*>(buffer.data() + (exportDir->AddressOfNames - expData.VirtualAddress));
    WORD* ordinals = reinterpret_cast<WORD*>(buffer.data() + (exportDir->AddressOfNameOrdinals - expData.VirtualAddress));
    DWORD* functions = reinterpret_cast<DWORD*>(buffer.data() + (exportDir->AddressOfFunctions - expData.VirtualAddress));

    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i) {
        const char* name = reinterpret_cast<const char*>(buffer.data() + (nameRVAs[i] - expData.VirtualAddress));
        if (strcmp(name, symbolName) == 0) {
            WORD ordinal = ordinals[i];
            DWORD funcRVA = functions[ordinal];
            return moduleBase + funcRVA;
        }
    }

    return 0;
}

uintptr_t getProcAddressEx(HANDLE hProcess, uintptr_t moduleBase, const char *symbolName) {
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

bool ModifyMappedData(const std::string &filePath, uintptr_t targetAddress, uintptr_t newValue) {
    // 打开目标文件
    HANDLE fileHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "无法打开文件: " << filePath << ", 错误代码: " << GetLastError() << std::endl;
        return false;
    }

    // 获取文件大小
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(fileHandle, &fileSize)) {
        std::cerr << "无法获取文件大小，错误代码: " << GetLastError() << std::endl;
        CloseHandle(fileHandle);
        return false;
    }

    // 将文件映射到内存
    HANDLE mapHandle = CreateFileMappingA(fileHandle, nullptr, PAGE_READWRITE, 0, 0, nullptr);
    if (!mapHandle) {
        std::cerr << "无法创建文件映射，错误代码: " << GetLastError() << std::endl;
        CloseHandle(fileHandle);
        return false;
    }

    auto* fileData = (uint8_t*)MapViewOfFile(mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!fileData) {
        std::cerr << "无法映射文件，错误代码: " << GetLastError() << std::endl;
        CloseHandle(mapHandle);
        CloseHandle(fileHandle);
        return false;
    }

    // 遍历文件的静态区域 (.data / .rdata) 等段
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)fileData;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "无效的 DOS 头" << std::endl;
        UnmapViewOfFile(fileData);
        CloseHandle(mapHandle);
        CloseHandle(fileHandle);
        return false;
    }

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(fileData + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "无效的 NT 头" << std::endl;
        UnmapViewOfFile(fileData);
        CloseHandle(mapHandle);
        CloseHandle(fileHandle);
        return false;
    }

    // 获取节表头
    PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
    bool found = false;

    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i, ++sectionHeader) {
        std::string sectionName(reinterpret_cast<char*>(sectionHeader->Name), IMAGE_SIZEOF_SHORT_NAME);

        // 检查是否是 .data 或 .rdata 段
        if (sectionName == ".data" || sectionName == ".rdata") {
            uintptr_t sectionStart = (uintptr_t)(fileData + sectionHeader->PointerToRawData);
            uintptr_t sectionEnd = sectionStart + sectionHeader->SizeOfRawData;

            uintptr_t addr = sectionStart;
            // 搜索目标地址内容
            for (; addr < sectionEnd; addr += sizeof(uintptr_t)) {
                uintptr_t value = *(uintptr_t*)addr;
                if (value == targetAddress) {  // 匹配地址数据块
                    std::cout << "找到目标地址: " << std::hex << addr << std::endl;
                    *(uintptr_t*)addr = newValue; // 替换为新的值
                    found = true;
                    break;
                }
            }
        }
        if (found) break;
        else  std::cout<<"没能找到目标地址";
    }

    // 保存修改并清理
    UnmapViewOfFile(fileData);
    CloseHandle(mapHandle);
    CloseHandle(fileHandle);

    return found;
}

std::wstring PrintProcessGroups() {
    std::wstring ret;

    HANDLE tokenHandle;
    DWORD tokenInfoLength = 0;
    PTOKEN_GROUPS tokenGroups;

    // 打开当前进程的访问令牌
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle)) {
        return L"无法打开进程访问令牌，错误代码：" + std::to_wstring(GetLastError());
    }

    // 获取组信息的大小
    GetTokenInformation(tokenHandle, TokenGroups, nullptr, 0, &tokenInfoLength);
    tokenGroups = (PTOKEN_GROUPS)malloc(tokenInfoLength);

    // 获取完整的组信息
    if (GetTokenInformation(tokenHandle, TokenGroups, tokenGroups, tokenInfoLength, &tokenInfoLength)) {
        for (DWORD i = 0; i < tokenGroups->GroupCount; i++) {
            SID_NAME_USE sidType;
            TCHAR name[256];
            TCHAR domain[256];
            DWORD nameLen = sizeof(name) / sizeof(TCHAR);
            DWORD domainLen = sizeof(domain) / sizeof(TCHAR);

            // 将 SID 转换为组名
            if (LookupAccountSid(nullptr, tokenGroups->Groups[i].Sid, name, &nameLen, domain, &domainLen, &sidType)) {
                std::wstring groupType = (tokenGroups->Groups[i].Attributes & SE_GROUP_ENABLED) ? L"(启用)" : L"(禁用)";
                ret = std::wstring(L"组名称：") + domain + L"\\" + name + groupType;
            } else {
                ret = L"无法解析 SID 为组名，SID：" + std::to_wstring(uintptr_t (tokenGroups->Groups[i].Sid));
            }
        }
    } else {
        ret = L"获取组信息失败，错误代码：" + std::to_wstring(GetLastError());
    }

    // 清理
    free(tokenGroups);
    CloseHandle(tokenHandle);

    return ret;
}

