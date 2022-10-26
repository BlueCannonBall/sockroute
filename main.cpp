#include "json.hpp"
#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>

#define BUFFER_SIZE        (sizeof(struct tcphdr) + sizeof(struct ip)) * 2
#define FATAL_ERROR_HEADER "Fatal error: "
#define ERROR_HEADER       "Error: "
#define PARSE_ERROR_HEADER "Invalid config file: "

namespace po = boost::program_options;
using json = nlohmann::json;

std::string config_location = "config.json";
json config;

template <class T1, class T2>
inline bool in_map(const T1& map, const T2& object) {
    return map.find(object) != map.end();
}

template <class T1, class T2>
inline bool in_vec(const T1& vec, const T2& object) {
    return find(vec.begin(), vec.end(), object) != vec.end();
}

void parse_config() {
    std::ifstream config_file(config_location);
    if (!config_file.is_open()) {
        throw std::runtime_error(std::string(strerror(errno)));
    }
    config_file >> config;

    if (!config.is_object()) {
        throw std::runtime_error("Not an object");
    }

    if (!in_map(config, "server")) {
        throw std::runtime_error("`server` key missing");
    } else if (!config["server"].is_object()) {
        throw std::runtime_error("`server` key is not an object");
    }

    if (!in_map(config, "client")) {
        throw std::runtime_error("`client` key missing");
    } else if (!config["client"].is_object()) {
        throw std::runtime_error("`client` key is not an object");
    }

    if (!in_map(config["server"], "host")) {
        throw std::runtime_error("`host` key in `server` key missing");
    } else if (!config["server"]["host"].is_string()) {
        throw std::runtime_error("`host` key in `server` key is not a string");
    }
    if (!in_map(config["server"], "port")) {
        throw std::runtime_error("`port` key in `server` key missing");
    } else if (!config["server"]["port"].is_number_unsigned()) {
        throw std::runtime_error("`port` key in `server` key is not an unsigned integer");
    }

    if (!in_map(config["client"], "host")) {
        throw std::runtime_error("`host` key in `client` key missing");
    } else if (!config["client"]["host"].is_string()) {
        throw std::runtime_error("`host` key in `client` key is not a string");
    }
    if (!in_map(config["client"], "port")) {
        throw std::runtime_error("`port` key in `client` key missing");
    } else if (!config["client"]["port"].is_number_unsigned()) {
        throw std::runtime_error("`port` key in `client` key is not an unsigned integer");
    }

    config_file.close();
}

void route(int inbound_client, int outbound_client) {
    for (;;) {
        char buffer[BUFFER_SIZE];
        int read_result;
        if ((read_result = read(outbound_client, &buffer, sizeof(buffer))) == 0) {
            close(inbound_client);
            close(outbound_client);
            return;
        } else if (read_result == -1) {
            perror(ERROR_HEADER "write");
            close(inbound_client);
            close(outbound_client);
            return;
        }

        if (write(inbound_client, &buffer, read_result) == -1) {
            perror(ERROR_HEADER "write");
            close(outbound_client);
            close(inbound_client);
            return;
        }
    }
}

void handle_conn(int inbound_client) {
    int outbound_client;
    struct sockaddr_in address;

    if ((outbound_client = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror(FATAL_ERROR_HEADER "socket");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(config["client"]["port"]);

    {
        int result;
        if ((result = inet_pton(AF_INET, std::string(config["client"]["host"]).c_str(), &address.sin_addr)) == 0) {
            std::cerr << PARSE_ERROR_HEADER << "Client host invalid" << std::endl;
            exit(1);
        } else if (result == -1) {
            std::cerr << PARSE_ERROR_HEADER << strerror(errno) << std::endl;
            exit(1);
        }
    }

    if (connect(outbound_client, (struct sockaddr*) &address, sizeof(address)) == -1) {
        perror(FATAL_ERROR_HEADER "connect");
        exit(1);
    }

    std::thread(route, outbound_client, inbound_client).detach();
    std::thread(route, inbound_client, outbound_client).detach();
}

void print_help(po::options_description& desc, char** argv) {
    std::cout << "Usage: " << argv[0] << " [OPTIONS]\n\n"
              << desc;
}

int main(int argc, char** argv) {
    po::options_description desc {"Options"};
    po::variables_map vm;

    desc.add_options()("help,h", "Show this help message and exit")("config", po::value(&config_location)->default_value("config.json"), "Path to config file");

    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            print_help(desc, argv);
            return 0;
        }
    } catch (std::exception& e) {
        if (vm.count("help")) {
            print_help(desc, argv);
            return 0;
        } else {
            std::cerr << e.what() << std::endl;
        }

        return 1;
    }

    try {
        parse_config();
    } catch (std::exception& e) {
        std::cerr << FATAL_ERROR_HEADER << PARSE_ERROR_HEADER << e.what() << std::endl;
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    int server_fd;
    int opt = 1;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror(FATAL_ERROR_HEADER "socket");
        return 1;
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror(FATAL_ERROR_HEADER "setsockopt");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(config["server"]["port"]);

    {
        int result;
        if ((result = inet_pton(AF_INET, std::string(config["server"]["host"]).c_str(), &address.sin_addr)) == 0) {
            std::cerr << PARSE_ERROR_HEADER << "Server host invalid" << std::endl;
            return 1;
        } else if (result == -1) {
            std::cerr << PARSE_ERROR_HEADER << strerror(errno) << std::endl;
            return 1;
        }
    }

    if (bind(server_fd, (struct sockaddr*) &address, sizeof(address)) == -1) {
        perror(FATAL_ERROR_HEADER "bind");
        return 1;
    }

    if (listen(server_fd, 128) == -1) {
        perror(FATAL_ERROR_HEADER "listen");
    }

    for (;;) {
        int inbound_client;
        struct sockaddr inbound_address;
        socklen_t inbound_addrlen = sizeof(inbound_address);
        if ((inbound_client = accept(server_fd, (struct sockaddr*) &inbound_address, &inbound_addrlen)) == -1) {
            perror(FATAL_ERROR_HEADER "accept");
            return 1;
        }
        handle_conn(inbound_client);
    }

    return 0;
}
