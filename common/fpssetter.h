#ifndef FPS_SETTER_H
#define FPS_SETTER_H

#include <windows.h>
#include <TlHelp32.h>

#include <iostream>
#include <memory>
#include <functional>

class autoxTimerProxy;

class FpsSetter
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
    uintptr_t   funcaddr = 0x7ffad6277700;
    uintptr_t   dyrcx;
//    const char  func_name[256] = "PyOS_ReadlineFunctionPointer";
#define DYRCX_P_OFFSET   funcaddr + 0xA758
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