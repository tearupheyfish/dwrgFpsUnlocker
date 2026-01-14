> 注意：使用MSVC编译时请下降到vc143版本、或使用clang-cl
> <br>&emsp;&emsp;（vc145似乎开始严格要求constexpr）

### 依赖
- Qt(qtbase)
- QHotKey
- yalantinglibs
- minizip-ng

### 编译选项
- `BUILD_SINGLE`:使用系统存储（wincred)|ON则静态编译单文件
- `BUILD_GUI`:编译GUI版本（OFF则编译命令行）

*&emsp;注意单文件需要静态的Qt库*

*&emsp;以下用于开发者:*

- `USE_LOG`:启用日志（仅gui）
- `PRE_RELEASE`:编译预发布线
- `BUILTIN_MT`: 链接静态多线程

### 编译选项
- `BUILD_SINGLE`:使用系统存储（wincred)|ON则静态编译单文件
- `BUILD_GUI`:编译GUI版本（OFF则编译命令行）

*&emsp;注意单文件需要静态的Qt库*

*&emsp;以下用于开发者:*

- `USE_LOG`:启用日志（仅gui）
- `PRE_RELEASE`:编译预发布线