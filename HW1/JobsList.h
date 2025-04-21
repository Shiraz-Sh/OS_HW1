#pragma once

class Command;

class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
        int pid;
        int jobID;  // jobID will start from 1 and increase each time we add a job
        int* wstatus;
    public:
        int getJobPid() { return this->pid; }
        int getJobID() { return this->jobID; }
        int* getWstatus() {return this->wstatus; }
    };

    // TODO: Add your data members
public:
    JobsList();

    ~JobsList();

    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    //TODO: return nullptr if couldnt find job id
    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);

    // TODO: Add extra methods or modify exisitng ones as needed
    /**
     * @return true if jobs list is empty
     */
    bool isJobsListEmpty();

    /**
     * @return the job with the maximal jobID
     */
    JobEntry* getMaxJobID();
};