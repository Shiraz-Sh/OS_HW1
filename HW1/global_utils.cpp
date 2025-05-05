#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <vector>
#include <sys/syscall.h>  // for SYS_getdents64
#include <unistd.h>       // for syscall(), close()
#include <cstring>        // for strerror
#include <errno.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <stdio.h>
#include <dirent.h>

#include "global_utils.hpp"

#define BUF_SIZE 1024 * 20


std::vector<char> read_file(const std::string& path){
    std::vector<char> buffer;

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1){
        SYSCALL_FAIL("open");
        return buffer;
    }

    const size_t chunk_size = 4096; // I think it is a reasonable size for a chunk

    while (true){
        size_t current_size = buffer.size();
        buffer.resize(current_size + chunk_size);
        ssize_t nread = read(fd, buffer.data() + current_size, chunk_size);

        // EOF encountered
        if (nread == 0)
            break;
        if (nread < 0){
            SYSCALL_FAIL("read");
            buffer.clear();
            break;
        }

        buffer.resize(current_size + nread);
        if (nread < static_cast<ssize_t>(chunk_size)) break; // finished reading
    }

    if (close(fd) == -1){
        SYSCALL_FAIL("close");
        return std::vector<char>();
    }
    return buffer;
}

struct linux_dirent64{
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

std::vector<std::string> list_directory(const std::string& path) {
    std::vector<std::string> result;

    int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        SYSCALL_FAIL("open");
        return result;
    }

    char buf[BUF_SIZE];
    while (true) {
        int nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
        if (nread == -1) {
            SYSCALL_FAIL("getdents64");
            if (close(fd) == -1){
                SYSCALL_FAIL("close");
            }
            return std::vector<std::string>();
        }
        if (nread == 0)
            break;

        for (int bpos = 0; bpos < nread;) {
            struct linux_dirent64* d = (struct linux_dirent64*)(buf + bpos);
            result.emplace_back(d->d_name);
            bpos += d->d_reclen;
        }
    }

    if (close(fd) == -1){
        SYSCALL_FAIL("close");
    }
    return result;
}
