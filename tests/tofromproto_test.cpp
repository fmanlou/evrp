#include "evrp/sdk/types.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/tofromproto.h"

#include <gtest/gtest.h>

#include <any>
#include <map>

#include <google/protobuf/struct.pb.h>

TEST(ToFromProto, RoundtripStringsAndLogLevel) {
  MemorySetting m;
  m.insert("device", std::string("192.168.1.1:50051"));
  m.insert("logLevel", logging::LogLevel::Debug);
  google::protobuf::Struct s;
  evrp::sdk::toProto(m.snapshot(), &s);
  std::map<std::string, std::any> out;
  evrp::sdk::fromProto(out, s);
  EXPECT_EQ(std::any_cast<const std::string&>(out.at("device")),
            "192.168.1.1:50051");
  EXPECT_EQ(std::any_cast<logging::LogLevel>(out.at("logLevel")),
            logging::LogLevel::Debug);
}

TEST(ToFromProto, KindsListFromStruct) {
  google::protobuf::Struct s;
  google::protobuf::ListValue* lv =
      (*s.mutable_fields())["kinds"].mutable_list_value();
  lv->add_values()->set_string_value("keyboard");
  lv->add_values()->set_string_value("mouse");
  std::map<std::string, std::any> out;
  evrp::sdk::fromProto(out, s);
  auto kinds = std::any_cast<const std::vector<evrp::sdk::DeviceKind>&>(
      out.at("kinds"));
  ASSERT_EQ(kinds.size(), 2u);
  EXPECT_EQ(kinds[0], evrp::sdk::DeviceKind::kKeyboard);
  EXPECT_EQ(kinds[1], evrp::sdk::DeviceKind::kMouse);
}

TEST(ToFromProto, RoundtripKindsVector) {
  MemorySetting m;
  m.insert("kinds", std::vector<evrp::sdk::DeviceKind>{
                        evrp::sdk::DeviceKind::kTouchpad});
  google::protobuf::Struct s;
  evrp::sdk::toProto(m.snapshot(), &s);
  std::map<std::string, std::any> out;
  evrp::sdk::fromProto(out, s);
  auto kinds = std::any_cast<const std::vector<evrp::sdk::DeviceKind>&>(
      out.at("kinds"));
  ASSERT_EQ(kinds.size(), 1u);
  EXPECT_EQ(kinds[0], evrp::sdk::DeviceKind::kTouchpad);
}
