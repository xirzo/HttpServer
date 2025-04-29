# 🌐 WebServer

> [!WARNING]  
> This project was created for educational purposes and demonstration only.
> The code is not production-grade and may have significant limitations, bugs, or security issues.  
> **Do not use this :)**

---

## 😅 What is This?

**WebServer** is a simple HTTP server written in C.  
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
git clone https://github.com/xirzo/WebServer.git
cd WebServer
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
