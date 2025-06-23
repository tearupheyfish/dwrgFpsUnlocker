#ifndef FPS_SETTER_H
#define FPS_SETTER_H

#include <windows.h>
#include <TlHelp32.h>

#include <iostream>
#include <memory>
#include <functional>

class autoxTimerProxy;

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


class FpsSetter /*它目前是个抽象类*/
{
protected:
#define JMPLENTH 5
    // static const char targetWindowTitle[] = "第五人格";
    static constexpr char targetModuleName[] = "neox_engine.dll";
    /*static constexpr uintptr_t offset = 0x2525E0;
    static inline BYTE jumpInstruction[10] = {
        0xE9, 0x00, 0x00, 0x00, 0x00, //jmp <newMemoryAddress>
        0x90, 0x90, 0x90, 0x90, 0x90
    };
#define DFLT_FPS 60 //NOTE: 0-255, cuz 1 byte
#define FPS_OFFSET 3
#define ORIG_OFFSET 7
    static inline BYTE injectedCode[] = {
        0xC7, 0x42, 0x04, DFLT_FPS, 0x00, 0x00, 0x00, // mov dword ptr [rdx+4], 78
        0x0F, 0x10, 0x02,                         // movups xmm0, [rdx]
        0x0F, 0x11, 0x81, 0x88, 0x00, 0x00, 0x00, // movups [rcx+88h], xmm0
        0xE9, 0x00, 0x00, 0x00, 0x00
    };*/

    static inline std::shared_ptr<DWORD> allocrice = std::make_shared<DWORD>(0);

    bool        bad;    //Setter是否可用
    bool        fpsbad; //fps是否可读取
    // bool        autox;  //是否自动关闭句柄
    DWORD       processID;
    HANDLE      processHandle;
    uintptr_t   moduleBase;
//    const char  func_name[256] = "PyOS_ReadlineFunctionPointer";
    uintptr_t   dyrcx;
#define DYRCX_P_OFFSET   0x7FF9BB257700 - \
                         0x7FF9B50C0000 + 0xA758
#define DYRCX_O_OFFSET   0x10

    uintptr_t   preframerateaddr;
#define PFR_OFFSET 0x8  //√
#define FR_OFFSET  0x23C //√

#define FRT_OFFSET 0x68 //√

    friend class autoxTimerProxy;
    autoxTimerProxy* autoxprocesstimer;

public:
    FpsSetter(DWORD pid);

    ~FpsSetter();

    bool getAddress();

    explicit
    operator bool()const{return bad;}

    bool setFps(int fps);

    bool checkHandle();

    float getFps();

    bool openHandle();

    void closeHandle()
    {
        if(processHandle){
            CloseHandle(processHandle);
            processHandle = nullptr;
        }
    }

    DWORD getprocessID()const
    {
        return processID;
    }

    void pauseautox();
    void continueautox();

private:
    LPVOID findnearfreememory(uintptr_t target_vmemaddr, size_t size)
    {
        LPVOID allocaddr = nullptr;

        static const uint32_t max_offset = (1LL<<32)/ *allocrice;
        uint64_t  l(target_vmemaddr/ *allocrice), r(target_vmemaddr/ *allocrice+max_offset);
        /*do{
            int64_t mid = (l&r) + ((l^r)>>1);
            MEMORY_BASIC_INFORMATION pmbi;
            auto retsize = VirtualQueryEx(*processHandle, (LPVOID)(mid**allocrice), &pmbi, sizeof(pmbi));
            if (!(retsize == 0 || pmbi.State != MEM_FREE))
            {
                allocaddr = VirtualAllocEx(*processHandle, (LPVOID)(mid**allocrice), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                if(allocaddr)
                    return allocaddr;
            }
            r = mid;
        }while (r != l);*/

        uint32_t granularity = max_offset;
        MEMORY_BASIC_INFORMATION mbi;
        while (granularity)
        {
            for (uint64_t i = l+granularity/2; i < r; i+=granularity)
            {
                auto retsize = VirtualQueryEx(processHandle, (LPVOID)(i**allocrice), &mbi, sizeof(mbi));
                if (!(retsize == 0 || mbi.State != MEM_FREE))
                {
                    allocaddr = VirtualAllocEx(processHandle, (LPVOID)(i**allocrice), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                    if(allocaddr)
                        return allocaddr;
                }
            }
            granularity /=2;
        }
        return nullptr;
    }
    // writeMem();

    friend class autoxTimerProxy;
    friend bool getAddressImpl(FpsSetter&);

};

#endif // FPS_SETTER_H