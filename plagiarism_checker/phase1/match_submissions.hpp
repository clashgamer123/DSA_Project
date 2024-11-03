#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <fstream>
#include <chrono>
#include <set>
// -----------------------------------------------------------------------------

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

// Function to find total exact matched length
int ExactMatchLength(const std :: vector<int>& sub1, const std :: vector<int>& sub2, int& min_length)
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

    for(int i = 0; i<=m-min_length; i++)
    {
        // start checking for match from i in sub1;
        // lets use the iterator it to make sure we skip matched patterns in sub2
        auto it = matches.begin();
        for(int j = 0; j<=n-min_length; j++)
        {
            if(it != matches.end() && j > (it->first-min_length))
            {
                // A match of >=min_length isn't possible hence skip it 
                j = it->second ;
                ++it;
                continue;
            }
            if(sub1[i]!=sub2[j]) { continue ; }

            // Find the degree of match starting from here
            temp_length = 0;
            int k1 = i ;
            int k2 = j;
            while(k1<m && k2<n && sub1[k1] == sub2[k2])
            {
                if (it != matches.end() && k2 >= it->first) {
                    break;
                }
                k1++; k2++ ;
                temp_length+=1 ;
            }

            // Check if the match is valid
            if(temp_length>=min_length)
            {
                // match found
                total_matched_length+=temp_length ;
                matches.emplace(j, k2-1) ;
                i = k1-1 ;
                break ;
            }
        }  
    }
    // return the total exact matched length
    return total_matched_length;
}

void ComputeLCS(const std::vector<int>& sub1, const std::vector<int>& sub2, 
                std::vector<std::vector<int>>& LCS, 
                std::vector<int>& CommonSeq_1, std::vector<int>& CommonSeq_2)
{
    int n = sub1.size();
    int m = sub2.size();
    
    // Compute the LCS lengths using a dp table using recursive relations
    // We are using 1 indexing
    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            if (sub1[i - 1] == sub2[j - 1])
            {
                LCS[i][j] = 1 + LCS[i - 1][j - 1];
            }
            else
            {
                LCS[i][j] = std::max(LCS[i - 1][j], LCS[i][j - 1]);
            }
        }
    }
    
    // Backtrack to find the matching indices for the LCS
    int i = n, j = m;
    while (i > 0 && j > 0)
    {
        // If elements match, add indices to the sequences
        if (sub1[i - 1] == sub2[j - 1])
        {
            CommonSeq_1.push_back(i); // 1-indexed position in sub1
            CommonSeq_2.push_back(j); // 1-indexed position in sub2
            i--;
            j--;
        }
        // Move in the direction of the greater value in LCS table
        else if (LCS[i - 1][j] >= LCS[i][j - 1])
        {
            i--;
        }
        else
        {
            j--;
        }
    }

    // Since we collected indices in reverse order, reverse the vectors to get the correct sequence
    std::reverse(CommonSeq_1.begin(), CommonSeq_1.end());
    std::reverse(CommonSeq_2.begin(), CommonSeq_2.end());

    return ;
}


void ComputeApproximateMatches(const std :: vector<int>& sub1, const std :: vector<int>& sub2, const std :: vector<int>& CommonSeq_1, 
                               const std :: vector<int>& CommonSeq_2, int& longest_approx_length, int& start_index_1, int& start_index_2)
{
    // Temporary length and ratio holders
    int length ;
    double ratio ;
    int n = CommonSeq_1.size() ;

    for(int i = 0; i<n; i++)
    {
        for(int j = n-1; j>=i; j--)
        {
            if(CommonSeq_1[j]-CommonSeq_1[i]+1<30) {break ; }
            if(CommonSeq_2[j]-CommonSeq_2[i]+1<30) {break ; }
            length = std :: max(CommonSeq_1[j] - CommonSeq_1[i] + 1, CommonSeq_2[j] - CommonSeq_2[i] + 1) ;
            ratio = static_cast<double>(j-i+1)/(length) ;
            if(ratio<0.8) {continue ; }
            if(length>longest_approx_length)
            {
                longest_approx_length = length ;
                start_index_1 = CommonSeq_1[i]-1 ;
                start_index_2 = CommonSeq_2[i]-1 ;
            }
        }
    }
    return ;
}

std::array<int, 5> match_submissions(std::vector<int> &sub1, std::vector<int> &sub2)
{
    auto start = std :: chrono :: high_resolution_clock :: now() ;

    // sub1 and sub2 are the 2 submissions
    int m = sub1.size() ;
    int n = sub2.size() ;
    
    // the parameters to find
    int flag = 0;
    int total_exact_matched_length = 0;
    int longest_approx_length = 0;
    int start_index_1 = 0;
    int start_index_2 = 0;

    // Exact matches first. Use the function defined
    int min_length = 10;
    total_exact_matched_length = ExactMatchLength(sub1, sub2, min_length) ;

    // Fill the dp table of LCS and comppute both sequences
    std :: vector<std :: vector<int>> LCS(m+1, std :: vector<int>(n+1, 0)) ;
    std :: vector<int> CommonSeq_1;
    std :: vector<int> CommonSeq_2;
    ComputeLCS(sub1, sub2, LCS, CommonSeq_1, CommonSeq_2);

    // Compute the longest approx match and start indices
    ComputeApproximateMatches(sub1, sub2, CommonSeq_1, CommonSeq_2, longest_approx_length, start_index_1, start_index_2) ;
    
    // Calculate the plagiarism ratio based on longest match and the size of the smaller submission
    double plagiarism_ratio = static_cast<double>(total_exact_matched_length) / std :: min(m, n);

    // Define a threshold for plagiarism ratio
    double plagiarism_threshold = 0.2; // Adjust as necessary
    if (plagiarism_ratio >= plagiarism_threshold) {
        flag = 1; // Set plagiarism flag
    }
    
    // print them for verification and debugging
    std :: cout<<flag<<std :: endl ;
    std :: cout<<total_exact_matched_length<<std :: endl ;
    std :: cout<<longest_approx_length<<std :: endl ;
    std :: cout<<start_index_1<<std :: endl ;
    std :: cout<<start_index_2<<std :: endl ;
    std :: cout<<std :: endl ;

    std :: array<int, 5> result = {flag, total_exact_matched_length, longest_approx_length, start_index_1, start_index_2} ;

    auto end = std :: chrono :: high_resolution_clock :: now() ;
    auto duration = std :: chrono :: duration_cast<std :: chrono :: milliseconds>(end-start) ;
    std :: cout<<"For m = "<<m<<" , n = "<<n<<" : Time taken in milli seconds = "<<duration.count()<<std :: endl;
    std :: cout<<std :: endl;

    // return the result
    return result ; 
}



