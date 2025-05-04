#include <sys/types.h>
#include <pwd.h>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sys/wait.h>
#include <sstream>
#include <fcntl.h>

#include "SpecialCommands.hpp"
#include "parsing_utils.hpp"
#include "net_utils.hpp"

#define MAX_ARGS 20

void WhoamiCommand::execute() {
    this->prepare();
    uid_t uid = getuid();  // current user ID

    int fd = open("/etc/passwd", O_RDONLY);
    if (fd == -1) {
        perror("smash error: open failed");
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
        perror("smash error: read failed");
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

std::string joinArgs(char** args) {
    std::string result;
    for (int i = 0; args[i] != nullptr; ++i) {
        if (i > 0) {
            result += " ";
        }
        result += args[i];
    }
    return result;
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
        perror("smash error: pipe failed");
        this->cleanup();
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == -1){
        perror("smash error: fork failed");
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
        perror("smash error: fork failed");
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
        perror("smash error: waitpid failed");
    if (waitpid(pid2, nullptr, 0) == -1)
        perror("smash error: waitpid failed");

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

    cleanup();
}