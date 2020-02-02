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
#include <ios>
#include <fstream>
#include <unistd.h>
#include "spline.h"

using namespace std;

/*
 * 
 */

void process_mem_usage(double& vm_usage, double& resident_set)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}






double fRand(double fMin, double fMax) {
    double f = (double) rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

int main(int argc, char** argv) {
    std::vector<double> index(100000000);
    std::vector<double> positions(100000000);
    //std::vector<double> error(5000000);
    double step = 0.2;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> rand_int64(
            0, std::numeric_limits<int64_t>::max());
    //Create random test data (from xindex)
    for (size_t i = 0; i < index.size(); ++i) {
        index[i] = rand_int64(gen);
        positions[i] = double (i) / index.size();
    }


    sort(index.begin(), index.end());

    tk::spline s;
    
    double vm, rss;
    double vm1, rss1;
    process_mem_usage(vm, rss);
    cout << "VM Size before set point(Kb): " << (int64_t)vm << "; RSS(Kb): " << (int64_t)rss << endl;
    
    double position;
    double key;
    s.set_points(index, positions, false);
    std::cout <<  "Size after set point" << sizeof(s) << endl;
    
    process_mem_usage(vm1, rss1);
    cout << "VM Size after set point(Kb): " << (int64_t)vm1 << "; RSS(Kb): " << (int64_t) rss1 << endl;
    
     cout << "memory usage by function(Kb): " << (int64_t)vm1 - (int64_t)vm  << "; RSS (Kb): " << (int64_t) rss1 - (int64_t)rss<< endl;
    
    cout << positions.size() << " " << index.size() << endl;
    int64_t total_duration = 0;
    int64_t std_total_duration = 0;
    int count = 0;
    for (size_t i = 0; i < index.size(); ++i) {
        auto start = chrono::high_resolution_clock::now();
        position = s(index[i]);
        key = index[position * index.size()];
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(stop - start);
        total_duration = total_duration + duration.count();
        count++;

        //cout << "Random Key:" << (int64_t)key << ";" << (int64_t)index[i] << " Real Position:" << i << " Predicted position:"
        //         << position*index.size() << " Duration:" << duration.count() << endl;

    }
    std::cout << "Average get: " << total_duration / count << " Number of gets:" << count << std::endl;
    cout << "End" << endl;

    key = 1680000000000000;
    position = s(key)*index.size();
    double pos = index[position];
    double l_pos = index[position-1];
    double r_pos = index[position+1];
    
    std::cout << "Search Key:" << (int64_t)key << " Found Key" << (int64_t)pos << " Predicted pos:" 
            << (int64_t)position << " Left key:" << (int64_t)l_pos << " Right key:" << (int64_t)r_pos <<   std::endl;
    int64_t start_p = (int64_t) position; 
    key = 1688000000000000;
    position = s(key)*index.size();
    pos = index[position];
    l_pos = index[position-1];
    r_pos = index[position+1];
    
    std::cout << "Search Key:" << (int64_t)key << " Found Key" << (int64_t)pos << " Predicted pos:" 
            << (int64_t)position << " Left key:" << (int64_t)l_pos << " Right key:" << (int64_t)r_pos <<   std::endl;
  
    int64_t stop_p = (int64_t) position; 
  
    std::cout << "Start pos: " << start_p << " Stop pos:" << stop_p <<  std::endl;
    for (int64_t j = start_p; j <= stop_p; j++)
        std::cout << "Range keys: " <<(int64_t) index[j] << std::endl;
    return 0;
}

