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

