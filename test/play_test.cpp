#include "rxcpp/rx.hpp"

#include <gtest/gtest.h>

class PlayTestSuite : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(PlayTestSuite, Thread_safe_1) {
  // Given
  auto values1 = rxcpp::observable<>::range(1, 5);
  auto ret = false;

  // When
  values1.subscribe(
      [](int v) {
        printf("OnNext, 1: %d\n", v);
        printf("");
      },
      []() {
        printf("OnCompleted, 1\n");
        printf("");
      });

  values1.subscribe(
      [](int v) {
        printf("OnNext, 2: %d\n", v);
        printf("");
      },
      [&]() {
        printf("OnCompleted, 2\n");
        printf("");
        ret = true;
      });

  // Then
  EXPECT_TRUE(ret);
}

TEST_F(PlayTestSuite, Thread_safe_2) {
  // Given
  auto values1 = rxcpp::observable<>::range(1, 5);
  auto ret = false;

  auto threads = rxcpp::observe_on_new_thread();

  // When
  values1.subscribe_on(threads).subscribe(
      [](int v) {
        printf("OnNext, 1: %d\n", v);
        printf("");
      },
      []() {
        printf("OnCompleted, 1\n");
        printf("");
      });

  auto threads2 = rxcpp::observe_on_new_thread();
  values1.subscribe_on(threads2).subscribe(
      [](int v) {
        printf("OnNext, 2: %d\n", v);
        printf("");
      },
      [&]() {
        printf("OnCompleted, 2\n");
        printf("");
      });

  auto threads3 = rxcpp::observe_on_new_thread();
  values1.subscribe_on(threads3).as_blocking().subscribe(
      [](int v) {
        printf("OnNext, 3: %d\n", v);
        printf("");
      },
      [&]() {
        printf("OnCompleted, 3\n");
        printf("");
        ret = true;
      });

  // Then
  EXPECT_TRUE(ret);
}