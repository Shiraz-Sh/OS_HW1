
#include "JobsList.h"
#include "Commands.h"
#include <signal.h>
#include <sys/types.h>

JobsList::~JobsList(){
    // TODO: clean up if needed
}

bool JobsList::isJobsListEmpty(){
    for (const auto& [job_id, entry] : jobs)
        if (kill(entry.getJobPid(), 0) != ESRCH) //check if a job didn't finish
            return false;
    return true;
}

JobsList::JobEntry* JobsList::getMaxJobID() {
    int max_id = 0;
    JobEntry* res = nullptr;

    for (const auto& [job_id, entry] : jobs)
        if (kill(entry.getJobPid(), 0) != ESRCH && max_id < job_id){ //check if a job didn't finish
            max_id = job_id;
            res = &jobs[job_id];
        }
    return res;
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    auto entry = jobs.find(jobId);

    // if the entry doesn't exist or the job in the entry already finished
    if (entry == jobs.end() || kill(entry->second.getJobPid(), 0) == ESRCH)
        return nullptr;
    return &entry->second;
}

void JobsList::addJob(Command *cmd, bool isStopped) {
    job_cnt++;
    pid_t pid = fork();

    if (pid == 0){
        // child proc
    }
    else{
        // parent proc
    }
}