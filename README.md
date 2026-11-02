# ARSENAL 2.0 · C++ / Qt 图形军械库

独立窗口的Linux桌面开箱模拟器。界面、业务和存档全部使用C++17；第三方库为 **Qt 6 Widgets / Qt Test**。没有Web页面，没有Python界面，无真实支付。旧版终端程序保留，两个前端共用同一套游戏引擎。

## 安装与启动（Ubuntu / Debian）

请在带桌面环境的Linux里运行。Windows可使用具备图形应用支持的WSL2 / WSLg，或Linux虚拟机。

```bash
sudo apt update
sudo apt install build-essential git pkg-config qt6-base-dev qt6-base-dev-tools fonts-noto-cjk
cd arsenal
make -j4
make run
```

`make`默认构建窗口程序 `build/release/arsenal-desktop`。启动命令写在终端里，但程序打开的是独立图形窗口，不用输入游戏命令。

首次需要安装开发依赖，后续运行无需网络。Qt由系统动态链接，压缩包不包含Qt库；不要直接把本机编译的二进制复制到缺少Qt的另一台机器。

## 页面与功能

| 页面 | 功能 |
|---|---|
| 补给站 | 三种箱子卡片、原创箱子绘图、公开概率、单箱/五箱/十箱 |
| 开箱弹窗 | 滚动卡片、减速动画、中心指针、逐件揭晓、估值 |
| 我的军械库 | 武器缩略图、搜索、稀有度筛选、数值排序、锁定、确认出售、CSV导出 |
| 数据与成就 | 掉落柱状图、支出与持仓收益、七项成就、本次会话操作记录 |
| 概率实验室 | 最多100万次抽样、理论概率标线、进度、停止；分批执行保持窗口响应 |
| 关于与帮助 | 操作说明、Make/Git教学要点、项目说明 |

全局提供每日5000虚拟积分与存档备份。所有修改先保存再更新界面。开箱关闭动画不会丢失奖励，奖励在动画播放前就已入库。为保持课堂可讲解性，武器图采用原创抽象剪影，不使用CS商业贴图。

## 快捷命令

```bash
make check-deps        # 检查Qt、moc/rcc与编译器
make -j4              # 默认构建Qt窗口程序
make run              # 运行桌面版
make test             # 核心 + 终端 + Qt交互测试
make gui-test         # Qt离屏交互测试，不需要桌面显示
make debug            # 生成build/debug/arsenal-desktop
make core-test        # 不要求Qt的原核心与终端测试
make cli-run          # 原终端版
make demo             # 原终端版的自动演示（临时档案）
make clean            # 清理构建目录，不删存档
make dist             # 纯源码打包，不含Git历史
```

指定独立课堂档案：

```bash
./build/release/arsenal-desktop --save ./classroom.sav
```

默认档案：`$XDG_DATA_HOME/arsenal/profile.sav`，或 `$HOME/.local/share/arsenal/profile.sav`。兼容1.0存档。终端版与桌面版不能同时打开同一个档案，Linux文件锁会阻止重复访问。损坏存档报错退出，不会用空档覆盖。

## VS Code

用VS Code打开整个 `arsenal` 文件夹。内置 `.vscode/tasks.json`：**Ctrl+Shift+B**通过Makefile构建。已安装微软C/C++扩展与gdb时，可按 **F5**构建Debug并调试图形窗口。

F5所需：`sudo apt install gdb`。扩展建议在 `.vscode/extensions.json`。不要用“只编译当前main.cpp”的快捷运行方式，GUI依赖多个模块、moc生成文件和Qt链接库。

## 工程结构

```text
include/ + src/        原始C++引擎、数据模型、存档、终端前端
 gui/session.*        Qt控制层，事务、通知、导出、备份
 gui/art.*            QPainter原创箱子/武器与统计图
 gui/store.cpp        箱子商店
 gui/inventory.cpp    背包表格、筛选、交易
 gui/reveal.*         开箱滚动动画
 gui/statistics.cpp   统计、成就、非阻塞概率实验
 gui/window.*         导航和主窗口
 gui/main.cpp         QApplication入口
resources/            QSS样式与Qt资源清单
 tests/               核心、CLI与Qt交互测试
 .vscode/             Make构建与GDB调试配置
 docs/                Make/Git讲解、展示流程、验证说明、实际窗口截图
```

## Make与Git怎样体现

Make不仅编译 `.cpp`，还驱动 **moc生成信号槽代码、rcc打包样式资源、pkg-config查找Qt**。生成文件、业务源码分别编译再统一链接；`.d`自动依赖保证修改头文件后正确重编译；Debug/Release目录隔离。

Git历史保留从核心引擎到终端版再到桌面版的演进，含功能分支、非快进合并和发布标签。所有提交在本次制作时由助手创建，使用本仓库的演示身份；不代表真实长期开发或多人协作，旧版交付时未推送；本次远程提交与验证见 [远程仓库实践](docs/远程仓库实践.md)。

```bash
git log --all --graph --oneline --decorate
git diff v1.0..v2.0 --stat
git show feature/qt-desktop --stat
git tag
```

完整交付ZIP含 `.git` 和 `arsenal-history.bundle`。如解压工具遗漏隐藏目录，可在外面执行 `git clone arsenal/arsenal-history.bundle arsenal-restored`恢复已提交源码；bundle还原后功能分支位于远程跟踪分支下，可用 `git branch -a` 查看。

## 建议阅读

1. [桌面版展示与构建讲解](docs/桌面版指南.md)：本次升级的重点与课堂流程。
2. [Make与Git入门](docs/教学与展示.md)：从零理解变量、依赖、暂存、分支。
3. [原引擎设计](docs/设计说明.md)：概率、估值、存档一致性。
4. [Qt第三方依赖说明](THIRD_PARTY.md)：官方参考与依赖范围。
5. [桌面版验证记录](docs/桌面版验证.md)。

本项目是模块化课程作业规模的单机桌面程序，尚不包含联网账号、真实交易或商业级安全机制。每日奖励基于本机日期；文本档案可被用户编辑。备份采用原存档格式，未提供图形化恢复入口；关闭程序后手动备份原档并替换即可。

## 课程提交材料

- [课程知识点与项目对应](docs/课程知识点与项目对应.md)
- [小组实验报告](docs/小组实验报告.md)：正文已补充，成员信息和拟定比例已填，具体职责待小组核对。
- [远程仓库实践](docs/远程仓库实践.md)
- [本次验证记录](docs/本次验证记录.md)
- [修订汇报PPT](docs/submission/arsenal-linux-presentation-revised.pptx)
- [实验报告Word版](docs/submission/小组实验报告.docx)

本次未提供老师的报告模板及课程课件原文，因此不能声称逐页核对或完全符合指定模板。
