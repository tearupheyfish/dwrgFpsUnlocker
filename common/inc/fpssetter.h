#ifndef FPS_SETTER_H
#define FPS_SETTER_H

#include <windows.h>
#include <TlHelp32.h>

#include <iostream>
#include <memory>
#include <functional>

/** 地址布局（截至2025.7.2）（'[]'表解引用）
 *              ↓ funcaddr
 * [ [PyOS_ReadlineFunctionPointer+0xA758]
 *            +0x10] ← dyrcx (一段动态内存地址；这样命名只是因为跟踪时用的寄存器几乎都是rcx)
 * [dyrcx+08] + 0x23C  ← 游戏自带检测的帧率地址
 *      ⤒ preframeaddr
 */
class FpsSetter
{
protected:
    /* 最初写rcx+80的版本，麻烦IDE折叠一下
#define JMPLENTH 5
    static constexpr uintptr_t offset = 0x2525E0;
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

//  static inline DWORD allocrice = 0;

    bool        bad;        //Setter是否可用
    bool        fpsbad;     //fps是否可读取
//  bool        autox;      //是否自动关闭句柄
    DWORD       processID;
    HANDLE      processHandle;
    uintptr_t   moduleBase;
    uintptr_t   funcaddr;
    uintptr_t   dyrcx;
#define DYRCX_P_OFFSET  (funcaddr + 0xA75)
#define DYRCX_O_OFFSET   0x10

    uintptr_t   preframerateaddr;
#define PFR_OFFSET       0x8
#define FR_OFFSET        0x23C

#define FRT_OFFSET       0x68

    friend class autoxtimerproxy;
    autoxtimerproxy* autoxprocesstimer;

public:
    /* 禁止拷贝的三-五，
     * 原因：只是因为初始化需要的进程id没写在这里 */ //question: 什么意思？
    FpsSetter(DWORD pid);
    FpsSetter():bad(true), fpsbad(true), processHandle(NULL), autoxprocesstimer(nullptr){};

    FpsSetter(const FpsSetter&) = delete;
    FpsSetter& operator=(const FpsSetter&) = delete;

    FpsSetter(FpsSetter&& right) noexcept;
    FpsSetter& operator=(FpsSetter&& right) noexcept;

    ~FpsSetter();

explicit
    operator bool()const{return !bad;}

    float getFps();
    bool setFps(int fps);

    //autox: 自动脱离(停止访问并释放句柄)
    void pauseAutoX();
    void continueAutoX();

protected:
    //获取内存地址。但因为反向依赖不知道怎么 todo: 导出为dll
    bool getAddress();
    //打开或检查访问进程的句柄有效，否则返回false
    bool openHandle();
    void closeHandle();
    bool checkGameLiving();

      /* 最初需要的分配jmp的就近x86地址的函数。麻烦IDE折叠一下
    LPVOID findnearfreememory(uintptr_t target_vmemaddr, size_t size)
    {
        LPVOID allocaddr = nullptr;

        static const uint32_t max_offset = (1LL<<32)/ *allocrice;
        uint64_t  l(target_vmemaddr/ *allocrice), r(target_vmemaddr/ *allocrice+max_offset);
//        do{
//            int64_t mid = (l&r) + ((l^r)>>1);
//            MEMORY_BASIC_INFORMATION pmbi;
//            auto retsize = VirtualQueryEx(*processHandle, (LPVOID)(mid**allocrice), &pmbi, sizeof(pmbi));
//            if (!(retsize == 0 || pmbi.State != MEM_FREE))
//            {
//                allocaddr = VirtualAllocEx(*processHandle, (LPVOID)(mid**allocrice), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
//                if(allocaddr)
//                    return allocaddr;
//            }
//            r = mid;
//        }while (r != l);

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
    }*/
};

#endif // FPS_SETTER_H