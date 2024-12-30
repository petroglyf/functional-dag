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
#include <expected>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "fb_gen/guid_generated.h"
#include "functional_dag/error_codes.h"

namespace fn_dag {
using namespace std;

/** (Utility) For defining a globally unique UUIDv4 identifier.
 *
 * This class is a utility that serializes a UUIDv4 to a flatbuffer uuid and
 * back. More specificaly, UUIDs are stored as 128bit unsigned integers for
 * speed of comparison and sorting once IDs have been turned into it's internal
 * representation.
 */
template <typename T>
class GUID {
 public:
  /// The internal UUID representation.
  GUID_vals m_id;

  /** Wrapper of the 128 bit integer.
   *
   * GUID Constructor.
   *
   * @param _values A 128bit uint to wrap.
   */
  GUID(const GUID_vals &_values);

  /** Copy constructor of the GUID.
   *
   * GUID Constructor.
   *
   * @param _values Another GUID to copy.
   */
  GUID(const GUID<T> &_values);

  /** Get the UUIDv4 representation of the GUID.
   *
   * This function takes the internal 128bit unsigned integer and formats
   * it as a well formed hexidecimal UUIDv4.
   *
   * @return A well formed UUIDv4 using the underlying 128 bit integer.
   */
  string to_uuid() const;

  /** Parse a UUIDv4 representation into an internal 128bit unsigned int.
   *
   * This function will attempt to parse a string that contains a UUIDv4.
   * In the event that it does not successfully parse it, an error is returned
   * instead.
   *
   * @param _uuid A well formed UUIDv4 string.
   * @return An expectation of a GUID or a specific error in the event of a
   * parsing problem.
   */
  static expected<GUID<T>, error_codes> from_uuid(const string_view &_uuid);
};

template <typename T>
GUID<T>::GUID(const GUID_vals &_values) {
  m_id = _values;
}

template <typename T>
GUID<T>::GUID(const GUID<T> &_values) {
  m_id = _values.m_id;
}

/** Equality operator.
 *
 * This function checks to see if two GUIDs of the same underlying type are
 * equal.
 *
 * @param _left One GUID to check.
 * @param _right Another GUID to check.
 * @return True if equal, false otherwise.
 */
template <typename T>
bool operator==(const GUID<T> &_left, const GUID<T> &_right) {
  return _left.m_id.bits1() == _right.m_id.bits1() &&
         _left.m_id.bits2() == _right.m_id.bits2();
}

/** Less than operator.
 *
 * This function checks to see if the GUID on the left is less than the GUID on
 * the right. This function is required for sorting in general.
 *
 * @param _left One GUID to check.
 * @param _right Another GUID to check.
 * @return True if left is less than right, otherwise false.
 */
template <typename T>
bool operator<(const GUID<T> &_left, const GUID<T> &_right) {
  return _left.m_id.bits1() < _right.m_id.bits1() ||
         (_left.m_id.bits1() == _right.m_id.bits1() &&
          _left.m_id.bits2() < _right.m_id.bits2());
}

template <typename T>
string GUID<T>::to_uuid() const {
  stringstream uuid{};
  uint32_t first_four_bytes = (m_id.bits1() >> 4 * 8);
  uint16_t second_two_bytes = (m_id.bits1() << 4 * 8) >> 6 * 8;
  uint16_t last_two_bytes = (m_id.bits1() & ((255 << 8) + 255));

  uint16_t first_two_bytes = (m_id.bits2() >> 6 * 8);
  uint64_t last_six_bytes = (m_id.bits2() << 2 * 8 >> 2 * 8);

  uuid << hex << first_four_bytes << "-" << hex << second_two_bytes << "-"
       << hex << last_two_bytes << "-" << hex << first_two_bytes << "-" << hex
       << last_six_bytes;
  return uuid.str();
}

template <typename T>
expected<GUID<T>, error_codes> GUID<T>::from_uuid(const string_view &_uuid) {
  constexpr uint64_t expect_len[5] = {8, 4, 4, 4, 12};
  uint64_t first_four_bytes;
  uint64_t second_two_bytes;
  uint64_t last_two_bytes;
  uint64_t first_two_bytes;
  uint64_t last_six_bytes;
  uint64_t *const dest_vals[5] = {&first_four_bytes, &second_two_bytes,
                                  &last_two_bytes, &first_two_bytes,
                                  &last_six_bytes};
  int i = 0;
  size_t pos = 0;
  constexpr char sep = '-';
  size_t end_pos = _uuid.find(sep, pos);
  while (end_pos != string_view::npos || i == 4) {
    const string_view next_hex = _uuid.substr(pos, end_pos - pos);
    if (next_hex.size() != expect_len[i]) {
      return unexpected(error_codes::HEX_SIZE_INCORRECT);
    }

    istringstream(string(next_hex)) >> hex >> *dest_vals[i];
    i++;

    if (i == 5) {
      break;
    }
    pos = end_pos + 1;
    end_pos = _uuid.find(sep, pos);
  }
  if (i != 5) {
    return unexpected(error_codes::NOT_ENOUGH_ELEMENTS);
  }
  uint64_t bits1 = (first_four_bytes << 4 * 8) + (second_two_bytes << 2 * 8) +
                   last_two_bytes;
  uint64_t bits2 = (first_two_bytes << 6 * 8) + last_six_bytes;
  return GUID<T>(GUID_vals(bits1, bits2));
}
}  // namespace fn_dag