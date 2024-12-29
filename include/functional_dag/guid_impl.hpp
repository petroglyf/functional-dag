#pragma once

#include <expected>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "fb_gen/guid_generated.h"
#include "functional_dag/error_codes.h"

namespace fn_dag {
using namespace std;

template <typename T>
class GUID {
 public:
  GUID_vals _id;

  GUID(const GUID_vals &values);
  string to_uuid() const;
  static expected<GUID<T>, error_codes> from_uuid(const string_view &);
};

template <typename T>
GUID<T>::GUID(const GUID_vals &values) {
  _id = values;
}

template <typename T>
bool operator==(const GUID<T> &left, const GUID<T> &right) {
  return left._id.bits1() == right._id.bits1() &&
         left._id.bits2() == right._id.bits2();
}

template <typename T>
bool operator<(const GUID<T> &left, const GUID<T> &right) {
  return left._id.bits1() < right._id.bits1() ||
         (left._id.bits1() == right._id.bits1() &&
          left._id.bits2() < right._id.bits2());
}

template <typename T>
string GUID<T>::to_uuid() const {
  stringstream uuid{};
  uint32_t first_four_bytes = (_id.bits1() >> 4 * 8);
  uint16_t second_two_bytes = (_id.bits1() << 4 * 8) >> 6 * 8;
  uint16_t last_two_bytes = (_id.bits1() & (255 << 8) + 255);

  uint16_t first_two_bytes = (_id.bits2() >> 6 * 8);
  uint64_t last_six_bytes = (_id.bits2() << 2 * 8 >> 2 * 8);

  uuid << hex << first_four_bytes << "-" << hex << second_two_bytes << "-"
       << hex << last_two_bytes << "-" << hex << first_two_bytes << "-" << hex
       << last_six_bytes;
  return uuid.str();
}

template <typename T>
expected<GUID<T>, error_codes> GUID<T>::from_uuid(const string_view &guid_str) {
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
  size_t end_pos = guid_str.find(sep, pos);
  while (end_pos != string_view::npos || i == 4) {
    const string_view next_hex = guid_str.substr(pos, end_pos - pos);
    if (next_hex.size() != expect_len[i]) {
      return unexpected(error_codes::HEX_SIZE_INCORRECT);
    }

    istringstream(string(next_hex)) >> hex >> *dest_vals[i];
    i++;

    if (i == 5) {
      break;
    }
    pos = end_pos + 1;
    end_pos = guid_str.find(sep, pos);
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