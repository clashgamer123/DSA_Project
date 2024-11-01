#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
#include<algorithm>
#include<unordered_set>
#include <fstream>
#include<chrono>
// -----------------------------------------------------------------------------

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

// std :: cout<<m<<" "<<n<<std :: endl;
//  // Create ofstream object for file output
// if(m==570){
// std::ofstream file_out("debug_output.txt");
// for(int i = 0; i<m; i++) {file_out<<sub1[i]<<" "<<sub2[i]<<std :: endl ;}
// }

// Function to find total exact matched length
int ExactMatchLength(const std :: vector<int>& sub1, const std :: vector<int>& sub2, int& min_length)
{
    // sizes of the vectors
    int m = sub1.size() ;
    int n = sub2.size() ;
    
    // the total exact matched length
    int total_matched_length = 0;

    // temporary length holder  
    int temp_length ;

    // A vector of pairs (i, j) indicating pattern match from i to j in sub2
    // so as to prevent overlapping ie skipping them
    std :: vector<std :: pair<int, int>> matches ;


    for(int i = 0; i<=m-min_length; i++)
    {
        // start checking for match from i;
        // lets use j_check to make sure we skip matched patterns in sub2
        int j_check = 0;
        for(int j = 0; j<=n-min_length; j++)
        {
            if(j_check<matches.size() && j==matches[j_check].first)
            {
                // we skip the previously matched part in sub2
                j = matches[j_check].second ;
                j_check++;
                continue;
            }
            
            // Find the degree of match starting from here
            temp_length = 0;
            int k1 = i ;
            int k2 = j;
            while(k1<m && k2<n && sub1[k1] == sub2[k2])
            {
                // break out in case we step into already matched pattern
                if(j_check<matches.size() && k2>=matches[j_check].first) { break ; }

                k1++; k2++ ;
                temp_length+=1 ;
            }

            // Check if the match is valid
            if(temp_length>=min_length)
            {
                // match found
                total_matched_length+=temp_length ;
                matches.emplace_back(j, k2-1) ;
                i = k1-1 ;
                break ;
            }
        }  
    }

    // return the total exact matched length
    return total_matched_length;
}

void ComputeApproximateMatches(const std :: vector<int>& sub1, const std :: vector<int> sub2, const std :: vector<std :: vector<int>>& LCS, 
                               int& longest_approx_length, int& start_index_1, int& start_index_2)
{
    int m = sub1.size() ;
    int n = sub2.size() ;
    // We are solving for the longest approximate match 
    // Lets use an iterative approach
    double ratio;
    int length;
    bool found = false ;
    for(int f = 1; f<=m; f++)
    {
        int slimit = n+1 - std :: ceil(0.8*static_cast<double>(m-f+1)) ;
        for(int s = 1; s<=slimit; s++)
        {
            found  = false ;
            // start pattern match from f in sub1 and s in sub2(1-indexed)
            int l1 = m, l2 = n;
            while(l1>=f+29 && l2>=s+29)
            {
                length = LCS[l1][l2]-LCS[f-1][s-1] ;
                ratio = length/static_cast<double>(std :: max(l1-f+1, l2-s+1)) ;
                if(std :: max(l1-f+1, l2-s+1)>=30 && ratio>=0.8)
                {   if(length>longest_approx_length)
                    {
                        longest_approx_length = length;
                        start_index_1 = f-1;
                        start_index_2 = s-1;
                    }
                    found = true ; break ;
                }
                if(l1-f+1>l2-s+1)
                {
                    l1--; continue ;
                }
                if(l1-f+1<l2-s+1)
                {
                    l2--; continue ;
                }
                l1--; l2--; continue;
            }

            if(found) { break ;}
        }
        if(m-f<=longest_approx_length || m-f<30) { break ; }
    }

    return ;
}

void ComputeLCS(const std :: vector<int>& sub1, const std :: vector<int>& sub2, std :: vector<std :: vector<int>>& LCS)
{
    // LCS[i][j] == Length of longest common subsequence from 1 to i in sub1 
    // and 1 to j in sub2. 
    // Lets use an iterative approch
    for(int i = 1; i<=sub1.size(); i++)
    {
        for(int j = 1; j<=sub2.size(); j++)
        {
            // LCS of sub1[1:i] and sub2[1:j] ( 1 indexed notation )
            if(sub1[i-1] == sub2[j-1])
            {
                LCS[i][j] = 1 + LCS[i-1][j-1] ;
                continue; 
            }
            LCS[i][j] = std :: max(LCS[i-1][j], LCS[i][j-1]) ;
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

    // Fill the dp table of LCS
    std :: vector<std :: vector<int>> LCS(m+1, std :: vector<int>(n+1, 0)) ;
    ComputeLCS(sub1, sub2, LCS);

    // Compute the longest approx matches 
    ComputeApproximateMatches(sub1, sub2, LCS, longest_approx_length, start_index_1, start_index_2) ;

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


// 1
// 410
// 280
// 280
// 160

// 1
// 890
// 890
// 0
// 0

// 1
// 490
// 180
// 180
// 170

// 423
// 903
// 509
