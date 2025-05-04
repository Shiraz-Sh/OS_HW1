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
    close(fd);

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
        close(pipefd[0]);   // close unused read end
        close(pipefd[1]);   // close write end after dup2
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
        close(pipefd[0]);
        close(pipefd[1]);
        cmd2->execute();
        exit(0);
    }

    // parent process - only the parent comes here thanks to exit()
    close(pipefd[0]);
    close(pipefd[1]);

    if (waitpid(pid1, nullptr, 0) == -1)
        SYSCALL_FAIL("waitpid");
    if (waitpid(pid2, nullptr, 0) == -1)
        SYSCALL_FAIL("waitpid");

    delete[] cmd_line1;
    free(cmd_line2);
}

void RedirectionCommand::execute() {
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

    // parse the source command
    pid_t pid = fork();
    if (pid < 0){
        SYSCALL_FAIL("fork");
        return;
    }
    else if (pid == 0){
        // child process
        setpgrp();

        // use dst as the stdout.
        close(1);
        int fd;
        if (append)
            fd = open(dst.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666); // allow append
        else
            fd = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666); // force truncation

        if (fd == -1){
            SYSCALL_FAIL("open");
            exit(-1);
        }

        // run the command using the smash create command.
        Command* cmd = SmallShell::getInstance().CreateCommand(src.c_str());
        cmd->execute();
        close(fd);
        exit(0);
    }

    wait(nullptr);

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

    std::cout << "IP Address: " << get_ip(args[1]) << std::endl;
    std::cout << "Subnet Mask: " << get_subnet_mask(args[1]) << std::endl;

    // print gateway if exists else None
    std::string gw = get_default_gateway();
    std::cout << "Default Gateway: " << ((gw.empty()) ? "None" : gw) << std::endl;

    // print DNS servers else None
    std::cout << "DNS Servers: ";
    auto servers = get_dns_servers();
    if (servers.empty()){
        std::cout << "None" << std::endl;
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


int get_size_recursive(const std::string& path){
    struct stat info;
    int res = stat(path.c_str(), &info);
    if (res == -1){
        std::cout << "error on: " << path << std::endl;
        SYSCALL_FAIL("stat");
        return -1;
    }
    else if (res == 0 && S_ISREG(info.st_mode)){
        return (int) info.st_size;
    }
    else if (res == 0 && S_ISDIR(info.st_mode)){
        int size = 0;

        auto names = list_directory(path);
        std::cout << "dir: " << path << std::endl;
        for (auto name : names){
            if (name.compare(".") == 0 || name.compare("..") == 0){
                continue;
            }
            // std::cout << "special-file" << std::endl;
            int val = get_size_recursive(path + "/" + name);
            if (val == -1){
                return -1;
            }
            size += val;
        }
        return size;
    }
    return 0;
}

void DuCommand::execute() {
    this->prepare();

    if (count > 2){
        std::cerr << "smash error: du: too many arguments" << std::endl;
        this->cleanup();
        return;
    }
    
    struct stat info;
    int res = stat(args[1], &info);
    if (res == -1){
        SYSCALL_FAIL("stat");
        return;
    }
    else if (!(res == 0 && S_ISDIR(info.st_mode))){
        std::cerr << "smash error: du: directory " << args[1] << " does not exist" << std::endl;
        return;
    }

    
    int size = get_size_recursive(std::string(args[1]));

    if (size != -1){
        std::cout << "Total disk usage: " << size << " KB" << std::endl;
    }

    this->cleanup();
}