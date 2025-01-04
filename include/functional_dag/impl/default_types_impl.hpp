#pragma once
/** ---------------------------------------------
 *    ___                 .___
 *   |_  \              __| _/____     ____
 *    /   \    ______  / __ |\__  \   / ___\
 *   / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
 *  /_/  \__\         \____ |(____  /\___  /
 *                         \/     \//_____/
 * ---------------------------------------------
 * @author ndepalma@alum.mit.edu
 */

#include <memory>

#include "functional_dag/dag_interface.hpp"

namespace fn_dag {
using namespace std;
/** A source that just returns null pointers
 *
 * Sometimes you just need a root that does nothing.
 */
template <class Out>
class dag_fake_source : public dag_source<Out> {
 public:
  /** Update generator function that returns NULL
   * @return NULL pointer.
   */
  unique_ptr<Out> update() { return make_unique<Out>(nullptr); }
};
}  // namespace fn_dag