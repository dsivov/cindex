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
#include <libalglib/interpolation.h> // alglib
#include <gsl/gsl_math.h>
#include <gsl/gsl_spline.h>

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
    std::vector<double> index(10000000);
    std::vector<double> positions(10000000);
    
    
    
    //std::vector<double> error(5000000);
    //double step = 0.2;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> rand_int64(
      0, std::numeric_limits<int64_t>::max());
    //Create random test data (from xindex)
    for (size_t i = 0; i < index.size(); ++i){
        index[i] = rand_int64(gen);   
        positions[i]=double (i)/index.size();
    }
    
  
    sort(index.begin(), index.end());
    //gsl init
    gsl_interp_accel *acc = gsl_interp_accel_alloc();
    gsl_spline *spline_steffen = gsl_spline_alloc(gsl_interp_steffen, 10000000);
    gsl_spline_init(spline_steffen, &index[0], &positions[0], 10000000);
    
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
    int total_duration = 0 ;
    int std_total_duration = 0;
    int count = 0;
    for (size_t i = 0; i < index.size(); ++i){  
        auto start = chrono::high_resolution_clock::now(); 
        //double position = s(index[i]);
        //double position = alglib::spline1dcalc(spline, index[i]);
        double position = gsl_spline_eval(spline_steffen, index[i], acc);
        double key = index[position*index.size()];
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(stop - start);
        total_duration = total_duration + duration.count();
        count++;        
      
        cout << "Random Key:" << (int64_t)key << ";" << (int64_t)index[i] << " Real Position:" << i << " Predicted position:"
                << position*index.size() << " Duration:" << duration.count() << endl;
            
    }
    std::cout << "Average get: " << total_duration/count << " Number of gets:" << count << std::endl;   
//    double position = alglib::spline1dcalc(spline,index[5003]);
//    double st = (index[5003] - index[5002])/2;
//    double stored_position = alglib::spline1dcalc(spline,index[5003] - st);
//    cout << "ALGLIB " << "Stored Min:" << (int64_t)index[5002] << " Stored Max:" << (int64_t)index[5003] << " Predict this Key:" 
//            << (int64_t)(index[5003] - st) << " Near stored position:"
//              << (double)position*index.size() << " Predicted position:" << (double)stored_position*index.size() << " Real key:" 
//            << (int64_t)index[(int)(stored_position*index.size())]<< endl;
    
    
    return 0;
}

