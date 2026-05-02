#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/setting/overlaysetting.h"

#include <gtest/gtest.h>

TEST(OverlaySetting, TopShadowsBelow) {
  MemorySetting base;
  base.insert("k", 1);
  MemorySetting top;
  top.insert("k", 2);
  OverlaySetting overlay(&top);
  overlay.addLower(&base);
  EXPECT_EQ(overlay.get<int>("k", 0), 2);
}

TEST(OverlaySetting, FallsThroughWhenTopMisses) {
  MemorySetting base;
  base.insert("k", 1);
  MemorySetting top;
  OverlaySetting overlay(&top);
  overlay.addLower(&base);
  EXPECT_EQ(overlay.get<int>("k", 0), 1);
}

TEST(OverlaySetting, InsertWritesOnlyTop) {
  MemorySetting base;
  base.insert("k", 1);
  MemorySetting top;
  OverlaySetting overlay(&top);
  overlay.addLower(&base);
  overlay.insert("k", 99);
  EXPECT_EQ(top.get<int>("k", 0), 99);
  EXPECT_EQ(base.get<int>("k", 0), 1);
}

TEST(OverlaySetting, ChainOrder) {
  MemorySetting a;
  a.insert("k", 1);
  MemorySetting b;
  b.insert("k", 2);
  MemorySetting top;
  OverlaySetting overlay(&top);
  overlay.addLower({&b, &a});
  EXPECT_EQ(overlay.get<int>("k", 0), 2);
}

TEST(OverlaySetting, AddLowerMergesDistinctKeys) {
  MemorySetting l0, l1, top;
  l0.insert("a", 0);
  l1.insert("b", 1);
  OverlaySetting o(&top);
  o.addLower({&l1, &l0});
  EXPECT_EQ(o.get<int>("a", -1), 0);
  EXPECT_EQ(o.get<int>("b", -1), 1);
}

TEST(OverlaySetting, FirstLowerWinsWhenSameKey) {
  MemorySetting l0, l1, top;
  l0.insert("k", 0);
  l1.insert("k", 1);
  OverlaySetting o(&top);
  o.addLower({&l1, &l0});
  EXPECT_EQ(o.get<int>("k", -1), 1);
}

TEST(OverlaySetting, ConstructWithLowers) {
  MemorySetting base;
  base.insert("k", 42);
  MemorySetting top;
  OverlaySetting overlay(&top, {&base});
  EXPECT_EQ(overlay.get<int>("k", 0), 42);
}

TEST(OverlaySetting, NullTopUsesOwnedMemoryLayer) {
  MemorySetting lower;
  lower.insert("k", 1);
  OverlaySetting overlay(nullptr);
  overlay.addLower(&lower);
  EXPECT_EQ(overlay.get<int>("k", 0), 1);
  overlay.insert("k", 2);
  EXPECT_EQ(overlay.get<int>("k", 0), 2);
  EXPECT_EQ(lower.get<int>("k", 0), 1);
}

TEST(OverlaySetting, NullTopWithLowersInConstructor) {
  MemorySetting base;
  base.insert("k", 7);
  OverlaySetting overlay(nullptr, {&base});
  EXPECT_EQ(overlay.get<int>("k", 0), 7);
}
