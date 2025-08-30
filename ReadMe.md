用于显示[flecs](https://github.com/SanderMertens/flecs)内部数据，用于更方便地理解flecs工作原理的程序。

## 如何编译

使用git submodule拉取第三方库子模块：

```bash
git submodule update --init --recursive
```

然后使用cmake编译:

```bash
cmake -S . -B cmake-build
cmake --build cmake-build
```