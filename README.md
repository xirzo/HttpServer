# 🌐 HttpServer

> [!WARNING]  
> This project was created for educational purposes and demonstration only.
> The code is not production-grade and may have significant limitations, bugs, or security issues.  
> **Do not use this :)**

---

## 🤔 What is This?

**HttpServer** is a simple HTTP server written in C.  
It serves static files, maps URL routes to files, and tries to handle basic HTTP requests. It is intentionally minimal and imperfect, meant to help me learn about:

- Berkeley sockets
- HTTP basics
- Routing and serving static files
- Handling content types

---

## 🏗️ How it Works

- Uses low-level POSIX sockets to accept connections
- Parses HTTP requests with [HttpParser](https://github.com/xirzo/HttpParser)
- Maps URL paths to files using a simple `Routes` structure
- Sends responses (with basic content-type detection) back to clients

---

## 📦 Directory Structure

- `assets/` - Static files served to clients (HTML, CSS, etc.)
- `src/` - C source files (server logic, routing, etc.)

---

## 🚀 Getting Started

### 1. Clone the repo

```sh
git clone https://github.com/xirzo/HttpServer.git
cd HttpServer
```

### 2. Build (requires CMake)

```sh
mkdir build
cd build
cmake ..
make
```

### 3. Run

```sh
./webserver
```

### 4. Open in your browser

```
http://localhost:5000 (uses port 5000 by default)
```

---

## 📚 Using This Library

### Building and Installing

To build and install the HttpServer library system-wide:

```sh
mkdir build
cd build
cmake ..
cmake --build .
sudo cmake --install . --prefix /usr/local
```

### Using with FetchContent

You can include this library in your CMake project using FetchContent:

```cmake
include(FetchContent)

FetchContent_Declare(
  HttpServer
  GIT_REPOSITORY https://github.com/xirzo/HttpServer.git
  GIT_TAG        main
)

FetchContent_MakeAvailable(HttpServer)

# Link to your target
target_link_libraries(your_target PRIVATE HttpServer::HttpServer)
```

### Using with find_package

If you have installed the library globally, you can use it with find_package:

```cmake
find_package(HttpServer REQUIRED)
target_link_libraries(your_target PRIVATE HttpServer::HttpServer)
```

> **Note**: The library requires [HttpParser](https://github.com/xirzo/HttpParser) as a dependency.
