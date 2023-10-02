#pragma once
// ---------------------------------------------
//    ___                 .___  
//   |_  \              __| _/____     ____
//    /   \    ______  / __ |\__  \   / ___\
//   / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
//  /_/  \__\         \____ |(____  /\___  /
//                         \/     \//_____/
// ---------------------------------------------
// @author ndepalma@alum.mit.edu

#include <iostream>
#include <string>

using namespace std;

namespace fn_dag {
  /** Context datastructure for all classes to share state
   *  The context provides shared state amongst the nodes and the fan-in / fan-out
   *  infrastructure. When the class is turned off, all threads are able to use this
   *  shared state to stop themselves.
   */
  struct _dag_context {
    bool filter_off = false;            //! Whether the dag is running
    bool run_single_threaded = false;   //! Whether the dag is running in threads or single threaded
    string indent_str = "  ";           //! How far to indent when printing the dag info
    ostream *log = &cout;               //! Which output stream to log to. Useful to override.
  };
};