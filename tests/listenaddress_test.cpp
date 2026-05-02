#include "evrp/sdk/listenaddress.h"

#include <gtest/gtest.h>

TEST(ListenAddress, RejectsNullOut) {
  EXPECT_FALSE(evrp::sdk::parseListenPort("127.0.0.1:1", nullptr));
}

TEST(ListenAddress, RejectsEmpty) {
  std::uint16_t p = 0;
  EXPECT_FALSE(evrp::sdk::parseListenPort("", &p));
}

TEST(ListenAddress, ParsesIpv4HostPort) {
  std::uint16_t p = 0;
  ASSERT_TRUE(evrp::sdk::parseListenPort("127.0.0.1:50051", &p));
  EXPECT_EQ(p, 50051);
}

TEST(ListenAddress, ParsesBarePortAfterColon) {
  std::uint16_t p = 0;
  ASSERT_TRUE(evrp::sdk::parseListenPort(":65535", &p));
  EXPECT_EQ(p, 65535);
}

TEST(ListenAddress, ParsesBracketedIpv6Style) {
  std::uint16_t p = 0;
  ASSERT_TRUE(evrp::sdk::parseListenPort("[::1]:9", &p));
  EXPECT_EQ(p, 9);
}

TEST(ListenAddress, RejectsBracketedWithoutPort) {
  std::uint16_t p = 0;
  EXPECT_FALSE(evrp::sdk::parseListenPort("[::1]", &p));
}

TEST(ListenAddress, RejectsBracketedMissingColonBeforePort) {
  std::uint16_t p = 0;
  EXPECT_FALSE(evrp::sdk::parseListenPort("[::1]50051", &p));
}

TEST(ListenAddress, RejectsNoColon) {
  std::uint16_t p = 0;
  EXPECT_FALSE(evrp::sdk::parseListenPort("127.0.0.1", &p));
}

TEST(ListenAddress, RejectsPortZero) {
  std::uint16_t p = 0;
  EXPECT_FALSE(evrp::sdk::parseListenPort("0.0.0.0:0", &p));
}

TEST(ListenAddress, RejectsPortTooLarge) {
  std::uint16_t p = 0;
  EXPECT_FALSE(evrp::sdk::parseListenPort("0.0.0.0:65536", &p));
}

TEST(ListenAddress, RejectsNonNumericPort) {
  std::uint16_t p = 0;
  EXPECT_FALSE(evrp::sdk::parseListenPort("0.0.0.0:abc", &p));
}

TEST(ListenAddress, RejectsEmptyPortAfterColon) {
  std::uint16_t p = 0;
  EXPECT_FALSE(evrp::sdk::parseListenPort("0.0.0.0:", &p));
}
