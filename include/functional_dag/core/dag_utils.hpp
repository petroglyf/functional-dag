/**
 *   ___                 .___               
 *  |_  \              __| _/____     ____  
 *   /   \    ______  / __ |\__  \   / ___\ 
 *  / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
 * /_/  \__\         \____ |(____  /\___  / 
 *                        \/     \//_____/   
 *                             
 * If a user decides to implement the functionals themselves, then they can use these interfaces.
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */ 
#pragma once
#include <iostream>
#include <string>

using namespace std;

namespace fn_dag {
  struct _dag_context {
    bool filter_off = false;
    bool run_single_threaded = false;
    string indent_str = "  ";
    ostream *log = &cout;
  };
};