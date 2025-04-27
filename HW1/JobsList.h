#pragma once

#include <map>

class Command;

class JobsList {
public:
    class JobEntry{
    private:
        // TODO: Add your data members
        pid_t pid;
        int job_id;  // job_id will start from 1 and increase each time we add a job
        int* wstatus;
    public:
        JobEntry() = default;
        JobEntry(pid_t pid, int jid, int* status) : pid(pid), job_id(jid), wstatus(status){}
        pid_t getJobPid() const{ return this->pid; }
        int getJobID() const { return this->job_id; }
        int* getWstatus() const {return this->wstatus; } // Indicates a change where details about the child process that has ended will be stored.
    };

private:
    // TODO: Add your data members
    std::map<int, JobEntry> jobs;
    int job_cnt = 0;
public:
    JobsList() = default;

    ~JobsList();

    // TODO: I assumed that addJob fork the process itself
    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    //TODO: return nullptr if couldnt find job id
    JobEntry* getJobById(int jobId); // V

    void removeJobById(int jobId); 

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId); 

    // TODO: Add extra methods or modify exisitng ones as needed
    /**
     * @return true if jobs list is empty
     */
    bool isJobsListEmpty(); // V

    /**
     * @return the job with the maximal job_id
     */
    JobEntry* getMaxJobID(); // V
};