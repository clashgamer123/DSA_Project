#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <chrono>
#include <set>
#include <mutex>
#include <thread>
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
    submission_data_t(std :: vector<int>& tokens, int& time_stamp, std :: shared_ptr<submission_t> addr) ;
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
    bool pair_wise_plag(const std :: vector<int>& sub1, const std :: vector<int>& sub2, int& no_matches) ;
    void plag_this_submission(std :: shared_ptr<submission_t>& addr) ;
};
