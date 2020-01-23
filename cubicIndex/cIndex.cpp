/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cIndex.cpp
 * Author: dimas
 *
 * Created on January 19, 2020, 12:37 PM
 */

#include <cstdlib>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <random>
#include <chrono> 
#include "spline.h"

using namespace std;

/*
 * 
 */

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}


int main(int argc, char** argv) {
    std::vector<double> index(1000000);
    std::vector<double> position(1000000);
    //std::vector<double> error(5000000);
    double step = 0.2;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> rand_int64(
      0, std::numeric_limits<int64_t>::max());
    //Create random test data (from xindex)
    for (size_t i = 0; i < index.size(); ++i){
        index[i] = rand_int64(gen);   
        position[i]=double (i)/index.size();
    }
    
  
    sort(index.begin(), index.end());
    
    tk::spline s;
    s.set_points(index,position);
    cout << position.size() << " " << index.size() << endl;
    int total_duration = 0 ;
    int std_total_duration = 0;
    int count = 0;
    for (size_t i = 0; i < index.size(); ++i){  
        auto start = chrono::high_resolution_clock::now(); 
        double position = s(index[i]);
        double key = index[position*index.size()];
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(stop - start);
        total_duration = total_duration + duration.count();
        count++;        
      
       // cout << "Random Key:" << (int64_t)key << ";" << (int64_t)index[i] << " Real Position:" << i << " Predicted position:"
       //         << position*index.size() << " Duration:" << duration.count() << endl;
            
    }
    std::cout << "Average get: " << total_duration/count << " Number of gets:" << count << std::endl;   
    cout << "End" << endl;
    
    
    return 0;
}

