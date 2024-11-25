#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

#define MIN_LENGTH 15
#define TIME_IN_MILLI 1000
#define MAX_NUMBER_OF_FILES 100
#define BASE 31
#define MOD 1000000007

// submission_data_t methods

// Constructors
submission_data_t :: submission_data_t(void)
    : time_stamp(0), addr(nullptr), no_matches(0), isPlagged(false) {}

submission_data_t :: submission_data_t(std :: vector<int>& tokens, int& time_stamp, std::shared_ptr<submission_t>& addr)
{
    this->tokens = tokens;
    this->time_stamp = time_stamp;
    this->addr = addr;
    this->no_matches = 0;
    this->isPlagged = false;
}

// Destructor
submission_data_t::~submission_data_t(void) {}

// plagiarism_checker_t methods

// Constructors
plagiarism_checker_t::plagiarism_checker_t(void) : numOriginal(0), stop_thread(false)
{
    this->start_time = std::chrono::steady_clock::now();
    this->background_thread = std::thread(&plagiarism_checker_t::process_queue, this);

    // reserve space to prevent constant reallocation
    this->DataBase.reserve(MAX_NUMBER_OF_FILES);
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions) : numOriginal(__submissions.size()), stop_thread(false)
{
    this->background_thread = std::thread(&plagiarism_checker_t::process_queue, this);
    this->DataBase.reserve(MAX_NUMBER_OF_FILES);

    submission_data_t temp_submission;
    for (const std::shared_ptr<submission_t>& __submission : __submissions)
    {
        tokenizer_t tokenizer(__submission->codefile);
        temp_submission.tokens = tokenizer.get_tokens();
        temp_submission.time_stamp = 0 ;
        temp_submission.addr = __submission ;

        this->DataBase.push_back(temp_submission) ;
    }

    // Note the start time to calculate the time stamps of future submissions
    this->start_time = std::chrono::steady_clock::now() ;
}

// Destructor
plagiarism_checker_t::~plagiarism_checker_t()
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex) ;
        stop_thread = true ;
    }

    queue_condition.notify_all() ;

    // Complete all background processes before terminating
    if (background_thread.joinable())
    {
        background_thread.join() ;
    }
}

// Hashing function for a submission
void plagiarism_checker_t::hash_this_submission(int index, std :: unordered_map<long long, int>& hashed_map)
{
    const std::vector<int>& tokens = DataBase[index].tokens ;

    long long hash = 0, power = 1 ;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        hash = (hash * BASE + tokens[i]) % MOD ;
        if (i >= MIN_LENGTH - 1)
        {
            hashed_map[hash] = i - MIN_LENGTH + 1 ;
            hash = (hash - tokens[i - MIN_LENGTH + 1] * power % MOD + MOD) % MOD ;
        }
        if (i < MIN_LENGTH - 1)
        {
            power = (power * BASE) % MOD ;
        }
    }
}

// Add Submission function
void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission)
{
    // Record the time of submission
    auto now = std::chrono::steady_clock::now() ;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->start_time) ;
    int time_stamp = duration.count() ;
    
    // prepare the new submission to store in data base
    submission_data_t temp_sub ;
    temp_sub.addr = __submission ;
    temp_sub.time_stamp = time_stamp ;

    int index;
    {
        // Use mutex 
        std::lock_guard<std::mutex> lock(queue_mutex) ;
        index = DataBase.size() ;
        DataBase.push_back(temp_sub) ;
        submission_queue.push(index) ; 
    }
    
    // Notify the thread to continue processing
    this->queue_condition.notify_one() ;

    return ;
}

// The function that runs on the thread
// Process elements in queue one by one when notified
void plagiarism_checker_t::process_queue(void)
{
    while (true)
    {
        int index;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_condition.wait(lock, [this]() {
                return !submission_queue.empty() || stop_thread;
            });

            if (stop_thread && submission_queue.empty())
            {
                break;
            }

            index = submission_queue.front();
            submission_queue.pop();
        }

        this->plagiarism_check(index);
    }
    return ;
}

// Plagiarism check function
void plagiarism_checker_t::plagiarism_check(int index)
{
    tokenizer_t tokenizer(DataBase[index].addr->codefile) ;
    DataBase[index].tokens = tokenizer.get_tokens() ;
    
    // hash the tokens in 15 chunks to the resp start index
    std :: unordered_map<long long, int> hashed_map ;
    this->hash_this_submission(index, hashed_map) ; 
    int no_matches = 0 ;
    int time_diff ;

    for (int i = index - 1; i >= 0; i--)
    {
        time_diff = DataBase[index].time_stamp - DataBase[i].time_stamp ;
        
        if (DataBase[index].isPlagged && (time_diff > TIME_IN_MILLI || i < numOriginal))
        {
            // No more plagiarism to detect
            break;
        }

        int num_matches = 0;

        // Check the pair wise plagiarism between the files
        bool plagged = this->pair_wise_plag(hashed_map, index, i, num_matches);

        // Update the no_matches
        no_matches += num_matches;

        if (plagged)
        {
            // Plag as required
            if (time_diff <= TIME_IN_MILLI && !DataBase[i].isPlagged && i >= numOriginal)
            {
                this->plag_this_submission(DataBase[i].addr);
                DataBase[i].isPlagged = true;
            }
            if (!DataBase[index].isPlagged)
            {
                this->plag_this_submission(DataBase[index].addr);
                DataBase[index].isPlagged = true;
            }
        }
        
        // Patch work plagiarism on previous file
        if (time_diff <= TIME_IN_MILLI && !DataBase[i].isPlagged && i >= numOriginal)
        {
            DataBase[i].no_matches += num_matches;
            if (DataBase[i].no_matches >= 20)
            {
                this->plag_this_submission(DataBase[i].addr);
                DataBase[i].isPlagged = true;
            }
        }
        
        // Patch work plagiarism on current file
        if(!DataBase[index].isPlagged && no_matches>=20)
        {
            this->plag_this_submission(DataBase[index].addr) ;
            DataBase[index].isPlagged = true ;
        }

    }

    DataBase[index].no_matches = no_matches;

    return ;
}

// Plag the valid submissions by calling flag_student and flag_professor
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

bool plagiarism_checker_t::pair_wise_plag(const std :: unordered_map<long long, int>& map_curr, int curr, int prev, int& no_matches) 
{
    // curr_map is the hash map for the current submission 
    // prev is referencing old submission and curr for current submission
    const std :: vector<int>& tokens_prev = DataBase[prev].tokens ;
    const std :: vector<int>& tokens_curr = DataBase[curr].tokens ;

    int m = tokens_prev.size() ;
    int n = tokens_curr.size() ;

    // Variables for rolling hash
    long long hash = 0, power = 1;
    int total_matched_length = 0;
    int temp_length ;

    // To store matched ranges (start, end) in current submission
    std::set<std::pair<int, int>> matches; 

    // Compute rolling hash for prev submission
    for (size_t i = 0; i < m; i++) 
    {
        // Update the hash
        hash = (hash * BASE + tokens_prev[i]) % MOD; 

        // Precompute the power term for the first window
        if (i < MIN_LENGTH - 1) {
            power = (power * BASE) % MOD;
            continue;
        }

        // Only start checking after MIN_LENGTH tokens
        // Return if match not found in previous submission's hash map
        if(map_curr.find(hash) == map_curr.end())
        {
            // Update the hash and return 
            hash = (hash - tokens_prev[i - MIN_LENGTH + 1] * power % MOD + MOD) % MOD;
            continue;
        }

        // We found a match. Now continue matching beyond to not miss any matches
        int k1 = i + 1 ;
        int j = map_curr.at(hash) ;
        int k2 = j + 15 ;

        // Find the first pair with the second element > k2
        auto it = std::find_if(matches.begin(), matches.end(), [j](const std::pair<int, int>& p) {
            return p.second >= j;
        });
        if(it!=matches.end() && j>=it->first-14)
        {
            // Overlap detected => Skip the chunk
            hash = (hash - tokens_prev[i - MIN_LENGTH + 1] * power % MOD + MOD) % MOD;
            continue;
        }

        // the present match is k1-15 to k1-1 matched with k2-15 to k2-1 ;
        // temp_length is the current matched length
        temp_length = 15 ;
        hash = (hash - tokens_prev[i - MIN_LENGTH + 1] * power % MOD + MOD) % MOD;
        while(k1<m && k2<n && tokens_prev[k1]==tokens_curr[k2])
        {
            hash = (hash * BASE + tokens_prev[k1]) % MOD; 
            if(it!=matches.end() && k2>=it->first)
            {
                break ;
            }
            hash = (hash - tokens_prev[k1 - MIN_LENGTH + 1] * power % MOD + MOD) % MOD;
            k1++ ;
            k2++ ;
            temp_length+=1 ;
        }

        // Check if match is valid
        if(temp_length>=75)
        {
            return true ;
        }

        // not more than 75
        // update matches and also i properly
        total_matched_length+=temp_length ;
        if(total_matched_length>=150) { return true ; }

        // Update no_matches that is number of 15 length patterns detected
        no_matches+=(temp_length/15) ;

        // Update matches 
        matches.emplace(j, k2-1) ;

        // Skip the matched part in prev ie update i
        for(int count = 0; count<14; count++)
        {
            if(k1>=m) { break ; }
            hash = (hash * BASE + tokens_prev[k1]) % MOD;
            hash = (hash - tokens_prev[k1 - MIN_LENGTH + 1] * power % MOD + MOD) % MOD;
            k1++;
        }
        i = k1-1 ;
    }
    return false;
}

