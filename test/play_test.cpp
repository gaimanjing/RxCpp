#include "rxcpp/rx.hpp"

#include <gtest/gtest.h>

class PlayTestSuite : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(PlayTestSuite, subscribe) {
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

TEST_F(PlayTestSuite, subscribe_on_new_thread) {
  // Given
  auto values1 = rxcpp::observable<>::range(1, 2);
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

TEST_F(PlayTestSuite, observe_on_new_thread) {
  // Given
  auto values1 = rxcpp::observable<>::range(1, 2);
  auto ret = false;

  auto threads = rxcpp::observe_on_new_thread();

  // When
  values1.observe_on(threads).subscribe(
      [](int v) {
        printf("OnNext, 1: %d\n", v);
        printf("");
      },
      []() {
        printf("OnCompleted, 1\n");
        printf("");
      });

  auto threads2 = rxcpp::observe_on_new_thread();
  values1.observe_on(threads2).subscribe(
      [](int v) {
        printf("OnNext, 2: %d\n", v);
        printf("");
      },
      [&]() {
        printf("OnCompleted, 2\n");
        printf("");
      });

  auto threads3 = rxcpp::observe_on_new_thread();
  values1.observe_on(threads3).as_blocking().subscribe(
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

TEST_F(PlayTestSuite, observe_on_run_loop) {
  // Given
  rxcpp::subjects::subject<int> a_subject;
  auto values1 = a_subject.get_observable();

  auto main_run_loop = rxcpp::schedulers::run_loop();
  auto threads = rxcpp::observe_on_run_loop(main_run_loop);

  auto ret = false;

  values1.observe_on(threads).subscribe(
      [](int v) {
        printf("OnNext, 1: %d\n", v);
        printf("");
      },
      [&]() {
        printf("OnCompleted, 1\n");
        printf("");
        ret = true;
      });

  // When
  a_subject.get_subscriber().on_next(1);
  a_subject.get_subscriber().on_next(2);
  a_subject.get_subscriber().on_completed();

  while (!main_run_loop.empty()) {
    main_run_loop.dispatch();
  }

  // Then
  EXPECT_TRUE(ret);
}

TEST_F(PlayTestSuite, observe_on_event_loop) {
  // Given
  rxcpp::subjects::subject<int> a_subject;
  rxcpp::subjects::subject<int> block_subject;
  auto values1 = a_subject.get_observable();

  auto threads = rxcpp::observe_on_event_loop();

  auto ret = false;

  values1.observe_on(threads).subscribe(
      [](int v) {
        // run in new thread
        printf("OnNext, 1: %d\n", v);
        printf("");
      },
      [&]() {
        // run in new thread
        printf("OnCompleted, 1\n");
        printf("");
        ret = true;
        block_subject.get_subscriber().on_completed();
      });

  // When
  a_subject.get_subscriber().on_next(1);
  a_subject.get_subscriber().on_next(2);
  a_subject.get_subscriber().on_completed();

  block_subject.get_observable().as_blocking().subscribe();

  // Then
  EXPECT_TRUE(ret);
}