## bpk

bpk packages files as binary resources availables in your project

### Requirements

  - [cmake](https://cmake.org/install/)
  - [conan](https://docs.conan.io/en/latest/installation.html)

### Download & Build

```sh
git clone --recursive https://github.com/a-cordier/bpk.git
cd bpk
mkdir build
cd build
cmake ..
make
```

### Package your files

```sh
bpk -d <src_dir> -o <file_name>.h -n <namespace>
```

### Use in your project

  - Add the output file to your project
  - Use `<namespace>::getResource(const char* key)` to get access to your file

Files are named after their relative path inside their resource directory

### Example

  - Assuming the following resource directory

```
resources
└── svg
    ├── next.svg
    ├── pause.svg
    ├── play.svg
    ├── previous.svg
    └── stop.svg
```

Running:

```sh
bpk -d ./resources -o resources.h -n resources
```

  - Will generate the following `resources.h` file

```cpp
#include <iostream>
#include <vector>
#include <map>
#include <utility>

namespace resources {

	namespace {

		std::map<std::string, std::vector<char> > data = {
			{ "svg/previous.svg", { /* Data chunks */ } },
			{ "svg/pause.svg", { /* Data chunks */ } },
			{ "svg/play.svg", { /* Data chunks */ } },
			{ "svg/next.svg", { /* Data chunks */ } },
		};
	}

	char* getResource(const char* resourceName) {
		auto it = data.find(resourceName);
		return it == data.end() ? nullptr : it->second.data();
	}
}
```

  - Resources being accessed the following way

```cpp
#include "resources.h"

auto data = resources::getResource("svg/play.svg")
```

### Integrating with CMAKE

Integration with CMAKE for automating generation at build time may be achieved using the 
[ExternalProject](https://cmake.org/cmake/help/latest/module/ExternalProject.html) module

#### CMAKE integration sample

```cmake
include(ExternalProject)

ExternalProject_Add(
        bpk
        GIT_REPOSITORY "https://github.com/a-cordier/bpk.git"
        GIT_TAG v1.0.0
        SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/bpk-src"
        BINARY_DIR        "${CMAKE_CURRENT_BINARY_DIR}/bpk-build"
        INSTALL_COMMAND ""
)

set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(RESOURCES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/resources.h")

add_custom_command(
        TARGET bpk
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND "${CMAKE_CURRENT_BINARY_DIR}/bpk-build/bin/bpk" 
	        "-o" "${RESOURCES_FILE}" 
		"-d" "${RESOURCES_DIR}" 
		"-n" "resources"
        COMMENT "Running bpk to generate resources file"
)

```
