# AccSys CMake Template

## Build

### Prerequisites

- A compiler that supports C++17, such as clang or gcc (gcc 9.4.0 or newer is recommended)
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

#### Adjust Build Options (Optional)

You can modify `CMakeLists.txt` and pass build options to cmake.
For example, export `compile_commands.json` with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` if you are using `clangd`