#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

#include "JobsList.hpp"
#include "Commands.hpp"

bool JobsList::init_flag = false;

JobsList::~JobsList(){
    // TODO: clean up if needed
}

bool JobsList::isJobsListEmpty(){
    removeFinishedJobs();
    return jobs.empty();
}

JobsList::JobEntry* JobsList::getMaxJobID() {
    removeFinishedJobs();
    return &jobs[jids.back()];
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    auto entry = jobs.find(jobId);

    // if the entry doesn't exist or the job in the entry already finished
    if (entry == jobs.end() || waitpid(entry->second.getJobPid(), nullptr, WNOHANG) != 0)
        return nullptr;
    return &entry->second;
}

void JobsList::addJob(Command* cmd, std::string cmd_line, bool isStopped){
    pid_t pid = fork();
    if (pid < 0){
        SYSCALL_FAIL("fork");
        return;
    }
    else if (pid == 0){
        setpgrp();

        cmd->execute();
        exit(0);
    }
    
    job_cnt++;
    jobs[job_cnt] = JobEntry(pid, job_cnt, cmd_line);
    jids.push_back(job_cnt);
}

void JobsList::killAllJobs(){
    std::cout << "smash: sending SIGKILL signal to <N> jobs:" << std::endl;
    printJobsList("", ":");
    for (const auto& pair : jobs){
        kill(pair.second.getJobPid(), SIGKILL);
    }
    jids.clear();
    jobs.clear();
}

void JobsList::printJobsList(std::string l_str, std::string r_str, bool use_pid){
    removeFinishedJobs();
    for (const auto& pair : jobs){
        if (use_pid)
            std::cout << l_str << pair.second.getJobPid() << r_str << " " << pair.second.getCommand() << std::endl;
        else
            std::cout << l_str << pair.first << r_str << " " << pair.second.getCommand() << std::endl;
    }
}

void JobsList::removeFinishedJobs(){
    std::vector<int> jids_updated;
    for (auto& jid : jids){
        
        if (waitpid(jobs[jid].getJobPid(), nullptr, WNOHANG) != 0){ //check if a job didn't finish
            jobs.erase(jid);
        }
        else{
            jids_updated.push_back(jid);
        }
    }
    jids = jids_updated;
}
        