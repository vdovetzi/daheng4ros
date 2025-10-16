#include "daheng4ros/camera.hpp"

#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/calib3d.hpp>

#include <image_transport/image_transport.hpp>
#include <opencv2/core/mat.hpp>
#include <rcl_interfaces/msg/parameter_descriptor.hpp>
#include <rclcpp/rclcpp.hpp>

using Camera = sdk_daheng::Camera;
using OnSetParametersCallbackHandle =
    rclcpp::node_interfaces::OnSetParametersCallbackHandle;

std::shared_ptr<Camera> cam;

cv::Mat rgb_image;

int32_t width = 0;
int32_t height = 0;

int32_t contrastFactor = 0;
int32_t brightnessFactor = 0;
float sharpenFactor = 0.;

rcl_interfaces::msg::SetParametersResult
parameters_callback(const std::vector<rclcpp::Parameter> &parameters) {
  auto result = rcl_interfaces::msg::SetParametersResult();
  result.successful = true;

  for (const auto &param : parameters) {
    const std::string &param_name = param.get_name();

    if (param_name == "brightness") {
      brightnessFactor = param.as_int();
    } else if (param_name == "contrast") {
      contrastFactor = param.as_int();
    } else if (param_name == "sharpness") {
      sharpenFactor = param.as_double();
    } else if (param_name == "gain") {
      cam->SetGain(param.as_double());
    } else if (param_name == "exposure") {
      cam->SetExposure(param.as_double());
    }
  }
  return result;
}

void applyEnhancements(unsigned char *p_rgb) {
  DxContrast(p_rgb, p_rgb, width * height * 3, contrastFactor);
  DxBrightness(p_rgb, p_rgb, width * height * 3, brightnessFactor);
  DxSharpen24B(p_rgb, p_rgb, width, height, sharpenFactor);
}

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  rclcpp::Node::SharedPtr node = rclcpp::Node::make_shared("image_publisher");

  // #############Parameters#############
  node->declare_parameter<int32_t>("fps");
  node->declare_parameter<float>("exposure");
  node->declare_parameter<double>("gain");
  node->declare_parameter<int32_t>("width", 640);
  node->declare_parameter<int32_t>("height", 480);
  node->declare_parameter<std::vector<double>>("K", std::vector<double>());
  node->declare_parameter<std::vector<double>>("D", std::vector<double>());
  node->declare_parameter<std::string>("camera_frame", "daheng_cam");
  node->declare_parameter<std::string>("camera_topic", "daheng/image");

  rcl_interfaces::msg::ParameterDescriptor brightness_desc;
  brightness_desc.description = "Brightness parameter";
  rcl_interfaces::msg::IntegerRange brightness_range;
  brightness_range.from_value = -150;
  brightness_range.to_value = 150;
  brightness_range.step = 1;
  brightness_desc.integer_range.push_back(brightness_range);
  node->declare_parameter<int32_t>("brightness", brightness_desc);

  rcl_interfaces::msg::ParameterDescriptor contrast_desc;
  brightness_desc.description = "Contrast parameter";
  rcl_interfaces::msg::IntegerRange contrast_range;
  contrast_range.from_value = -50;
  contrast_range.to_value = 100;
  contrast_range.step = 1;
  contrast_desc.integer_range.push_back(contrast_range);
  node->declare_parameter<int32_t>("contrast", contrast_desc);

  rcl_interfaces::msg::ParameterDescriptor sharpness_desc;
  sharpness_desc.description = "Sharpness parameter";
  rcl_interfaces::msg::FloatingPointRange sharpness_range;
  sharpness_range.from_value = 0.0;
  sharpness_range.to_value = 5.0;
  sharpness_range.step = 0.5;
  sharpness_desc.floating_point_range.push_back(sharpness_range);
  node->declare_parameter<float>("sharpness", sharpness_desc);

  int32_t fps = node->get_parameter("fps").as_int();
  double exposure = node->get_parameter("exposure").as_double();
  double gain = node->get_parameter("gain").as_double();
  std::vector<double> K = node->get_parameter("K").as_double_array();
  std::vector<double> D = node->get_parameter("D").as_double_array();
  std::string camera_frame = node->get_parameter("camera_frame").as_string();
  std::string camera_topic = node->get_parameter("camera_topic").as_string();
  node->get_parameter("width", width);
  node->get_parameter("height", height);
  node->get_parameter("brightness", brightnessFactor);
  node->get_parameter("contrast", contrastFactor);
  node->get_parameter("sharpness", sharpenFactor);

  OnSetParametersCallbackHandle::SharedPtr param_callback_handle =
      node->add_on_set_parameters_callback(parameters_callback);
  // ####################################
  cv::Mat camera_matrix = cv::Mat(3, 3, CV_64F);
  cv::Mat dist_coeffs = cv::Mat(D.size(), 1, CV_64F);
  if (!K.empty() && !D.empty()) {
    for (size_t i = 0; i < K.size(); ++i) {
      camera_matrix.at<double>(i / 3, i % 3) = K[i];
    }

    for (size_t i = 0; i < D.size(); ++i) {
      dist_coeffs.at<double>(i) = D[i];
    }
  }

  cam = std::make_shared<Camera>();

  auto [dim1, dim2] = cam->GetFrameDimensions();

  // Settings
  cam->SetFrameRate(fps);
  cam->SetExposure(exposure);
  cam->SetGain(gain);

  std::shared_ptr<image_transport::ImageTransport> image_transport =
      std::make_shared<image_transport::ImageTransport>(node);
  image_transport::Publisher image_pub =
      image_transport->advertise(camera_topic, 10);

  cv::Mat map1, map2;
  if (!camera_matrix.empty() && !dist_coeffs.empty()) {
    cv::initUndistortRectifyMap(camera_matrix, dist_coeffs, cv::Mat(),
                                camera_matrix, cv::Size(width, height),
                                CV_32FC1, map1, map2);
  }

  cam->SetCallback([&](unsigned char *p) {
    if (p == nullptr) {
      RCLCPP_WARN(node->get_logger(), "Received null image buffer");
      return;
    }

    try {
      rgb_image = cv::Mat(dim2, dim1, CV_8UC3, p);

      cv::resize(rgb_image, rgb_image, {width, height});

      applyEnhancements(rgb_image.data);

      if (!camera_matrix.empty() && !dist_coeffs.empty()) {
        cv::remap(rgb_image, rgb_image, map1, map2, cv::INTER_LINEAR);
      }

      auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "rgb8", rgb_image)
                     .toImageMsg();

      msg->header.stamp = node->now();
      msg->header.frame_id = camera_frame;

      image_pub.publish(msg);

    } catch (const std::exception &e) {
      RCLCPP_ERROR(node->get_logger(), "Error processing image: %s", e.what());
    }
  });

  rclcpp::spin(node);

  return 0;
}