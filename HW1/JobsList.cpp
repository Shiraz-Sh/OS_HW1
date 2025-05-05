#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

#include "JobsList.hpp"
#include "Commands.hpp"

bool JobsList::init_flag = false;

bool JobsList::isJobsListEmpty(){
    removeFinishedJobs();
    return jobs.empty();
}

int JobsList::getMaxJobID() {
    removeFinishedJobs();
    if (isJobsListEmpty()){
        return 0;
    }
    return jobs[jids.back()].getJobID();
}

JobsList::JobEntry* JobsList::getJobById(int jobId){
    removeFinishedJobs();
    auto entry = jobs.find(jobId);

    // if the entry doesn't exist or the job in the entry already finished
    if (entry == jobs.end())
        return nullptr;
    return &entry->second;
}

void JobsList::addJob(Command* cmd, std::string cmd_line, bool isStopped){
    removeFinishedJobs();
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

    job_cnt = getMaxJobID() + 1;
    jobs[job_cnt] = JobEntry(pid, job_cnt, cmd_line);
    jids.push_back(job_cnt);
}

void JobsList::killAllJobs(bool silent){
    removeFinishedJobs();

    if (!silent){
        std::cout << "smash: sending SIGKILL signal to " << jobs.size() << " jobs:" << std::endl;
        printJobsList("", ":");
    }
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
        if (waitpid(jobs[jid].getJobPid(), nullptr, WNOHANG) == 0){ //check if a job didn't finish
            jobs.erase(jid);
        }
        else{
            jids_updated.push_back(jid);
        }
    }
    jids = jids_updated;
}
