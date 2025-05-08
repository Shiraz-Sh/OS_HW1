#include <string>
#include <cstring>          // for memset, strerror
#include <unistd.h>         // for close
#include <sys/ioctl.h>      // for ioctl
#include <net/if.h>         // for struct ifreq
#include <sys/socket.h>     // for socket, sockaddr_in
#include <arpa/inet.h>      // for inet_ntoa
#include <iostream>
#include <sstream>

#include "global_utils.hpp"

std::string get_ip(const char* iface){
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0){
        SYSCALL_FAIL("socket");
        return "";
    }

    // copy the iface name into the iterface name in request
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

    // send the request for the address
    if (ioctl(fd, SIOCGIFADDR, &ifr) == -1){
        SYSCALL_FAIL("ioctl");
        if (close(fd) == -1){
            SYSCALL_FAIL("close");
        }
        return "";
    }

    if (close(fd) == -1){
        SYSCALL_FAIL("close");
    }
    // extract the address from the request result in IPv4 format 
    return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
}

std::string get_subnet_mask(const char* iface){
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0){
        SYSCALL_FAIL("socket");
        return "";
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);

    // send the request from the subnet mask
    if (ioctl(fd, SIOCGIFNETMASK, &ifr) == -1){
        SYSCALL_FAIL("ioctl");
        if (close(fd) == -1){
            SYSCALL_FAIL("close");
        }
        return "";
    }
    if (close(fd) == -1){
        SYSCALL_FAIL("close");
    }
    return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
}

bool interface_exists(const std::string& iface_name){
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0){
        SYSCALL_FAIL("socket");
        return false;
    }

    // seems like a big enough buffer
    const int BUFFERSIZE = 8192;
    char buffer[BUFFERSIZE];

    // fill the buffer with interface data using ioctl
    struct ifconf ifc;
    ifc.ifc_len = BUFFERSIZE;
    ifc.ifc_buf = buffer;

    if (ioctl(fd, SIOCGIFCONF, &ifc) < 0){
        SYSCALL_FAIL("ioctl");
        if (close(fd) == -1){
            SYSCALL_FAIL("close");
        }
        return false;
    }

    struct ifreq* ifr = reinterpret_cast<struct ifreq*>(ifc.ifc_buf); // interface info request result 
    int n = ifc.ifc_len / sizeof(struct ifreq);

    for (int i = 0; i < n; ++i){
        char* name = ifr[i].ifr_name;
        if (name != nullptr && std::strlen(name) > 0 && iface_name.compare(name) == 0)
            return true;
    }

    if (close(fd) == -1){
        SYSCALL_FAIL("close");
    }
    return false;
}

std::string get_default_gateway(){
    // read the file and convert it into stringstream
    auto file = read_file("/proc/net/route");
    std::string buffer(file.begin(), file.end());
    std::stringstream file_stream(buffer);

    // skip header
    std::string line;
    std::getline(file_stream, line);

    while (std::getline(file_stream, line)){
        std::istringstream iss(line);
        std::string iface, destination, gateway, flags;

        // ignore null rows
        if (!(iss >> iface >> destination >> gateway >> flags))
            continue;
        
        if (destination == "00000000"){
            // convert from hex to ip address integer
            unsigned long gw;
            std::stringstream ss;
            ss << std::hex << gateway;
            ss >> gw;

            struct in_addr addr;
            addr.s_addr = gw;
            return inet_ntoa(addr); // convert int to string
        }
    }
    return std::string();
}

std::vector<std::string> get_dns_servers(){
    std::vector<std::string> dns_list;

    auto file = read_file("/etc/resolv.conf");
    std::string buffer(file.begin(), file.end());
    std::stringstream file_stream(buffer);
    std::string line;

    while (std::getline(file_stream, line)){
        std::istringstream iss(line);
        std::string key, ip;
        if (iss >> key >> ip && key == "nameserver"){
            dns_list.push_back(ip);
        }
    }

    return dns_list;
}
