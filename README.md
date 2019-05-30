## CPP Binary Loader

bpk packages files in a resources directory to make them available in your project as binary resources.

### Build & Install (with cmake)

[conan](https://docs.conan.io/en/latest/installation.html) is required for dependency management.

```sh
git clone https://github.com/a-cordier/cpp-binary-loader
cd cpp-binary-loader
mkdir build
cd build
cmake ..
make
```

### Package your files

```sh
./binary-loader -d <src_dir> -o <file_name>.h -n <namespace>
```

### Use in your project

  - Add the output file to your project
  - Use `<namespace>.getResource(const char* key)` to get access to your file

File are identified by their relative path inside their resource directory

Example:

Assuming the following resource directory:

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
bpk -d ./resources -o resources.h -n Resource
```

Will generate the following `resources.h` file:

```cpp
#include <iostream>
#include <vector>
#include <map>
#include <utility>

namespace Resources {

	namespace {

		std::map<std::string, std::vector<char> > data = {
			{ "svg/previous.svg", {
					/* Data chunks */
				}
			},
			{ "svg/pause.svg", {
					/* Data chunks */
				}
			},
			{ "svg/play.svg", {
			        /* Data chunks */
				}
			},
			{ "svg/next.svg", {
                    /* Data chunks */
				}
			},
		};
	}

	char* getResource(const char* resourceName) {
		auto it = data.find(resourceName);
		return it == data.end() ? nullptr : it->second.data();
	}
}
```

Resources being accessed the following way:

```cpp
auto data = Resources.getResource("svg/play.svg")
```

