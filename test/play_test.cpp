#include "rxcpp/rx.hpp"

#include "helper.hpp"

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

TEST_F(PlayTestSuite, serialize_one_worker) {
  // Given
  rxcpp::subjects::subject<int> a_subject;
  rxcpp::subjects::subject<int> block_subject;
  auto values1 = a_subject.get_observable();

  // auto threads = rxcpp::serialize_event_loop();
  // auto threads = rxcpp::serialize_new_thread();
  // auto threads = rxcpp::serialize_same_worker();

  auto threads = rxcpp::synchronize_event_loop();

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

TEST_F(PlayTestSuite, SchedulerOne) {
  //---------- Get a Coordination
  auto coordination = rxcpp::serialize_new_thread();

  //------- Create a Worker instance
  auto worker = coordination.create_coordinator().get_worker();

  auto sub_action = rxcpp::schedulers::make_action(
      [](const rxcpp::schedulers::schedulable &) {
        std::cout << "Action Executed in Thread # : "
                  << std::this_thread::get_id() << std::endl;
      });

  auto scheduled = rxcpp::schedulers::make_schedulable(worker, sub_action);
  scheduled.schedule();
}

TEST_F(PlayTestSuite, SchedulerTwo) {
  std::cout << "#0 Thread #" << std::this_thread::get_id() << std::endl;
  auto coordination = rxcpp::identity_current_thread();
  // auto coordination = rxcpp::observe_on_new_thread();

  auto worker = coordination.create_coordinator().get_worker();
  auto start = coordination.now() + std::chrono::milliseconds(1);
  auto period = std::chrono::milliseconds(1);
  auto values = rxcpp::observable<>::interval(start, period)
                    .take(5)
                    .replay(2, coordination);

  worker.schedule([&](const rxcpp::schedulers::schedulable &) {
    values.subscribe(

        [](long v) {
          std::cout << "#1 Thread #" << std::this_thread::get_id()
                    << "; on_next: " << v << std::endl;
        },

        []() { printf("#1 --- OnCompleted\n"); });
  });

  worker.schedule([&](const rxcpp::schedulers::schedulable &) {
    values.subscribe(

        [](long v) {
          std::cout << "#2 Thread #" << std::this_thread::get_id()
                    << "; on_next: " << v << std::endl;
        },

        []() { printf("#2 --- OnCompleted\n"); });
  });

  // Start emitting

  worker.schedule(
      [&](const rxcpp::schedulers::schedulable &) { values.connect(); });

  // Add blocking subscription to see results

  values.as_blocking().subscribe();
}

TEST_F(PlayTestSuite, subject_no_subscriber) {
  // Given
  rxcpp::subjects::subject<int> a_subject;
  auto values1 = a_subject.get_observable();

  // When
  a_subject.get_subscriber().on_next(1);
  a_subject.get_subscriber().on_next(2);
  a_subject.get_subscriber().on_completed();

  // Then
}

TEST_F(PlayTestSuite, subject) {
  // Given
  rxcpp::subjects::subject<int> a_subject;
  auto values1 = a_subject.get_observable();

  auto ret = false;
  values1.subscribe(
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

  // Then
  EXPECT_TRUE(ret);
}

TEST_F(PlayTestSuite, range_map_observe_on) {
  auto values =
      rxcpp::observable<>::range(1, 4).map([](int v) { return v * v; });

  std::cout << "Main Thread id => " << std::this_thread::get_id() << std::endl;

  values.observe_on(rxcpp::synchronize_new_thread())
      .as_blocking()
      .subscribe(
          [](int v) {
            std::cout << "Observable Thread id => "
                      << std::this_thread::get_id() << "  " << v << std::endl;
          },
          []() { std::cout << "OnCompleted" << std::endl; });

  std::cout << "Main Thread id => " << std::this_thread::get_id() << std::endl;
}

TEST_F(PlayTestSuite, multi_observe_on_new_thread) {
  auto values =
      rxcpp::observable<>::range(1, 4)
          .observe_on(rxcpp::observe_on_new_thread())
          .map([](int v) {
            print_indentation(1);
            std::cout << "#1, thread id => " << std::this_thread::get_id()
                      << "  " << v << std::endl;
            return v;
          })
          .observe_on(rxcpp::observe_on_new_thread())
          .map([](int v) {
            print_indentation(2);
            std::cout << "#2, thread id => " << std::this_thread::get_id()
                      << "  " << v << std::endl;
            return v;
          })
          .observe_on(rxcpp::observe_on_new_thread())
          .map([](int v) {
            print_indentation(3);
            std::cout << "#3, thread id => " << std::this_thread::get_id()
                      << "  " << v << std::endl;
            return v;
          });

  std::cout << "#0, Main thread id => " << std::this_thread::get_id()
            << std::endl;
  values.as_blocking().subscribe();
}

TEST_F(PlayTestSuite, muliti_observe_on_new_thread) {
  auto values = rxcpp::observable<>::interval(std::chrono::milliseconds(50),
                                              rxcpp::observe_on_new_thread())
                    .take(5)
                    .publish();

  auto threads = rxcpp::observe_on_new_thread();
  values.observe_on(threads).subscribe(
      [](int v) {
        print_indentation(1);
        std::cout << "#1, thread: #" << std::this_thread::get_id()
                  << ", on_next: " << v << std::endl;
      },
      []() {
        print_indentation(1);
        std::cout << "#1, thread: #" << std::this_thread::get_id()
                  << ", on_completed: " << std::endl;
      });
  values.observe_on(threads).subscribe(
      [](int v) {
        print_indentation(2);
        std::cout << "#2, thread: #" << std::this_thread::get_id()
                  << ", on_next: " << v << std::endl;
      },
      []() {
        print_indentation(2);
        std::cout << "#2, thread: #" << std::this_thread::get_id()
                  << ", on_completed: " << std::endl;
      });

  // When
  values.connect();
  values.as_blocking().subscribe();
}

TEST_F(PlayTestSuite, as_blocking) {
  auto values = rxcpp::observable<>::interval(std::chrono::milliseconds(50),
                                              rxcpp::observe_on_new_thread())
                    .take(5)
                    .publish();

  // When
  values.connect();
  values.as_blocking().subscribe();
}
