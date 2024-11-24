#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <chrono>
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and classes here

struct submission_data_t
{
    std :: vector<int> tokens ;
    int time_stamp ;
    std :: shared_ptr<submission_t> addr ;
    int no_matches ; 
    bool isPlagged ;

    public:
    submission_data_t(void) ;
    submission_data_t(std :: vector<int>& tokens, int& time_stamp, std :: shared_ptr<submission_t>& addr) ;
    ~submission_data_t(void) ;
};


class plagiarism_checker_t 
{
    // You should NOT modify the public interface of this class.
public:
    plagiarism_checker_t(void);
    plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> 
                            __submissions);
    ~plagiarism_checker_t(void);
    void add_submission(std::shared_ptr<submission_t> __submission);

protected:
    // vector of submissions
    std :: vector<submission_data_t> DataBase ;
    std::chrono::time_point<std::chrono::steady_clock> start_time;

    // function to check plag
    void plagiarism_check(int index) ;
    bool pair_wise_plag(const std :: unordered_map<long long, int>& map_curr, int curr, int prev, int& no_matches) ;
    void plag_this_submission(std :: shared_ptr<submission_t>& addr) ;
    void hash_this_submission(int index, std :: unordered_map<long long, int>& hashed_map) ;

    // Number of original files
    int numOriginal ;

    // thread for background processing
    std :: thread background_thread ;

    // mutex for thread safe access
    std :: mutex queue_mutex ;

    // conditional variable for thread notifications
    std :: condition_variable queue_condition ;

    // Queue of submission indexes to be processed
    std :: queue<int> submission_queue ;

    // Flag to stop the thread
    bool stop_thread ;

    // Background thread function
    void process_queue(void) ; 
};
