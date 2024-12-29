#pragma once
/**
 * ---------------------------------------------
 *    ___                 .___
 *   |_  \              __| _/____     _______
 *    /   \    ______  / __ |\__  \   / ___  /
 *   / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
 *  /_/  \__\         \____ |(____  /\___  /
 *                         \/     \//_____/
 * ---------------------------------------------
 * @author ndepalma@alum.mit.edu
 */
#include <iostream>

namespace fn_dag {
using namespace std;
/** Context datastructure for all classes to share state
 *  The context provides shared state amongst the nodes and the fan-in / fan-out
 *  infrastructure. When the class is turned off, all threads are able to use
 * this shared state to stop themselves.
 */
struct _dag_context {
  bool filter_off;           //! Whether the dag is running
  bool run_single_threaded;  //! Whether the dag is running in threads or single
                             //! threaded

  ostream *log;  //! Which output stream to log to. Useful to override.
  string_view indent_str;  //! How far to indent when printing the dag info

  _dag_context()
      : filter_off(false),
        run_single_threaded(false),
        log(&cout),
        indent_str("  ") {}
};
};  // namespace fn_dag
