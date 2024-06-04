# AccSys CMake Template

## Build

### Prerequisites

- 支持 C++17 的编译器，例如 clang 或 gcc (推荐 gcc 9.4.0 或更高的版本，有完整 C++ 17 支持)
- flex & bison

#### Install Dependencies

#### Get Template Source

```bash
git clone git@git.zju.edu.cn:accsys/accsys-cmake-template.git
```


### Build

```bash
cmake -B build          # create & generate build configs under `build/` directory
cmake --build build     # build target in `build/` directory
```

#### CMake Build Type

支持 `Debug` 或者 `Release` 构建配置：

- 对于单配置构建系统 Makefile 和 Ninja，你可以通过设置 `CMAKE_BUILD_TYPE` 变量来指定构建配置
- 对于多配置构建系统 Visual Studio， Xcode 或者 Ninja Multi-Config，我们已经设置了 `Debug` 和 `Release` 两种配置

对于单配置构建系统，`CMAKE_BUILD_TYPE` 没有设置默认值，不会启用相关编译参数.
在 `Debug` 模式下，默认开启 address sanitizer

实例：

```bash
# single configuration build systems, Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefile"
cmake --build build
# single configuration build systems, Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release -G "Unix Makefile"
cmake --build build

# multi configuration build systems
cmake -B build -G "Ninja Multi-Config"
# Debug build
cmake --build build --config Debug
# Release build
cmake --build build --config Release
```

#### Accsys Components

为了方便起见，我们把 IR 等相关组件的代码放在了 `accsys/` 目录下，提供了头文件和动态链接库.
如果你不使用我们提供的模板，但打算复用一些 IR 的轮子，可以复制 `accsys/` 下的内容，并添加下列内容到你的 CMakeLists.txt

```cmake
# compile accsys components.
add_subdirectory(accsys)

# link accsys libraries to the executable.
target_link_libraries(compiler PRIVATE accsys::accsys)
# add accsys header files directory for the source files of the compiler.
target_include_directories(compiler INTERFACE accsys::accsys)
```

#### Adjust Build Options (Optional)

你可以自行修改 `CMakeLists.txt` 并添加编译选项或开关.
下面提供一些示例：

- 为 `clangd` 导出 `compile_commands.json`，可以添加 `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.
- 选用不同的构建系统生成器，使用 `-G` 指定，例如使用 `Ninja` 可以添加 `-G Ninja`.
- 添加选项 `-DACCSYS_BUILD_TESTS=ON` 可以启用编译 googletest 与 accsys 库的单元测试，使用 `cmake --build build --target test` 测试
- 对于 Unix Makefile 构建系统生成器，默认为单线程编译，可以使用 `--parallel` 并行，例如 `cmake --build build --parallel 16`

## FAQ

### Why newly added C++ source files are not compiled?

为了方便起见，我们使用 `file(GLOB)` 来指定编译的源代码，你添加了新的源码文件后 `CMakeCache` 并不会更新，构建系统不会意识到更改.
如果遇到这种情况，请重新执行一次 `cmake`.