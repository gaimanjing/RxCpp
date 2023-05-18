#include "rxcpp/rx.hpp"

#include "helper.hpp"

#include <gtest/gtest.h>

class PlaySuite : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(PlaySuite, app) {
  std::cout << "#0, main thread: #" << std::this_thread::get_id() << std::endl;
  // ui event
  auto ui_event_sequence =
      rxcpp::observable<>::interval(std::chrono::milliseconds(50),
                                    rxcpp::observe_on_new_thread())
          .take(5)
          .publish();

  bool finished = false;

  // model event
  rxcpp::subjects::subject<int> model_subject;
  auto model_coordination = rxcpp::observe_on_new_thread();
  ui_event_sequence.observe_on(model_coordination)
      .subscribe(
          [&](int v) {
            print_indentation(1);
            std::cout << "#1, model thread: #" << std::this_thread::get_id()
                      << ", on_next: " << v << std::endl;
            model_subject.get_subscriber().on_next(v);
          },
          [&]() {
            print_indentation(1);
            std::cout << "#1, model thread: #" << std::this_thread::get_id()
                      << ", on_completed: " << std::endl;
            model_subject.get_subscriber().on_completed();
          });

  // main loop
  auto run_loop = rxcpp::schedulers::run_loop();
  auto main_coordination = rxcpp::observe_on_run_loop(run_loop);

  auto main_observer = model_subject.get_observable()
                           .observe_on(main_coordination)
                           .subscribe(
                               [](int v) {
                                 print_indentation(2);
                                 std::cout << "#2, observer thread: #"
                                           << std::this_thread::get_id()
                                           << ", on_next: " << v << std::endl;
                               },
                               [&]() {
                                 print_indentation(2);
                                 std::cout << "#2, observer thread: #"
                                           << std::this_thread::get_id()
                                           << ", on_completed: " << std::endl;
                                 finished = true;
                               });

  // When
  ui_event_sequence.connect();

  // loop
  print_indentation(3);
  std::cout << "#3, before run loop" << std::endl;
  while (!run_loop.empty() || !finished) {
    run_loop.dispatch();
  }
  print_indentation(3);
  std::cout << "#3, after run loop" << std::endl;
}