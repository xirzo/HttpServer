# HTTP/0.9 Server in C

A minimal implementation of an HTTP/0.9 server written in C. This server handles basic `GET` requests and serves raw HTML files. It is designed for educational purposes to demonstrate the simplicity of the HTTP/0.9 protocol.

---

## Features

- Supports HTTP/0.9 protocol.
- Handles `GET` requests for static HTML files.
- Serves files from the server's directory.
- Closes the connection after sending the response (non-persistent connections).

---

## Running the Server

1. Start the server:

   ```bash
   ./http_server
   ```

   By default, the server listens on `127.0.0.1` (localhost) and port `8080`.

2. To specify a custom port, pass it as an argument:

   ```bash
   ./http_server 9000
   ```

   This will start the server on port `9000`.

---

## Testing the Server

1. Create a sample HTML file in the server's directory:

   ```bash
   echo "<html><body><h1>Hello, HTTP/0.9!</h1></body></html>" > index.html
   ```

2. Use `curl` or a web browser to make a request:

   ```bash
   curl http://127.0.0.1:8080/index.html
   ```

   Output:

   ```html
   <html>
     <body>
       <h1>Hello, HTTP/0.9!</h1>
     </body>
   </html>
   ```

3. If the file does not exist, the server will close the connection without sending a response.
