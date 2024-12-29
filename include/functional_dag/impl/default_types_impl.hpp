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

#include "functional_dag/dag_interface.hpp"

/** A source that just returns null pointers
 *
 * Sometimes you just need a root that does nothing.
 */
template <class Out>
class dag_fake_source : public dag_source<FirstOut> {
 public:
  /** Update generator function that returns NULL
   * @return NULL pointer.
   */
  Out *update() { return nullptr; }
};
