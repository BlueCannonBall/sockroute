# Sockroute
Route TCP socket connections from one server to another.

## Prerequisites
- Python 3 (tested on 3.8)

## Usage
Create a JSON configuration file in your project directory. Here's a sample config file:
```json
{
    "buffer_size": 65535, "//1": "Send and recieve buffer size in bytes (optional, default is 65535)",

    "packet_logging": { "//1": "This section is not required",
        "log_packets": false, "//2": "Enable packet logging (optional, default is false)",
        "log_location": "packets.log", "//3": "Log packets to a file called packets.log in the current directory (optional, default location is packets.log)"
    },

    "//2": "The next two sections are required",
    "server": {
        "host": "0.0.0.0", "//1": "Hosts server on 0.0.0.0 (required)",
        "port": 8080, "//2": "Hosts server on port 8080 (required)"
    },

    "client": {
        "host": "127.0.0.1", "//1": "Routes connections to a server on localhost (required)",
        "port": 8000, "//2": "Routes using port 8000 (required)"
    }
}
```
In the sample above, the socket router is set up to route all connections on port 8080 to 127.0.0.1:8000. Run sockroute with `./sockroute.py --config config.json`. If no configuration file is specified, sockroute looks for a file called config.json in the current directory.
