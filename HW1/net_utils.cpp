#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <string.h>

#include "global_macros.hpp"

std::string get_ip(const char* iface){
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0){
        SYSCALL_FAIL("socket");
        return "";
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) == -1){
        SYSCALL_FAIL("ioctl");
        close(fd);
        return "";
    }
    close(fd);
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

    if (ioctl(fd, SIOCGIFNETMASK, &ifr) == -1){
        SYSCALL_FAIL("ioctl");
        close(fd);
        return "";
    }
    close(fd);
    return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
}

bool interface_exists(const std::string& iface_name){
    struct ifaddrs* ifaddr, * ifa;
    bool found = false;

    if (getifaddrs(&ifaddr) == -1){
        SYSCALL_FAIL("getifaddrs");
        return false;
    }

    // go over the addresses linked list
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next){
        if (ifa->ifa_name == nullptr)
            continue;
        if (iface_name.compare(ifa->ifa_name) == 0){
            found = true;
            break;
        }
    }

    freeifaddrs(ifaddr);
    return found;
}

// std::string getDefaultGateway(){
//     int route_fd = open("/proc/net/route", O_WRONLY);
//     if (fd < 0){
//         SYSCALL_FAIL("open");
//     }
//     std::string line;
//     std::getline(route_fd, line); // skip header

//     while (std::getline(route_fd, line)){
//         std::istringstream iss(line);
//         std::string iface, destination, gateway, flags;
//         iss >> iface >> destination >> gateway >> flags;
//         if (destination == "00000000"){
//             unsigned long gw;
//             std::stringstream ss;
//             ss << std::hex << gateway;
//             ss >> gw;

//             struct in_addr addr;
//             addr.s_addr = gw;
//             return inet_ntoa(addr); // e.g. "192.168.1.1"
//         }
//     }

//     close(route_fd);
//     return "No default gateway found";
// }