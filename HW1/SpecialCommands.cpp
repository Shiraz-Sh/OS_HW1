#include <sys/types.h>
#include <pwd.h>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sys/wait.h>
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <utility>
#include <set>

#include "SpecialCommands.hpp"
#include "parsing_utils.hpp"
#include "net_utils.hpp"
#include "global_utils.hpp"

#define MAX_ARGS 20

void WhoamiCommand::execute() {
    this->prepare();
    uid_t uid = getuid();  // current user ID

    int fd = open("/etc/passwd", O_RDONLY);
    if (fd == -1) {
        SYSCALL_FAIL("open");
        this->cleanup();
        return;
    }

    constexpr size_t BUF_SIZE = 4096; // On most systems, the memory page size is 4096 bytes
    char buffer[BUF_SIZE];
    ssize_t bytes_read;
    std::string content;

    // Read entire file content into `content`
    while ((bytes_read = read(fd, buffer, BUF_SIZE)) > 0) {
        content.append(buffer, bytes_read);
    }
    if (close(fd) == -1){
        SYSCALL_FAIL("close");
    }

    if (bytes_read == -1) {
        SYSCALL_FAIL("read");
        this->cleanup();
        return;
    }

    // Parse line-by-line
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        std::istringstream linestream(line);
        std::string username, x, uid_str, gid, comment, home;

        std::getline(linestream, username, ':');
        std::getline(linestream, x, ':');
        std::getline(linestream, uid_str, ':');

        if (static_cast<uid_t>(std::stoi(uid_str)) == uid) { // checks if the user is our user
            std::getline(linestream, gid, ':');
            std::getline(linestream, comment, ':');
            std::getline(linestream, home, ':');
            std::cout << username << " " << home << std::endl;
            break;
        }
    }
    this->cleanup();
}

void PipeCommand::execute() {
    char* pipe_pos = strchr(cmd_line, '|'); // Find the '|' character

    // Determine if it's a simple pipe or error pipe
    bool isSimplePipe = !(pipe_pos[1] == '&');

    // copy the first part before '|'
    size_t len1 = pipe_pos - cmd_line;
    char* cmd_line1 = new char[len1 + 1]; // +1 for null terminator
    strncpy(cmd_line1, cmd_line, len1);
    cmd_line1[len1] = '\0'; // null-terminate

    char* cmd2_start;
    // copy the second part after '|'
    if (isSimplePipe) {
        cmd2_start = pipe_pos + 1;
    } else {
        cmd2_start = pipe_pos + 2;
    }

    // skip leading spaces in cmd_line2
    while (*cmd2_start == ' ') ++cmd2_start;
    char* cmd_line2 = strdup(cmd2_start);

    // Gets the relevant commands
    SmallShell& smash = SmallShell::getInstance();
    Command* cmd1 = smash.CreateCommand(cmd_line1);
    Command* cmd2 = smash.CreateCommand(cmd_line2);

    int pipefd[2];  // pipefd[0] for the read end and pipefd[1] for the write end
    if (pipe(pipefd) == -1) {
        SYSCALL_FAIL("pipe");
        this->cleanup();
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == -1){
        SYSCALL_FAIL("fork");
        this->cleanup();
        return;
    } else if (pid1 == 0) {      // first son process
        // Redirect
        setpgrp();

        if (isSimplePipe){                   // we use operator '|'
            dup2(pipefd[1], STDOUT_FILENO);     // replace stdout with pipe's write end
        } else {                               // we use operator '|&'
            dup2(pipefd[1], STDERR_FILENO);      // replace stderr with pipe’s write end
        }

        int res1 = close(pipefd[0]);
        int res2 = close(pipefd[1]);
        if (res1 == -1 || res2 == -1){  // close unused read end
            SYSCALL_FAIL("close");
            exit(0);
        }
        
        cmd1->execute();
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        SYSCALL_FAIL("fork");
        this->cleanup();
        return;
    }
    else if (pid2 == 0){       // second son process
        setpgrp();
        
        // Redirect
        if (isSimplePipe) {                 // we use operator '|'
            dup2(pipefd[0], STDIN_FILENO);     // replace stdin with pipe's read end
        } else {                             // we use operator '|&'
            dup2(pipefd[0], STDIN_FILENO);      // replace stdin with pipe's read end
        }

        int res1 = close(pipefd[0]);
        int res2 = close(pipefd[1]);
        if (res1 == -1 || res2 == -1){  // close unused read end
            SYSCALL_FAIL("close");
            exit(0);
        }
        cmd2->execute();
        exit(0);
    }

    // parent process - only the parent comes here thanks to exit()

    int res1 = close(pipefd[0]);
    int res2 = close(pipefd[1]);
    if (res1 == -1 || res2 == -1){  // close unused read end
        SYSCALL_FAIL("close");
        free(cmd_line2);
        delete[] cmd_line1;
        exit(0);
    }

    if (waitpid(pid1, nullptr, 0) == -1)
        SYSCALL_FAIL("waitpid");
    if (waitpid(pid2, nullptr, 0) == -1)
        SYSCALL_FAIL("waitpid");

    delete[] cmd_line1;
    free(cmd_line2);
    delete cmd1;
    delete cmd2;
}

void RedirectionCommand::execute(){
    // TODO: implement
    prepare();

    std::string src, dst;
    std::string buffer;

    std::stringstream line_s;
    line_s << args[0];
    for (int i = 1; i < count; i++){
        line_s << " " << args[i];
    }

    std::string line = line_s.str();
    auto pos = line.find(">");

    // check for appending / creating new file
    bool append = false;
    if (pos != std::string::npos){
        src = _trim(line.substr(0, pos));
        if (line[pos + 1] == '>'){
            dst = _trim(line.substr(pos + 2));
            append = true;
        }
        else
            dst = _trim(line.substr(pos + 1));
    }

    int fd;
    if (append)
        fd = open(dst.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666); // allow append
    else
        fd = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666); // force truncation
    if (fd == -1){
        SYSCALL_FAIL("open");
        cleanup();
        return;
    }

    // Save current stdout and close it
    int saved_stdout = dup(1);
    if (saved_stdout == -1){
        SYSCALL_FAIL("dup");
        if (close(fd) == -1){
            SYSCALL_FAIL("close");
        }
        cleanup();
        return;
    }

    // Redirect stdout to file
    if (dup2(fd, 1) == -1){
        SYSCALL_FAIL("dup2");
        if (close(fd) == -1){
            SYSCALL_FAIL("close");
        }
        cleanup();
        return;
    }
    if (close(fd) == -1){
        SYSCALL_FAIL("close");
    }

    // run the command using the smash create command.
    Command* cmd = SmallShell::getInstance().CreateCommand(src.c_str());
    cmd->execute();
    delete cmd;
    
    if (dup2(saved_stdout, 1) == -1){
        SYSCALL_FAIL("dup2");
    }
    if (close(saved_stdout) == -1){
        SYSCALL_FAIL("close");
    }

    cleanup();
}

void NetInfoCommand::execute(){
    prepare();

    if (count == 1){
        std::cerr << "smash error: netinfo: interface not specified" << std::endl;
        return;
    }
    if (!interface_exists(std::string(args[1]))){
        std::cerr << "smash error: netinfo: interface " <<  args[1] << " does not exist " << std::endl;
        return;
    }
    std::string ip = get_ip(args[1]);
    std::cout << "IP Address: " << ((ip.compare("") == 0) ? "" : ip) << std::endl;

    std::string subnetmask = get_subnet_mask(args[1]);
    std::cout << "Subnet Mask: " << ((subnetmask.compare("") == 0) ? "" : subnetmask) << std::endl;

    // print gateway if exists else None
    std::string gw = get_default_gateway();
    std::cout << "Default Gateway: " << ((gw.empty()) ? "" : gw) << std::endl;

    // print DNS servers else None
    std::cout << "DNS Servers: ";
    auto servers = get_dns_servers();
    if (servers.empty()){
        std::cout << "" << std::endl;
    }
    else{
        std::cout << servers.front();
        servers.erase(servers.begin());

        for (auto& dns : servers){
            std::cout << ", " << dns;
        }
        std::cout << std::endl;
    }

    cleanup();
}

std::set<std::pair<dev_t, ino_t>> seen_inodes;

int get_size_recursive(const std::string& path){
    struct stat info;

    // Don't follow symlinks
    if (lstat(path.c_str(), &info) == -1){
        SYSCALL_FAIL("lstat");
        return -1;
    }

    // Check for symlink
    if (S_ISLNK(info.st_mode)){
        // Just count the symlink file itself (not its target)
        return info.st_blocks;
    }

    // Check for regular files
    if (S_ISREG(info.st_mode)){
        auto inode_key = std::make_pair(info.st_dev, info.st_ino);
        if (seen_inodes.count(inode_key)){
            return 0;
        }
        seen_inodes.insert(inode_key);
        return info.st_blocks;
    }

    if (S_ISDIR(info.st_mode)){
        int total_blocks = info.st_blocks;
        auto entries = list_directory(path);

        for (const auto& name : entries){
            if (name == "." || name == "..")
                continue;

            std::string full_path = path + "/" + name;
            int sub_size = get_size_recursive(full_path);
            if (sub_size == -1)
                return -1;

            total_blocks += sub_size;
        }
        return total_blocks;
    }

    return info.st_blocks;
}

void DuCommand::execute() {
    this->prepare();

    if (count > 2) {
        std::cerr << "smash error: du: too many arguments" << std::endl;
        this->cleanup();
        return;
    }

    std::string target_dir = (count == 1) ? "." : std::string(args[1]);

    struct stat info;
    if (stat(target_dir.c_str(), &info) == -1 || !S_ISDIR(info.st_mode)){
        std::cerr << "smash error: du: directory " << target_dir << " does not exist" << std::endl;
        this->cleanup();
        return;
    }

    int total_blocks = get_size_recursive(target_dir);
    if (total_blocks != -1) {
        std::cout << "Total disk usage: " << (total_blocks * 512 + 1023) / 1024 << " KB" << std::endl;
    }

    this->cleanup();
}

