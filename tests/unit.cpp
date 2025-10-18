#include "rclcpp/rclcpp.hpp"
#include <gtest/gtest.h>
#include <thread>

#include "daheng4ros/camera.hpp"

using namespace sdk_daheng;
using namespace std::chrono_literals;

class CameraTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Setup before each test
  }

  void TearDown() override
  {
    // Cleanup after each test
  }
};

// Test basic construction and destruction
TEST_F(CameraTest, ConstructorDestructor) {
  Camera camera;
  SUCCEED();
}

TEST_F(CameraTest, SetCallback) {
  Camera camera;

  bool callback_called = false;

  camera.SetCallback([&callback_called](unsigned char *data [[maybe_unused]]) {
      callback_called = true;
  });

  sleep(3);

  ASSERT_TRUE(callback_called);
  SUCCEED();
}

TEST_F(CameraTest, ExposureSettings) {
  Camera camera;

  camera.SetExposure(10000.0f);

  ASSERT_EQ(camera.GetExposure(), 10000.0f);

  camera.SetExposure(5000.0f);

  ASSERT_EQ(camera.GetExposure(), 5000.0f);
  SUCCEED();
}

// TODO: add FPS getter
TEST_F(CameraTest, FrameRateSettings) {
  Camera camera;

  // Test various frame rates
  camera.SetFrameRate(30.0f);
  camera.SetFrameRate(60.0f);

  SUCCEED();
}

// TODO: add gain getter
TEST_F(CameraTest, GainSettings) {
  Camera camera;

  camera.SetGain(1.0f);
  camera.SetGain(10.0f);

  SUCCEED();
}

//TODO: segfault
// TEST_F(CameraTest, CallbackThreadSafety) {
//   Camera camera;
//   const size_t N_THREADS = 2;
//   const size_t N_TIMES = 100;

//   std::atomic<int32_t> cnt{0};
//   std::vector<std::thread> threads;
//   std::vector<bool> inits(N_THREADS, false);

//   // Create threads
//   for (size_t i = 0; i < N_THREADS; ++i) {
//     threads.emplace_back([&camera, &cnt, &inits, i]() {
//         for (size_t j = 0; j < N_TIMES; ++j) {
//           camera.SetCallback(
//             [&cnt, &inits, i](unsigned char *data [[maybe_unused]]) {
//               if (!inits[i]) {
//                 cnt.fetch_add(1, std::memory_order_relaxed);
//                 inits[i] = true;
//               }
//             });
//         }
//     });
//     sleep(1);
//   }

//   for (auto & t : threads) {
//     if (t.joinable()) {
//       t.join();
//     }
//   }
//   ASSERT_EQ(cnt, N_THREADS);
//   SUCCEED();
// }

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);

  // Initialize ROS2 if needed for any tests
  rclcpp::init(argc, argv);

  int result = RUN_ALL_TESTS();

  rclcpp::shutdown();
  return result;
}
