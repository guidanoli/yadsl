# Installation {#install}

Follow the steps in order to install **yadsl** on your machine.

## Step 1: Get CMake

It is needed to install *CMake* in order to generate a build system adequate for your platform.
There are various forms of obtaining a copy of *CMake* on your machine. If you are using a *POSIX*-like machine,
the best would be to search for packages containing the keyword 'cmake' in your favourite package manager.
For those on *Windows*, you are able to obtain the necessaries binaries in the [CMake home page](https://cmake.org/).

## Step 2: Configure your build system

Having installed *CMake*, make sure its version is at least 3.12. Check it by running:

```sh
$ cmake --version
```

If you've never used CMake before, all you need to know is that it generates files suited for a certain build system.
If you are using a *POSIX*-like machine, you might want to choose the *Unix Makefile* generator. Now, if you're on
*Windows*, maybe it is better to choose a native tool like *Visual Studio* (assuming you already have it installed).
But of course these are only recommendations. You are free to play around with CMake configurations on your own.
You can have all the generator options listed by running:

```sh
cmake --help
```

Having chosen one, create a new folder in the repository root (generally called ``build``), and there run:

```sh
cmake .. -G <your-generator-name> <more-options>
```

You can customize your build environment even more by adding optional ``-D<key>=<value>`` arguments.

Here are the supported options for YADSL.

| Option                     | Description                                                          | Default value |
| :------------------------- | :------------------------------------------------------------------- | :------------ |
| ``YADSL_BUILD_TESTS``      | Build basic unit tests                                               | ON            |
| ``YADSL_BUILD_LONG_TESTS`` | Build unit tests that might take extra time to run                   | OFF           |
| ``YADSL_BUILD_SELF_TESTS`` | Build unit tests that test the testing framework                     | OFF           |
| ``YADSL_BUILD_ALL_TESTS``  | Build all unit tests                                                 | OFF           |
| ``YADSL_LUA_SUPPORT``      | Build Lua libraries (requires Lua)                                   | OFF           |
| ``YADSL_PYTHON_SUPPORT``   | Build Python libraries (requires Python)                             | OFF           |
| ``YADSL_GENERATE_DOCS``    | Build documentation (requires Doxygen)                               | OFF           |
| ``YADSL_CODE_COVERAGE``    | Build binaries ready for coverage tests (requires GCC or Clang/LLVM) | OFF           |

## Step 3: Compile the source code

Having configured your build system, simply run:

```sh
cmake --build . [--config <configuration>]
```

If your build system supports multiple configurations (such as *Visual Studio*), you can specify the type of
build by adding ``--config <configuration>`` in the build command. Note that ``<configuration>`` can be either
``Release`` or ``Debug``.

In build systems like *Unix Makefiles*, you need, instead, to add ``-DCMAKE_BUILD_TYPE=<configuration>``
in the configuration command, for example:

```sh
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
```

## Step 4: Run the tests

In order to run the *C* tests (which is always recommended), simply run:

```sh
ctest [-C <configuration>]
```

Once again, if you have multiple configurations, you may need to specify it - this time by adding ``-C <configuration>``.
