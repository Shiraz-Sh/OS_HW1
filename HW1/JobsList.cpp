
#include "JobsList.h"
#include "Commands.h"
#include <signal.h>
#include <sys/types.h>

JobsList::~JobsList(){
    // TODO: clean up if needed
}

bool JobsList::isJobsListEmpty(){
    for (const auto& [job_id, entry] : jobs){
        // Check if the job is still running. If stopped
        if (kill(entry.getJobPid(), 0) == ESRCH){
            continue;
        }
    }
    return true;
}

JobsList::JobEntry* JobsList::getMaxJobID() {
    //TODO: implement
    return nullptr;
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    //TODO: implement
}

void JobsList::addJob(Command *cmd, bool isStopped) {
    // TODO: implement
}