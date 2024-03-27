# How to Build

You can build Tachyon with this guide. The instructions below will help you set up your environment, install necessary dependencies, and compile your projects efficiently. Follow along to start building with confidence.

## Prerequisites

### Bazel

Follow the installation instructions for Bazel [here](https://bazel.build/install).

### Requirements for each OS

#### Ubuntu

```shell
sudo apt install libgmp-dev libomp-dev
```

#### MacOS

```shell
brew install gmp
```

### Python dependencies

#### Python

- Install [Python](https://www.python.org/downloads/). Python **v3.10.12** is recommended.

- If you are using pyenv, don't forget to add an option `--enable-shared`.

  ```shell
  CONFIGURE_OPTS=--enable-shared pyenv install <version>
  ```

#### Matplotlib(optional)

If you want to print a benchmark plot, you need to install matplotlib. See [here](/benchmark/README.md) for more details.

```shell
pip install matplotlib
```

**_NOTE_:**

- To build `mac_logging` in MacOS, you need to install objc, which can be done by installing XCode.

- MacOS v14.0.0 or later is recommended. In certain versions of MacOS (prior to v13.5.1), a bug related to incorrect `BigInt` divide operations has been detected in the field generator when using the optimized build (`-c opt`). This [issue](https://github.com/kroma-network/tachyon/issues/98) will be fixed as soon as possible.

## Build from source

### Build

#### Build on Linux

```shell
bazel build --config linux //...
```

#### Build on Macos arm64

```shell
bazel build --config macos_arm64 //...
```

#### Build on Macos x64

```shell
bazel build --config macos_x86_64 //...
```

### Test

#### Test on Linux

```shell
bazel test --config linux //...
```

#### Test on Macos arm64

```shell
bazel test --config macos_arm64 //...
```

#### Test on Macos x64

```shell
bazel test --config macos_x86_64 //...
```

## Configurations

### OS options

- `linux`
- `macos_x86_64`
- `macos_arm64`

```shell
bazel build --config macos_arm64 //...
```

### Build options

- `opt`: Default optimized build option
- `dbg`: Build with debug info
- `fastbuild`: Fast build option

```shell
bazel build --config macos_arm64 --config dbg //...
```

### Hardware acceleration

#### CUDA backend

- `--config cuda`: Enable [cuda] backend.

  ```shell
  bazel build --config ${os} --config cuda //...
  ```

#### ROCm backend

- `--config rocm`: Enable [rocm] backend.

  ```shell
  bazel build --config ${os} --config rocm //...
  ```

_NOTE_: The `rocm` option is not recommended for current use because it is not being tested yet.

[cuda]: https://developer.nvidia.com/cuda-toolkit
[rocm]: https://www.amd.com/en/graphics/servers-solutions-rocm

### .bazelrc.user

Build options can be preset in `.bazelrc.user` for your convenience, eliminating the need to specify them on the command line.

For example:

```
# .bazelrc.user

build --config linux
build --config dbg
```

```shell
bazel build //...
# With the preset options in .bazelrc.user, this is the same as:
# bazel build --config linux --config dbg //...
```

## Building Tachyon from a Bazel repository

Tachyon can be built in your own Bazel project with the following two simple steps.

First, obtain the Tachyon code from a specific commit hash and get a SHA256 value from the fetched code through these commands:

```shell
wget https://github.com/kroma-network/tachyon/archive/c6a38895cb6f9b9f73e3d64928d85a9762b66639.tar.gz

shasum -a 256 c6a38895cb6f9b9f73e3d64928d85a9762b66639.tar.gz
```

Second, input the shasum output into your `WORKSPACE` file as the `sha256` argument like shown below:

```bzl
# WORKSPACE

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
http_archive(
    name = "kroma_network_tachyon",
    sha256 = "3ebc4edca153c9b93de200d5e22be66d1ff7ea52e2793c83947607c532dfacbc",
    strip_prefix = "tachyon-c6a38895cb6f9b9f73e3d64928d85a9762b66639",
    urls = ["https://github.com/kroma-network/tachyon/archive/c6a38895cb6f9b9f73e3d64928d85a9762b66639.tar.gz"],
)
```

## Debian packaging

There are two ways to install the Tachyon package. While it is recommended to download prebuilt binaries, it is also possible to build the package from source if necessary.

### Install package from pre-built binaries

``` shell
curl -LO https://github.com/kroma-network/tachyon/releases/download/v0.1.0/libtachyon_0.1.0_amd64.deb
curl -LO https://github.com/kroma-network/tachyon/releases/download/v0.1.0/libtachyon-dev_0.1.0_amd64.deb

sudo dpkg -i libtachyon_0.1.0_amd64.deb
sudo dpkg -i libtachyon-dev_0.1.0_amd64.deb
```

### Build package from source

Build a Debian package with the supported scheme (only halo2 for now) and the [options](#options) you want.
To build the Halo2 Debian package, `halo2` and `has_openmp` options are recommended. Run the following commands:

```shell
bazel build -c opt --config halo2 --//:has_openmp  --//:c_shared_object //scripts/packages/debian/runtime:debian
bazel build -c opt --config halo2 --//:has_openmp  --//:c_shared_object //scripts/packages/debian/dev:debian

sudo dpkg -i bazel-bin/scripts/packages/debian/runtime/libtachyon_0.1.0_amd64.deb
sudo dpkg -i bazel-bin/scripts/packages/debian/dev/libtachyon-dev_0.1.0_amd64.deb
```

## Other Info

### Debugging on macOS

Please add this line to your `.bazelrc.user`.

```
build --spawn_strategy=local
```

### Py Binding

`ModuleNotFoundError` may occur in certain python versions (v3.11.6). Python v3.10.12 is recommended.

### Build on Ubuntu 20.04

If you are using Ubuntu 20.04, update your g++ version. The default `g++-9` does not work.

```shell
sudo apt install g++-10
export CC=/usr/bin/gcc-10
export CXX=/usr/bin/g++-10
export GCC_HOST_COMPILER_PATH=/usr/bin/gcc-10
```

### Build CUDA with rust toolchain

You may run into the following problem:

```shell
Use --sandbox_debug to see verbose messages from the sandbox and retain the sandbox build root for debugging
error: linking with `external/local_config_cuda/crosstool/clang/bin/crosstool_wrapper_driver_is_not_gcc` failed: exit status: 127
...
  = note: /usr/bin/env: 'python': No such file or directory
```

If so, make your `python` point to the python interpreter.

```shell
sudo apt install python-is-python3
```

Additionally, please include these lines in your `.bazelc.user`.

```
build --action_env=PATH=/usr/bin:/usr/local/bin
build --host_action_env=PATH=/usr/bin:/usr/local/bin
```
