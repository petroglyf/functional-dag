#include <catch2/catch_test_macros.hpp>

#include "functional_dag/guid_impl.hpp"

TEST_CASE("Check string conversion", "[guid.checking_to_from]") {
  const std::string_view guid_str("2253c551-dd08-4ff4-928d-9b1e8c586c14");
  fn_dag::GUID<int> guid = fn_dag::GUID<int>::from_uuid(guid_str);
  REQUIRE(guid._id.bits1() == 2473537575747866612UL);
  REQUIRE(guid._id.bits2() == 10560267256759610388UL);
  const std::string conv_guid_str = guid.to_uuid();
  bool gstr_comp = guid_str == conv_guid_str;
  REQUIRE(gstr_comp);
}

TEST_CASE("Check comparison", "[guid.comparison]") {
  const std::string_view guid_str("2253c551-dd08-4ff4-928d-9b1e8c586c14");
  fn_dag::GUID<int> guid = fn_dag::GUID<int>::from_uuid(guid_str);
  fn_dag::GUID<int> pos_guid = fn_dag::GUID<int>::from_uuid(guid_str);

  const std::string_view neg_guid_str("714dc6ff-3afe-4d56-8174-9bc3086a7e26");
  fn_dag::GUID<int> neg_guid = fn_dag::GUID<int>::from_uuid(neg_guid_str);

  REQUIRE(guid == pos_guid);
  REQUIRE(guid == guid);
  bool gstr_comp = guid == neg_guid;
  REQUIRE_FALSE(gstr_comp);
}

TEST_CASE("Test error handling", "[guid.error_handling]") {
  fn_dag::GUID<int> guid =
      fn_dag::GUID<int>::from_uuid("2253c551-dd08-6-928d-9b1e8c586c14");
  REQUIRE(guid._id.bits1() == 0);
  REQUIRE(guid._id.bits2() == 0);

  guid = fn_dag::GUID<int>::from_uuid("");
  REQUIRE(guid._id.bits1() == 0);
  REQUIRE(guid._id.bits2() == 0);

  guid = fn_dag::GUID<int>::from_uuid("2253c551");
  REQUIRE(guid._id.bits1() == 0);
  REQUIRE(guid._id.bits2() == 0);

  guid = fn_dag::GUID<int>::from_uuid("2253c551-9b1e8c586c14");
  REQUIRE(guid._id.bits1() == 0);
  REQUIRE(guid._id.bits2() == 0);

  guid = fn_dag::GUID<int>::from_uuid("2253c551*9b1e8c586c14");
  REQUIRE(guid._id.bits1() == 0);
  REQUIRE(guid._id.bits2() == 0);
}