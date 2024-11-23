#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

#define MIN_LENGTH 15
#define TIME_IN_MILLI 1000
#define MAX_NUMBER_OF_FILES 100
// auto start = std :: chrono :: high_resolution_clock :: now() ;
// auto stop = std :: chrono :: high_resolution_clock :: now() ;
// auto duration = std :: chrono :: duration_cast<std :: chrono :: milliseconds>(stop - start) ;
// std :: cout<<"Time taken for exact match : "<<duration.count()<<endl ;

// Implement submission_data_t methods
submission_data_t :: submission_data_t(void) : time_stamp(0), addr(nullptr), no_matches(0), isPlagged(false) { }

submission_data_t :: submission_data_t(std :: vector<int>& tokens, int& time_stamp, std :: shared_ptr<submission_t>& addr)
{
    this->tokens = tokens ;
    this->time_stamp = time_stamp ;
    this->addr  = addr ;
    this->no_matches = 0 ;
    this->isPlagged = false ;
}

submission_data_t :: ~submission_data_t(void) { }

// Implement plagiarism_checker_t  methods

plagiarism_checker_t :: plagiarism_checker_t(void) : numOriginal(0), stop_thread(false)
{
    this->start_time = std :: chrono :: steady_clock :: now() ;
    this->background_thread = std :: thread(&plagiarism_checker_t :: process_queue, this) ;
    this->DataBase.reserve(MAX_NUMBER_OF_FILES) ;
}

plagiarism_checker_t :: plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions) : numOriginal(__submissions.size()), stop_thread(false)
{
    this->background_thread = std :: thread(&plagiarism_checker_t :: process_queue, this) ;
    this->DataBase.reserve(MAX_NUMBER_OF_FILES) ;

    submission_data_t temp_submission ;
    for(const std :: shared_ptr<submission_t>& __submission : __submissions)
    {
        tokenizer_t tokenizer(__submission->codefile) ;
        temp_submission.tokens = tokenizer.get_tokens() ;
        temp_submission.time_stamp = 0;
        temp_submission.addr = __submission ;
        this->DataBase.push_back(temp_submission) ;
    }
    
    // Note the start time to calculate the time stamps of future submissions
    this->start_time = std :: chrono :: steady_clock :: now() ; 
}

plagiarism_checker_t :: ~plagiarism_checker_t()
{
    {
        std :: lock_guard<std :: mutex> lock(queue_mutex) ;
        stop_thread = true ;
    }
    queue_condition.notify_all() ;
    if(background_thread.joinable())
    {
        background_thread.join() ;
    }
}

void plagiarism_checker_t :: add_submission(std::shared_ptr<submission_t> __submission)
{
    // Record the time of submission
    auto now = std :: chrono :: steady_clock :: now() ;
    auto duration = std :: chrono :: duration_cast<std :: chrono :: milliseconds>(now - this->start_time) ;
    int time_stamp = duration.count() ;
    // std :: cerr<<__submission->codefile<<"  =  "<<time_stamp<<std :: endl ;
    
    // Prepare the temp_sub, leave the tokenizing for later
    submission_data_t temp_sub ;
    temp_sub.addr = __submission ;
    temp_sub.time_stamp = time_stamp ;

    int index ;
    {
        std :: lock_guard<std :: mutex> lock(queue_mutex) ;
        index = DataBase.size() ;
        DataBase.push_back(temp_sub) ;
        submission_queue.push(index) ;
    }

    this->queue_condition.notify_one() ;

    return ;
}

void plagiarism_checker_t :: process_queue(void)
{
    while(true)
    {
        int index ;
        {
            std :: unique_lock<std :: mutex> lock(queue_mutex) ;
            queue_condition.wait(lock, [this](){
                return !submission_queue.empty() || stop_thread ;
            });

            if(stop_thread && submission_queue.empty())
            {
                break ;
            }

            index = submission_queue.front() ;
            submission_queue.pop() ;
        }

        this->plagiarism_check(index) ;
    }
    return ;
}

void plagiarism_checker_t :: plagiarism_check(int index)
{
    tokenizer_t tokenizer(DataBase[index].addr->codefile) ;
    DataBase[index].tokens = tokenizer.get_tokens() ;
    std :: vector<int>& sub = DataBase[index].tokens ;
    int no_matches = 0;
    int time_diff ;

    for(int i = index-1; i>=0; i--)
    {
        std :: vector<int>& prev_sub = DataBase[i].tokens ;
        time_diff = DataBase[index].time_stamp - DataBase[i].time_stamp ;

        // Files more than 1 second prior can be ignored if our file is already plagged
        if(DataBase[index].isPlagged && (time_diff>TIME_IN_MILLI || i<numOriginal))
        {
            break ;
        }

        // check plag between prev_sub and sub.
        int num_matches = 0;
        bool plagged = this->pair_wise_plag(sub, prev_sub, num_matches) ;
        no_matches+=num_matches ;

        if(plagged)
        {
            if(time_diff <=TIME_IN_MILLI && !DataBase[i].isPlagged && i>=numOriginal)
            {
                this->plag_this_submission(DataBase[i].addr) ;
                DataBase[i].isPlagged = true ;
            }
            if(!DataBase[index].isPlagged)
            {
                this->plag_this_submission(DataBase[index].addr) ;
                DataBase[index].isPlagged = true ;   
            }
        }

        // check for patch work plagiarism
        if(time_diff<=TIME_IN_MILLI  && !DataBase[i].isPlagged && i>=numOriginal)
        {
            DataBase[i].no_matches+=num_matches ;
            if(DataBase[i].no_matches>=20)
            {
                this->plag_this_submission(DataBase[i].addr) ;
                DataBase[i].isPlagged = true ;
            }
        }
    }

    // update no_matches of present submission
    DataBase[index].no_matches = no_matches ;

    // return 
    return ;
}

void plagiarism_checker_t :: plag_this_submission(std :: shared_ptr<submission_t>& addr)
{
    // check if submissions are valid and call the plagiarism flag
    if(addr->student!=nullptr)
    {
        addr->student->flag_student(addr) ;
    }

    if(addr->professor!=nullptr)
    {
        addr->professor->flag_professor(addr) ;
    }
    return ;
}

bool plagiarism_checker_t :: pair_wise_plag(const std :: vector<int>& sub1, const std :: vector<int>& sub2, int& no_matches)
{
    // sizes of the vectors
    int m = sub1.size() ;
    int n = sub2.size() ;
    
    // Initialize the total exact matched length
    int total_matched_length = 0 ;

    // temporary length holder  
    int temp_length ;

    // So as to prevent overlapping ie skipping them we use
    // Ordered Set to store already matched segments as pairs (start, end) in sub2
    std::set<std::pair<int, int>> matches;

    for(int i = 0; i<=m-MIN_LENGTH; i++)
    {
        // start checking for match from i in sub1;
        // lets use the iterator it to make sure we skip matched patterns in sub2
        auto it = matches.begin();
        for(int j = 0; j<=n-MIN_LENGTH; j++)
        {
            if(it != matches.end() && j > (it->first-MIN_LENGTH))
            {
                // A match of >=MIN_LENGTH isn't possible hence skip it 
                j = it->second ;
                ++it;
                continue;
            }
            if(sub1[i] != sub2[j]) { continue ; }

            // Find the degree of match starting from here
            temp_length = 0;
            int k1 = i ;
            int k2 = j ;
            while(k1<m && k2<n && sub1[k1] == sub2[k2])
            {
                // In case we reach a previously matched region break
                if (it != matches.end() && k2 >= it->first) {
                    break;
                }
                k1++; k2++ ;
                temp_length+=1 ;
            }

            // Check if the match is valid
            if(temp_length>=75) { return true ; }
            if(temp_length >= MIN_LENGTH)
            {
                // match found
                total_matched_length+=temp_length ;
                no_matches+=(temp_length%15) ;
                matches.emplace(j, k2-1) ;
                i = k1-1 ;
                break ;
            }
        }  
        if(total_matched_length>=130 ) { return true ; }
    }
    // Not plagged
    return false;
}
