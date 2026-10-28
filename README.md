# ARSENAL 军械库：Linux C++ 开箱模拟器

一个用于 Linux 课程展示的 C++17 终端项目。灵感来自游戏开箱，饰品名称、估值和概率都是自定义数据，仅使用虚拟积分。项目侧重多文件编译、模块设计、文件持久化、Linux 文件锁和 Git 版本管理。

## 快速开始

需要 Linux、支持 C++17 的 g++、GNU Make、Git；自动化测试还需要 Python 3。Ubuntu/Debian 如缺少工具，可自行运行 `sudo apt install build-essential git python3`。无需任何第三方 C++ 库，也无需联网运行。

```bash
cd arsenal
make -j4
make run
```

默认生成 `build/release/arsenal`，初始积分 10000。输入：

```text
cases
daily
open 2 5
bag price
lock 1
sell 1
sell-low
stats
achievements
simulate 3 100000
quit
```

`lock` 是切换状态，再运行一次解锁。`sell-low` 出售未锁定的消费级与军规级。`simulate` 使用独立随机引擎，不修改余额、背包或游戏开箱随机序列。

```bash
make demo                    # 临时独立存档的完整演示，不影响个人存档
make test                    # 核心测试 + CLI端到端测试
make debug                   # build/debug/arsenal，可用gdb调试
make sanitize                # 地址/未定义行为检查器 + 测试
make dist                    # dist/arsenal-source.tar.gz，源码包不含Git历史
make clean                   # 只删除build目录，不删除存档
```

## 功能

- 三种箱子、24种饰品、六档稀有度，概率透明且每次独立随机；没有保底。
- 单抽/十连、终端彩色输出及开箱进度动画；重定向输出时自动禁用颜色和动画。
- 磨损值、五种外观等级、图案编号、10% StatTrak，按磨损与计数器估值。
- 背包按ID、价格、稀有度排序，可按稀有度筛选；单卖、批量出售、锁定。
- 每日补给、七项成就、资产与掉落统计、大样本概率模拟。
- 自动保存、版本化文本存档、数据校验、同目录临时文件替换、Linux进程互斥锁。
- 非法参数检查、文件读写失败提示、损坏存档拒绝加载而不覆盖。

## 项目结构

```text
arsenal/
  include/       model / engine / storage / ui 头文件
  src/           五个实现模块：模型、引擎、存储、界面、入口
  tests/         核心C++测试与Python命令行测试
  scripts/       隔离存档的自动演示
  docs/          Makefile、Git与课堂展示讲解
  Makefile       多模式编译、自动依赖、测试、清理和打包
  README.md
```

详细阅读顺序：先 [入门与演示](docs/教学与展示.md)，再 [设计说明](docs/设计说明.md)。

## 存档与命令行

优先使用 `$XDG_DATA_HOME/arsenal/profile.sav`，否则使用 `$HOME/.local/share/arsenal/profile.sav`。没有这两个变量时使用当前目录的 `profile.sav`。

```bash
./build/release/arsenal --save ./my-demo.sav --seed 42 --no-animation
./build/release/arsenal --help
```

`--seed` 用于同一编译环境下复现随机序列，不同标准库实现可能产生不同结果。默认种子来自 `std::random_device`。
存档是可编辑文本，不用于防作弊；每日补给依赖本机日期。程序防止当前存档重复领取或日期回退领取，不提供联网授时。保存采用 rename 原子替换，未实现断电级 fsync 保证。

## Git历史

交付压缩包包含 `.git` 和 `arsenal-history.bundle`。提交由自动化助手在本次制作过程中创建，用于课程展示；不代表长时间开发或多人协作。作者使用项目内演示身份，不修改系统Git配置。未创建远程仓库或推送。

```bash
git log --all --oneline --graph --decorate
git branch -a
git show v0.1-core --stat
git diff v0.1-core..v1.0 --stat
```

若解压工具遗漏 `.git`，可以在项目目录外用 bundle 还原：

```bash
git clone arsenal/arsenal-history.bundle arsenal-restored
```

bundle 包含完整Git已提交源码；自身无需被Git跟踪。课堂要展示历史，请使用完整交付包，不要使用 `make dist` 生成的纯源码包。
