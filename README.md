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
  * [Example](#example)
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

## License

Distributed under the MIT License. See [LICENSE](https://github.com/brano-san/Logger/blob/master/LICENSE.txt) for more information.

## Authors

* **Andrey** - *C++ Developer* - [brano-san](https://github.com/brano-san) - *All work*