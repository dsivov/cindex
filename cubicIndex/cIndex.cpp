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
#include <immintrin.h>

using namespace std;

/*
 * 
 */

#define ALIGN __attribute__ ((aligned (32)))

double mac_with_simd (double* a, double* b, unsigned int length) {
        int i;
        
        __m256d ALIGN sum = {0.0, 0.0, 0.0, 0.0}; //vector to hold partial sums
        for (i = 0; i < length; i += 4) {
                __m256d va = _mm256_load_pd(&a[i]);
                __m256d vb = _mm256_load_pd(&b[i]);
                       
                sum = _mm256_fmadd_pd (va, vb, sum);
        }
        
        //sum now hold summations of all products in four parts
        //we want scalar result
        //two options below
        
#if 1 //Enable this block to perform vector sum with intrinsics
        
        //x86 architecture have little endian data ordering
        
        //                               index: 0  1  2  3 
        //sum contains quad 64bit doubles, say: m, n, p, q
        //we want scalar result = m + n + p + q
        
        //intrinsic function to extract upper 128 bits.
        //if second parameter is zero then lower 128 bits are extracted.
        __m128d xmm = _mm256_extractf128_pd (sum, 1); 
        //xmm contains: p, q
        
        //This intrinsic is compile time only.
        //__m256d ymm = _mm256_zextpd128_pd256 (xmm); //But missing in GCC 5.4.0
        
        //zero extend xmm to make 256bit vector ymm.
        __m256d ymm = {xmm[0], xmm[1], 0, 0};
        //ymm contains: p, q, 0, 0
        
        //intrinsic function to perform horizontal interleaved addition.
        sum = _mm256_hadd_pd (sum, ymm); 
        //sum contains: m+n, p+q, p+q, 0+0  
        
        //another round of horizontal interleaved addition
        sum = _mm256_hadd_pd (sum, sum);
        //sum contains: m+n+p+q, m+n+p+q, p+q+0, p+q+0

        return sum[0]; //scalar result = m+n+p+q

#else //vector sum with C arithmetic operators.
        
        double y = 0;
        
        for (i = 0; i < 4; i++) {
                y += sum[i];
        }
        
        return y; //scalar result
#endif
}

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

void monoCubicIndex (std::vector<double> index, std::vector<double> positions, int numberOfKeys) {
    int count = 0;
    double _position;
    double position;
    double key;   
    //gsl init
    double vm, rss;
    double vm1, rss1;
    int64_t total_duration = 0;
    int64_t std_total_duration = 0;
    process_mem_usage(vm, rss);
    cout << "VM Size before set point(Kb): " << (int64_t)vm << "; RSS(Kb): " << (int64_t)rss << endl;
    process_mem_usage(vm1, rss1);
    cout << "VM Size after set point(Kb): " << (int64_t)vm1 << "; RSS(Kb): " << (int64_t) rss1 << endl;
    cout << "memory usage by function(Kb): " << (int64_t)vm1 - (int64_t)vm  << "; RSS (Kb): " << (int64_t) rss1 - (int64_t)rss<< endl;
    cout << positions.size() << " " << index.size() << endl;
    sort(index.begin(), index.end());
    gsl_interp_accel *acc = gsl_interp_accel_alloc();
    gsl_spline *spline_steffen = gsl_spline_alloc(gsl_interp_steffen, numberOfKeys);
    gsl_spline_init(spline_steffen, &index[0], &positions[0], numberOfKeys); 
    cout << positions.size() << " " << index.size() << endl;
    for (size_t i = 0; i < index.size(); i++){  
        auto start = chrono::high_resolution_clock::now(); 
        //double position = s(index[i]);
        //double position = alglib::spline1dcalc(spline, index[i]);
        int res = gsl_spline_eval_e(spline_steffen, index[i], acc, &_position);
        position =(size_t)(_position*index.size());
//        if (position != i){
//            cout <<"For real key:" << (int64_t)index[i] << " on position: " << i
//                    << " we found: " << (int64_t)key << " on position:" << position << endl;
//            cout << " Near keys, from right:" << (int64_t)index[i+1] << " Position:" << i+1
//                    << " from left:" << (int64_t)index[i-1] << " Position:" << i-1 << endl; 
//        }
            
        key = index[position];
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(stop - start);
        total_duration = total_duration + duration.count();
        count++; 
//        if (index[i] != key) {
//            cout << "================================ERROR===========================" << endl;
//            cout <<"For real key:" << (int64_t)index[i] << " on position: " << i
//                    << " we found: " << (int64_t)key << " on position:" << position<< endl;
//            cout << " Near keys, from right:" << (int64_t)index[i+1] << " Position:" << i+1
//                    << " from left:" << (int64_t)index[i-1] << " Position:" << i-1 << endl; 
//            cout << "===============================================================" << endl;
//        }    
        //cout << "Random Key:" << (int64_t)key << ";" << (int64_t)index[i] << " Real Position:" << i << " Predicted position:"
         //       << position*index.size() << " Duration:" << duration.count() << endl;
            
    }
    //std::cout << "Average get: " << total_duration/count << " Number of gets:" << count << std::endl;
    return;
}

void gen_random(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    cout << "Generate str" << endl;
    for (int i = 0; i < len; ++i) {
        int random = rand();
        int pos = int(random % (sizeof(alphanum) - 1));
        s[i] = alphanum[pos];
        //cout << "Char: " << s[i] << " Pos: " << pos << "Random: " << random <<  endl;
    }

    s[len] = '\0';
}

double convertKey (char* key) {
    double ALIGN factor[] = {pow(10.0, 7), pow(10.0, 6), pow(10.0, 5), pow(10.0, 4), pow(10.0, 3), pow(10.0, 2), pow(10.0, 1), 1};
    char str1[] = "00000000";
    double ALIGN indeterminates[8];
    srand(time(NULL));
    gen_random(str1, 8);
    double res1 = 0; 
    double res2 = 0;
    auto start = chrono::high_resolution_clock::now(); 
    for (int i = 0 ; i <= 7 ; i++){        
        indeterminates[i] = (double) str1[i];
        res1 = res1 + ((int)indeterminates[i] * (factor[i]));  
        cout << "SizeOf: " << sizeof(indeterminates[i]) << endl;
    }
    
    //res1 = mac_with_simd(indeterminates, factor, 8);
    auto stop = chrono::high_resolution_clock::now(); 
    auto duration = chrono::duration_cast<chrono::nanoseconds>(stop - start);
    cout << " Duration:" << duration.count() << endl;
    cout << "String1 " << str1 << " Number: " << (int64_t)(res1) << endl;
    return 0;
}

void rangeTestCubicIndex () {
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
    
}
int numberOfKeys = 10000000;
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

    //GSL Lib monotonic cubic spline
    //monoCubicIndex (index, positions, numberOfKeys);
    char* t;
    convertKey(t);

   
    return 0;
}

