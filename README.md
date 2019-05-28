## CPP Binary Loader

A utility that reads files in one or more directories and write them
as binary resources packed in a cpp namespace to make them accessible
at runtime by a project.

### Build (with cmake)

```sh
cmake .
make
```

### Usage

```sh
./binary-loader -d resources
```