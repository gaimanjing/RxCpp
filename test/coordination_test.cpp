#include "rxcpp/rx.hpp"

#include "helper.hpp"

#include <gtest/gtest.h>

class CoordinationTestSuite : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

template <class T> void test_with_thread(T &thread) {
  // Given
  auto values1 = rxcpp::observable<>::range(1, 5).publish();
  auto ret = false;

  // When
  values1.observe_on(thread).subscribe(
      [](int v) {
        print_indentation(1);
        printf("OnNext, 1: %d\n", v);
        printf("");
      },
      []() {
        print_indentation(1);
        printf("OnCompleted, 1\n");
        printf("");
      });
  values1.observe_on(thread).subscribe(
      [](int v) {
        print_indentation(2);
        printf("OnNext, 2: %d\n", v);
        printf("");
      },
      []() {
        print_indentation(2);
        printf("OnCompleted, 2\n");
        printf("");
      });
  values1.observe_on(thread).subscribe(
      [](int v) {
        print_indentation(3);
        printf("OnNext, 3: %d\n", v);
        printf("");
      },
      []() {
        print_indentation(3);
        printf("OnCompleted, 3\n");
        printf("");
      });

  // Then
  values1.connect();
  values1.as_blocking().subscribe();
}

// identity_one_worker
TEST_F(CoordinationTestSuite, identity_immediate) {
  auto thread = rxcpp::identity_immediate();
  test_with_thread(thread);
}

TEST_F(CoordinationTestSuite, identity_current_thread) {
  auto thread = rxcpp::identity_current_thread();
  test_with_thread(thread);
}

// synchronize_in_one_worker
TEST_F(CoordinationTestSuite, synchronize_event_loop) {
  auto thread = rxcpp::synchronize_event_loop();
  test_with_thread(thread);
}

TEST_F(CoordinationTestSuite, synchronize_new_thread) {
  auto thread = rxcpp::synchronize_new_thread();
  test_with_thread(thread);
}

// serialize_one_worker
TEST_F(CoordinationTestSuite, serialize_event_loop) {
  auto thread = rxcpp::serialize_event_loop();
  test_with_thread(thread);
}
TEST_F(CoordinationTestSuite, serialize_new_thread) {
  auto thread = rxcpp::serialize_new_thread();
  test_with_thread(thread);
}

// observe_on_one_worker
TEST_F(CoordinationTestSuite, observe_on_event_loop) {
  auto thread = rxcpp::observe_on_event_loop();
  test_with_thread(thread);
}
TEST_F(CoordinationTestSuite, observe_on_new_thread) {
  auto thread = rxcpp::observe_on_new_thread();
  test_with_thread(thread);
}