#include <catch2/catch_test_macros.hpp>

#include "functional_dag/error_codes.h"
#include "functional_dag/guid_impl.hpp"

TEST_CASE("Check string conversion", "[guid.checking_to_from]") {
  const std::string_view guid_str("2253c551-dd08-4ff4-928d-9b1e8c586c14");
  if (auto res = fn_dag::GUID<int>::from_uuid(guid_str); res.has_value()) {
    fn_dag::GUID<int> guid = res.value();
    REQUIRE(guid.m_id.bits1() == 2473537575747866612UL);
    REQUIRE(guid.m_id.bits2() == 10560267256759610388UL);
    const std::string conv_guid_str = guid.to_uuid();
    bool gstr_comp = guid_str == conv_guid_str;
    REQUIRE(gstr_comp);
  } else {
    REQUIRE(res.has_value());
  }
}

TEST_CASE("Check comparison", "[guid.comparison]") {
  const std::string_view guid_str("2253c551-dd08-4ff4-928d-9b1e8c586c14");
  if (auto ref_res = fn_dag::GUID<int>::from_uuid(guid_str);
      ref_res.has_value()) {
    if (auto pos_res = fn_dag::GUID<int>::from_uuid(guid_str);
        pos_res.has_value()) {
      const fn_dag::GUID<int> guid = ref_res.value();
      const fn_dag::GUID<int> pos_guid = pos_res.value();
      const std::string_view neg_guid_str(
          "714dc6ff-3afe-4d56-8174-9bc3086a7e26");

      if (auto neg_res = fn_dag::GUID<int>::from_uuid(neg_guid_str);
          neg_res.has_value()) {
        fn_dag::GUID<int> neg_guid = neg_res.value();
        REQUIRE(guid == pos_guid);
        REQUIRE(guid == guid);
        const bool gstr_comp = guid == neg_guid;
        REQUIRE_FALSE(gstr_comp);
      } else {
        REQUIRE(neg_res.has_value());
      }
    } else {
      REQUIRE(pos_res.has_value());
    }
  } else {
    REQUIRE(ref_res.has_value());
  }
}

TEST_CASE("Test error handling", "[guid.error_handling]") {
  if (auto ref_res =
          fn_dag::GUID<int>::from_uuid("2253c551-dd08-6-928d-9b1e8c586c14");
      !ref_res.has_value()) {
    REQUIRE(ref_res.error() == fn_dag::HEX_SIZE_INCORRECT);
  } else {
    REQUIRE(!ref_res.has_value());
  }

  if (auto ref_res = fn_dag::GUID<int>::from_uuid(""); !ref_res.has_value()) {
    REQUIRE(ref_res.error() == fn_dag::NOT_ENOUGH_ELEMENTS);
  } else {
    REQUIRE(!ref_res.has_value());
  }

  if (auto ref_res = fn_dag::GUID<int>::from_uuid("2253c551");
      !ref_res.has_value()) {
    REQUIRE(ref_res.error() == fn_dag::NOT_ENOUGH_ELEMENTS);
  } else {
    REQUIRE(!ref_res.has_value());
  }

  if (auto ref_res = fn_dag::GUID<int>::from_uuid("2253c551-9b1e8c586c14");
      !ref_res.has_value()) {
    REQUIRE(ref_res.error() == fn_dag::NOT_ENOUGH_ELEMENTS);
  } else {
    REQUIRE(!ref_res.has_value());
  }

  if (auto ref_res = fn_dag::GUID<int>::from_uuid("2253c551*9b1e8c586c14");
      !ref_res.has_value()) {
    REQUIRE(ref_res.error() == fn_dag::NOT_ENOUGH_ELEMENTS);
  } else {
    REQUIRE(!ref_res.has_value());
  }
}
