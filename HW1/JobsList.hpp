#pragma once

#include <map>
#include <vector>

class Command;

class JobsList {
public:
    class JobEntry{
    private:
        // TODO: Add your data members
        pid_t pid;
        int job_id;  // job_id will start from 1 and increase each time we add a job
        std::string cmd;
    public:
        JobEntry() = default;
        JobEntry(pid_t pid, int jid, std::string cmd) : pid(pid), job_id(jid), cmd(cmd) {}

        pid_t getJobPid() const{ return pid; }
        int getJobID() const{ return job_id; }
        std::string getCommand() const{ return cmd; }
    };

private:
    static bool init_flag;

    std::map<int, JobEntry> jobs; // jobs by job-ID
    std::vector<int> jids;
    int job_cnt = 0;

    JobsList() = default;
public:
    virtual ~JobsList() = default;

    JobsList(JobsList const&) = delete;         // disable copy ctor

    void operator=(JobsList const&) = delete;   // disable = operator

    static JobsList& getInstance(){
        static JobsList instance; // Guaranteed to be destroyed.

        // Instantiated on first use.
        if (!init_flag){
            init_flag = true;
        }
        return instance;
    }

    // TODO: I assumed that addJob fork the process itself
    /**
     * Runs the command in the background and add it to the jobs list
     * @param cmd pointer to the command
     * @param no_bg_cmd the command line the user entered without the &
     * @param cmd_line the line the user entered
     */
    void addJob(Command* cmd, const std::string& no_bg_cmd, const std::string& cmd_line);

    /**
    * prints to os for each job: `<l_str><pid / jid><r_str> <command + input>`
    */
    void printJobsList(std::string l_str, std::string r_str, bool use_pid = true);

    /**
    * Kills all jobs and prints: `smash: sending SIGKILL signal to <N> jobs:`
    * and then for each job: `<pid>: <command + input>&`
    */
    void killAllJobs(bool silent);

    /**
     * Clears the list from the finished jobs
     */
    void removeFinishedJobs();

    //TODO: return nullptr if couldnt find job id
    /**
     * Returns a job by its ID
     * @param jobId the job ID of the wanted job
     */
    JobEntry* getJobById(int jobId);

    /**
     * Remove a jobs by its job ID
     */
    // void removeJobById(int jobId);

    /**
     * 
     */
    // JobEntry* getLastJob(int* lastJobId);

    // JobEntry *getLastStoppedJob(int *jobId); 

    // TODO: Add extra methods or modify exisitng ones as needed
    /**
     * @return true if jobs list is empty
     */
    bool isJobsListEmpty(); // V

    /**
     * @return the job with the maximal job_id
     */
    int getMaxJobID(); // V
};