# Sockroute
Route TCP socket connections from one server to another.

## Prerequisites
- A C++ Compiler
- Boost.Program_options

## Usage
Create a JSON configuration file in your project directory. Here's a sample config file:
```json
{
    "//1": "The following two sections are required",
    "server": {
        "host": "0.0.0.0", "//1": "Hosts server on 0.0.0.0 (required)",
        "port": 8080, "//2": "Hosts server on port 8080 (required)"
    },

    "client": {
        "host": "127.0.0.1", "//1": "Routes connections to a server on localhost (required)",
        "port": 8000, "//2": "Routes connections to port 8000 (required)"
    }
}
```
In the sample above, the socket router is set up to route all connections on port 8080 to 127.0.0.1:8000. Run Sockroute with `sockroute --config config.json`. If no configuration file is specified, Sockroute looks for a file called config.json in the current directory.

## Build & Install
```
$ make
$ sudo make install
```