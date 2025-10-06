//
// Created by Tofu on 25-6-16.
//

#include "winapiutil.h"
#include "fpssetter.h"
#include "errreport.h"

#ifdef _DEBUG
#include <psapi.h>
#endif

#include <QFile>
#include <QDebug>

#include <filesystem>

bool FpsSetter::getAddress() {

    static constexpr char funcname[]            = "PyOS_ReadlineFunctionPointer";
    static constexpr char targetModuleName[]    = "neox_engine.dll";

    //获取模块地址
    moduleBase = GetModuleBaseAddress(processID, targetModuleName);
    if (!moduleBase) {
        ErrorReporter::instance()->receive(ErrorReporter::严重, "无法找到模块基址");
        bad = true;
        return false;
    }
    qDebug() << "dll基址: " << Qt::hex << moduleBase;

    //查询函数符号地址
    auto t = clock();
    funcaddr = getProcAddressExBuffered(processHandle, moduleBase, funcname);
    if (!funcaddr) {
        ErrorReporter::instance()->receive(ErrorReporter::严重, "无法查询符号");
        bad = true;
        return false;
    }
    qDebug()<<"解析文件头用时"<<static_cast<float>(clock() - t)/CLOCKS_PER_SEC;

    //读取dyrcx
    if (!ReadProcessMemory(processHandle, (LPCVOID) DYRCX_P_OFFSET, &dyrcx, sizeof(dyrcx),
                           nullptr)) {
        ErrorReporter::instance()->receive(ErrorReporter::严重, "无法获取数组指针");
        qCritical()<<"读取"<<Qt::hex<<processHandle<<"::"<<(DYRCX_P_OFFSET)<<"失败："<<GetLastError();
        bad = true;
        return false;
    }
    //还是dyrcx
    if (!ReadProcessMemory(processHandle, (LPCVOID) (dyrcx + DYRCX_O_OFFSET), &dyrcx, sizeof(dyrcx),
                           nullptr)) {
        ErrorReporter::instance()->receive(ErrorReporter::严重, "无法获取数组指针");
        qCritical()<<"读取"<<Qt::hex<<(dyrcx+DYRCX_O_OFFSET)<<"失败："<<GetLastError();
        bad = true;
        return false;
    }
    qDebug() << "动态内存地址: " << Qt::hex << dyrcx;

    //读取pfraddr
    if (!ReadProcessMemory(processHandle, (LPCVOID) (dyrcx + PFR_OFFSET), &preframerateaddr, sizeof(preframerateaddr),
                           nullptr)) {
        ErrorReporter::instance()->receive(ErrorReporter::警告, "无法获取帧率所在内存段的指针");
        qCritical()<<"读取"<<Qt::hex<<(dyrcx+PFR_OFFSET)<<"失败："<<GetLastError();
        fpsbad = true;
    } else
        qInfo() << "帧率地址: " << Qt::hex << preframerateaddr + FR_OFFSET;

    return true;
}

