<h1 align="center">
  Logger
</h1>
  
<p align="center">
  Based On Asynchronous Low Latency C++ Logging Library Quill
  <br/>
</p>

## Table Of Contents
* [About the Project](#about-the-project)
* [Getting Started](#getting-started)
  * [Installation](#installation)
* [Usage](#usage)
  * [Logging Defines](#logging-defines)
* [Features](#features)
  * [Logging to Console and File](#logging-to-console-and-file)
  * [Categories](#categories)
  * [Logging Settings](#logging-settings)
  * [Boost StackTrace Output On Application Crash](#boost-stacktrace-output-on-application-crash)
* [License](#license)
* [Authors](#authors)

## About The Project

Logger based on [Quill](https://github.com/odygrd/quill) Logger. Provides Comfortable And Fast Interface To Log Data To Console And File.
```C++
LOG_INFO(Core, "Information Log Example");
// Output: [18:50:12.592225411] [8992] [   main.cpp:15    ] [   INFO    ] [ Core ] Core - Information Log Example
```

## Getting Started

### Installation
CMake connection:
1. Clone this project
2. Add subdirectory with cloned ```Logger``` project
3. Optionally set ```ENABLE_DEBUG``` variable (see [Usage](#usage))
4. Link Your target with target ```Logger```
```C++
set(ENABLE_DEBUG ON)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/third_party/Logger")
target_link_libraries(${PROJECT_NAME} PRIVATE Logger)
```

## Usage

### Logging Defines
Log names same as in [Quill](https://github.com/odygrd/quill) library, but with other interface:
```C++
LOG_INFO(<Category>, "<Message>", <Args>);
LOGV_INFO(<Category>, "<Message>", <Args>);
LOG_INFO_LIMIT_TIME(<Category>, <Time>, "<Message>", <Args>);
LOG_INFO_LIMIT_EVERY_N(<Category>, <Count>, "<Message>", <Args>);
```

### Features

#### Logging to Console and File
All of logging defines write messages to file and console at the same time. Example:
```C++
LOG_INFO(Core, "My First Log");
// Console: [19:49:42.221020444] [8916] [        main.cpp:31         ] [   INFO    ] [ Core ] My First Log
// File:    [19:49:42.221020444] [8916] [        main.cpp:31         ] [   INFO    ] [ Core ] My First Log
```
File logs saves in folder `logs` with name `log_<Application Start Time>.txt`, e.g. `log_29_06_2025_19_49_42.txt`

#### Categories
Define logger categories:
```C++
GENENUM(uint8_t, CoreLauncherSource, Core, OtherCategory); // CoreLauncherSource - enum of logger categories
DEFINE_CAT_LOGGER_MODULE(CoreLauncher, CoreLauncherSources); // CoreLauncher - Logger instance name
```
And then use them for log. Auto alignment for:
```C++
LOG_INFO(Core, "Core Category Log");
LOG_INFO(OtherCategory, "OtherCategory Log");
// Output:
// [20:27:52.686632538] [20760] [        main.cpp:31         ] [   INFO    ] [     Core      ] Core Category Log
// [20:27:52.686632800] [20760] [        main.cpp:32         ] [   INFO    ] [ OtherCategory ] OtherCategory Log
```


#### Logging Settings
To configure logging levels for different outputs by `LogSettings.ini` file:
```ini
[Description]
TRACE_L3 = T3
TRACE_L2 = T2
TRACE_L1 = T1
DEBUG = D
INFO = I
NOTICE = N
WARNING = W
ERROR = E
CRITICAL = C
BACKTRACE = BT
NONE = _

[Core]
Console = I
File = T3

[OtherCategory]
Console = I
File = T3
```
`[Core]` - Category name to configure
`Console = I` - Configure console output. In `Description` category includes all of possible logging levels
`File = T3` - Configure log level of file output

#### Boost StackTrace Output On Application Crash
Enable feature by set cmake variable `ENABLE_DEBUG`:
```cmake
set(ENABLE_DEBUG ON)
```
Enable feature in C++ code:
```C++
debug::setStackTraceOutputOnCrash(logger::s_CoreLauncherLogger.getFirstLoggerOrNullptr());
```
Then On Crash Stacktrace will be handled and printed to Console and written to File:
```C++
[20:27:53.518325478] [20760] [     StackTrace.cpp:49      ] [ CRITICAL  ] [     Core      ] CRASH Address[00007FF6ED84FA13] Location[0x000000000009FA13 in C:\LoggerLauncher\build\bin\LoggerLauncher\LoggerLauncher.exe]
[20:27:53.518325478] [20760] [     StackTrace.cpp:49      ] [ CRITICAL  ] [     Core      ] Address[00007FF6ED84AED3] Location[0x000000000009AED3 in C:\LoggerLauncher\build\bin\LoggerLauncher\LoggerLauncher.exe]
[20:27:53.518325478] [20760] [     StackTrace.cpp:49      ] [ CRITICAL  ] [     Core      ] Address[00007FF6ED84B4BD] Location[0x000000000009B4BD in C:\LoggerLauncher\build\bin\LoggerLauncher\LoggerLauncher.exe]
```


## License

Distributed under the MIT License. See [LICENSE](https://github.com/brano-san/Logger/blob/master/LICENSE.txt) for more information.

## Authors

* **Andrey** - *C++ Developer* - [brano-san](https://github.com/brano-san) - *All work*