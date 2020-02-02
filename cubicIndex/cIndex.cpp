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
#include <math.h>
//#include <libalglib/interpolation.h> // alglib
#include <gsl/gsl_math.h>
#include <gsl/gsl_spline.h>

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

int numberOfKeys = 100000000;
int main(int argc, char** argv) {
    std::vector<double> index(numberOfKeys);
    std::vector<double> positions(numberOfKeys);
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

    //tk::spline s;
    
    double vm, rss;
    double vm1, rss1;
    process_mem_usage(vm, rss);
    cout << "VM Size before set point(Kb): " << (int64_t)vm << "; RSS(Kb): " << (int64_t)rss << endl;
   
    
    process_mem_usage(vm1, rss1);
    cout << "VM Size after set point(Kb): " << (int64_t)vm1 << "; RSS(Kb): " << (int64_t) rss1 << endl;
    
     cout << "memory usage by function(Kb): " << (int64_t)vm1 - (int64_t)vm  << "; RSS (Kb): " << (int64_t) rss1 - (int64_t)rss<< endl;
    
    cout << positions.size() << " " << index.size() << endl;
    int64_t total_duration = 0;
    int64_t std_total_duration = 0;
    int count = 0;
    double position;
    double key;

    
    //gsl init
    gsl_interp_accel *acc = gsl_interp_accel_alloc();
    gsl_spline *spline_steffen = gsl_spline_alloc(gsl_interp_steffen, numberOfKeys);
    gsl_spline_init(spline_steffen, &index[0], &positions[0], numberOfKeys); 
    //alglib::real_1d_array AX, AY;
    //AX.setcontent(index.size(), &(index[0]));
    //AY.setcontent(positions.size(), &(positions[0]));
    //alglib::spline1dinterpolant spline;
    //alglib::spline1dbuildlinear(AX, AY, index.size(), 2,0.0,2,0.0, spline);
    //alglib::spline1dbuildlinear(AX, AY, index.size(),spline);
    //tk::spline s;
    //s.set_boundary(tk::spline::second_deriv,0,tk::spline::first_deriv,400,false);
    //s.set_points(index,positions);
    cout << positions.size() << " " << index.size() << endl;
    for (size_t i = 0; i < index.size(); ++i){  
        auto start = chrono::high_resolution_clock::now(); 
        //double position = s(index[i]);
        //double position = alglib::spline1dcalc(spline, index[i]);
        position = gsl_spline_eval(spline_steffen, index[i], acc);
        key = index[position*index.size()];
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(stop - start);
        total_duration = total_duration + duration.count();
        count++;         
        //cout << "Random Key:" << (int64_t)key << ";" << (int64_t)index[i] << " Real Position:" << i << " Predicted position:"
         //       << position*index.size() << " Duration:" << duration.count() << endl;
            
    }
    std::cout << "Average get: " << total_duration/count << " Number of gets:" << count << std::endl;   
//    double position = alglib::spline1dcalc(spline,index[5003]);
//    double st = (index[5003] - index[5002])/2;
//    double stored_position = alglib::spline1dcalc(spline,index[5003] - st);
//    cout << "ALGLIB " << "Stored Min:" << (int64_t)index[5002] << " Stored Max:" << (int64_t)index[5003] << " Predict this Key:" 
//            << (int64_t)(index[5003] - st) << " Near stored position:"
//              << (double)position*index.size() << " Predicted position:" << (double)stored_position*index.size() << " Real key:" 
//            << (int64_t)index[(int)(stored_position*index.size())]<< endl;

   /*
    key = 1680000000000000;
    position = gsl_spline_eval(spline_steffen, key, acc)*index.size();
    double pos = index[position];
    double l_pos = index[position-1];
    double r_pos = index[position+1];
    
    std::cout << "Search Key:" << (int64_t)key << " Found Key" << (int64_t)pos << " Predicted pos:" 
            << (int64_t)position << " Left key:" << (int64_t)l_pos << " Right key:" << (int64_t)r_pos <<   std::endl;
   
    int64_t start_p = (int64_t) position; 
    key = 1688000000000000;
    position = gsl_spline_eval(spline_steffen, key, acc)*index.size();
    pos = index[position];
    l_pos = index[position-1];
    r_pos = index[position+1];
    
    std::cout << "Search Key:" << (int64_t)key << " Found Key" << (int64_t)pos << " Predicted pos:" 
            << (int64_t)position << " Left key:" << (int64_t)l_pos << " Right key:" << (int64_t)r_pos <<   std::endl;
  
    int64_t stop_p = (int64_t) position; 
  
    std::cout << "Start pos: " << start_p << " Stop pos:" << stop_p <<  std::endl;
    for (int64_t j = start_p; j <= stop_p; j++)
        std::cout << "Range keys: " <<(int64_t) index[j] << std::endl;
    */
    return 0;
}

