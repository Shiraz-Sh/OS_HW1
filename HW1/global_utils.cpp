#include <unistd.h>
#include <fcntl.h>

#include "global_utils.hpp"


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

    close(fd);
    return buffer;
}