#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

#define MIN_LENGTH 15

// Implement submission_data_t methods
submission_data_t :: submission_data_t(void) : time_stamp(0), addr(nullptr), no_matches(0), isPlagged(false) { }

submission_data_t :: submission_data_t(std :: vector<int>& tokens, int& time_stamp, std :: shared_ptr<submission_t> addr)
{
    this->tokens = tokens ;
    this->time_stamp = time_stamp ;
    this->addr  = addr ;
    this->no_matches = 0 ;
    this->isPlagged = false ;
}

submission_data_t :: ~submission_data_t(void) { }

// Implement plagiarism_checker_t  methods

plagiarism_checker_t :: plagiarism_checker_t(void) {}

plagiarism_checker_t :: plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions)
{
    this->start_time = std :: chrono :: steady_clock :: now() ; 
    submission_data_t temp_submission ;
    for(const std :: shared_ptr<submission_t>& __submission : __submissions)
    {
        tokenizer_t tokenizer(__submission->codefile) ;
        temp_submission.tokens = tokenizer.get_tokens() ;
        temp_submission.time_stamp = 0;
        temp_submission.addr = __submission ;
        this->DataBase.push_back(temp_submission) ;
    }
}

void plagiarism_checker_t :: add_submission(std::shared_ptr<submission_t> __submission)
{
    submission_data_t temp_sub ;
    auto now = std :: chrono :: steady_clock :: now() ;
    auto duration = std :: chrono :: duration_cast<std :: chrono :: milliseconds>(now-start_time) ;
    int time_stamp = duration.count() ;
    
    tokenizer_t tokenizer(__submission->codefile) ;
    temp_sub.tokens = tokenizer.get_tokens() ;
    temp_sub.addr = __submission ;
    temp_sub.time_stamp = time_stamp ;
    DataBase.push_back(temp_sub) ;

    // handling plag check.
    this->plagiarism_check(DataBase.size()-1) ; 

    return ;
}

void plagiarism_checker_t :: plagiarism_check(int index)
{
    std :: vector<int>& sub = DataBase[index].tokens ;
    int no_matches = 0;
    int time_diff ;
    for(int i = index-1; i>=0; i--)
    {
        std :: vector<int>& prev_sub = DataBase[i].tokens ;
        time_diff = DataBase[index].time_stamp - DataBase[i].time_stamp ;
        if(DataBase[index].isPlagged && time_diff>1000)
        {
            break ;
        }
        // check plag between prev_sub and sub.
        int num_matches = 0;
        bool plagged = this->pair_wise_plag(sub, prev_sub, num_matches) ;
        no_matches+=num_matches ;
        if(plagged)
        {
            this->plag_this_submission(DataBase[index].addr) ;
            DataBase[index].isPlagged = true ;
            if(time_diff <=1000 && !DataBase[i].isPlagged )
            {
                this->plag_this_submission(DataBase[i].addr) ;
                DataBase[i].isPlagged = true ;
            }
        }
        // check for patch work plagiarism
        if(time_diff<=1000)
        {
            DataBase[i].no_matches+=num_matches ;
            if(DataBase[i].no_matches>=20 && !DataBase[i].isPlagged)
            {
                this->plag_this_submission(DataBase[i].addr) ;
                DataBase[i].isPlagged = true ;
            }
        }
    }
    DataBase[index].no_matches = no_matches ;
    return ;
}

void plagiarism_checker_t :: plag_this_submission(std :: shared_ptr<submission_t>& addr)
{
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

plagiarism_checker_t :: ~plagiarism_checker_t() {}

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
                no_matches++ ;
                matches.emplace(j, k2-1) ;
                i = k1-1 ;
                break ;
            }
        }  
        if(total_matched_length>=150) { return true ; }
    }
    // Not plagged
    return false;
}
