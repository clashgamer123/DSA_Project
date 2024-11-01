#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
#include<algorithm>
#include<unordered_set>
#include <fstream>
// -----------------------------------------------------------------------------

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

std::array<int, 5> match_submissions(std::vector<int> &sub1, std::vector<int> &sub2)
{
    int m = sub1.size() ;
    int n = sub2.size() ;
    // std :: cout<<m<<" "<<n<<std :: endl;
    //  // Create ofstream object for file output
    // if(m==570){
    // std::ofstream file_out("debug_output.txt");
    // for(int i = 0; i<m; i++) {file_out<<sub1[i]<<" "<<sub2[i]<<std :: endl ;}
    // }

    // sub1 and sub2 are the 2 submissions
    int flag = 0;
    int total_matched_length = 0;
    int longest_approx_length = 0;
    int start_index_1 = 0;
    int start_index_2 = 0;

    // exact mathces first
    int min_length = 10;
    int temp_length ;
    std :: vector<std :: pair<int, int>> match_2 ;
    for(int i = 0; i<=m-min_length; i++)
    {
        // start match from i;
        // lets use j_check to make sure we skip matched patterns in sub2
        int j_check = 0;
        for(int j = 0; j<=n-min_length; j++)
        {
            if(j_check<match_2.size() && j==match_2[j_check].first)
            {
                // we skip the previously matched part
                j = match_2[j_check].second ;
                j_check++;
                continue;
            }
            temp_length = 0;
            int k1 = i ;
            int k2 = j;
            while(k1<m && k2<n && sub1[k1] == sub2[k2])
            {
                if(j_check<match_2.size() && k2>=match_2[j_check].first) { break ; }
                k1++; k2++ ;
                temp_length+=1 ;
            }
            if(temp_length>=min_length)
            {
                // match found
                total_matched_length+=temp_length ;
                match_2.emplace_back(j, k2-1) ;
                i = k1-1 ;
                break ;
            }
        }
    }

    // approx matches greater or equal to 30 and should be atleast 80 percent of the longer string
    std :: vector<std :: vector<int>> lcs(m+1, std :: vector<int>(n+1, 0)) ;
    for(int i = 1; i<=m; i++)
    {
        for(int j = 1; j<=n; j++)
        {
            // LCS of sub1[1-i] and sub2[1-j] ( 1 indexed notation )
            if(sub1[i-1] == sub2[j-1])
            {
                lcs[i][j] = 1 + lcs[i-1][j-1] ;
                continue; 
            }
            lcs[i][j] = std :: max(lcs[i-1][j], lcs[i][j-1]) ;
        }
    }
    // std :: cout<<"LCS : "<<lcs[m][n]<<std :: endl; 
    
    // int s = std :: min(m, n) ;
    // int l = std :: max(m, n) ;
    // double ratio ;
    // int length ;
    // for(int i = 0; i<=s; i++)
    // {
    //     length = lcs[m-i][n-i] ;
    //     if(length<30) { continue; }
    //     ratio = length/static_cast<double>(l-i) ;
    //     if(ratio<0.8) { continue ; }
    //     // we found the match
    //     longest_approx_length = l-i ;
    //     break; 
    // }

    double ratio;
    int length;
    bool found = false ;
    for(int f = 1; f<=m; f++)
    {
        int slimit = n+1 - std :: ceil(0.8*static_cast<double>(m-f+1)) ;
        for(int s = 1; s<=slimit; s++)
        {
            // start pattern match from f in sub1 and s in sub2(1-indexed)
            int smaller = std :: min(m-f+1, n-s+1) ;
            int larger  = std :: max(m-f+1, n-s+1) ;
            for(int i = 0; i<=smaller; i++)
            {
                length = lcs[m-i][n-i]-lcs[f-1][s-1] ;
                if(length<30) {continue ;}
                ratio = length/static_cast<double>(larger-i) ;
                if(ratio<0.8) {continue;}
                // we found the answer
                longest_approx_length = smaller-i;
                start_index_1 = f;
                start_index_2 = s;
                found = true ;
                break;
            } 
            if(found) { break ;}
        }
        if(found) { break ; }
    }
    // We have calculate the dp table. now assuming 

    // Calculate the plagiarism ratio based on longest match and the size of the smaller submission
    double plagiarism_ratio = static_cast<double>(total_matched_length) / std :: min(m, n);

    // Define a threshold for plagiarism ratio
    double plagiarism_threshold = 0.3; // Adjust as necessary
    if (plagiarism_ratio >= plagiarism_threshold) {
        flag = 1; // Set plagiarism flag
    }
    
    std :: cout<<flag<<std :: endl ;
    std :: cout<<total_matched_length<<std :: endl ;
    std :: cout<<longest_approx_length<<std :: endl ;
    std :: cout<<start_index_1<<std :: endl ;
    std :: cout<<start_index_2<<std :: endl ;
    std :: cout<<std :: endl ;
    std :: array<int, 5> result = {flag, total_matched_length, longest_approx_length, start_index_1, start_index_2} ;
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