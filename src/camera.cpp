#include "daheng4ros/camera.hpp"
#include <iostream>

namespace sdk_daheng {
Camera::Camera() {
  GX_STATUS emStatus = GX_STATUS_SUCCESS;
  uint32_t ui32DeviceNum = 0;

  // Initialize libary
  emStatus = GXInitLib();
  if (emStatus != GX_STATUS_SUCCESS) {
    GetErrorString(emStatus);
    std::abort();
  }

  // Get device enumerated number
  emStatus = GXUpdateAllDeviceList(&ui32DeviceNum, 1000);
  if (emStatus != GX_STATUS_SUCCESS) {
    GetErrorString(emStatus);
    GXCloseLib();
    std::abort();
  }

  // If no device found, app exit
  if (ui32DeviceNum <= 0) {
    std::cerr << "<No device found>\n";
    GXCloseLib();
    std::abort();
  }

  // Open first device enumerated
  emStatus = GXOpenDeviceByIndex(1, &g_hDevice);
  if (emStatus != GX_STATUS_SUCCESS) {
    GetErrorString(emStatus);
    GXCloseLib();
    std::abort();
  }

  // Get Device Info
  std::cout << "***********************************************\n";
  // Get libary version
  std::cout << "<Libary Version : " << GXGetLibVersion() << ">\n";

  // Get string length of Vendor name
  GX_STRING_VALUE stStrVendorName;
  emStatus = GXGetStringValue(g_hDevice, "DeviceVendorName", &stStrVendorName);
  GX_VERIFY_EXIT(emStatus);
  std::cout << "<Vendor Name : " << stStrVendorName.strCurValue << ">\n";

  // Get string length of Model name
  GX_STRING_VALUE stStrModelName;
  emStatus = GXGetStringValue(g_hDevice, "DeviceModelName", &stStrModelName);
  GX_VERIFY_EXIT(emStatus);
  std::cout << "<Model Name : " << stStrModelName.strCurValue << ">\n";

  // Get string length of Serial number
  GX_STRING_VALUE stStrSerialNumber;
  emStatus =
      GXGetStringValue(g_hDevice, "DeviceSerialNumber", &stStrSerialNumber);
  GX_VERIFY_EXIT(emStatus);
  std::cout << "<Serial Number : " << stStrSerialNumber.strCurValue << ">\n";

  // Get string length of Device version
  GX_STRING_VALUE stStrDeviceVersion;
  emStatus = GXGetStringValue(g_hDevice, "DeviceVersion", &stStrDeviceVersion);
  GX_VERIFY_EXIT(emStatus);
  std::cout << "<Device Version : " << stStrDeviceVersion.strCurValue << ">\n";

  std::cout << "***********************************************\n";

  // Get the type of Bayer conversion. whether is a color camera.
  GX_NODE_ACCESS_MODE emAccessMode;
  emStatus = GXGetNodeAccessMode(g_hDevice, "PixelColorFilter", &emAccessMode);
  GX_VERIFY_EXIT(emStatus);
  g_bColorFilter = ((emAccessMode == GX_NODE_ACCESS_MODE_WO) ||
                    (emAccessMode == GX_NODE_ACCESS_MODE_RO) ||
                    (emAccessMode == GX_NODE_ACCESS_MODE_RW))
                       ? true
                       : false;

  // This app only support color cameras
  if (!g_bColorFilter) {
    std::cout << "<This app only support color cameras! App Exit!>\n";
    GXCloseDevice(g_hDevice);
    g_hDevice = NULL;
    GXCloseLib();
    std::abort();
  } else {
    GX_ENUM_VALUE emValue;
    emStatus = GXGetEnumValue(g_hDevice, "PixelColorFilter", &emValue);
    g_i64ColorFilter = emValue.stCurValue.nCurValue;
    GX_VERIFY_EXIT(emStatus);
  }

  uint32_t nDSNum = 0;
  emStatus = GXGetDataStreamNumFromDev(g_hDevice, &nDSNum);
  GX_VERIFY_EXIT(emStatus);

  if (nDSNum < 1) {
    std::cout << "Failed to obtain the number of streams\n";
    std::exit(GX_STATUS_ERROR);
  }

  GX_DS_HANDLE phDS = NULL;
  emStatus = GXGetDataStreamHandleFromDev(g_hDevice, 1, &phDS);
  GX_VERIFY_EXIT(emStatus);

  emStatus = GXGetPayLoadSize(phDS, &g_nPayloadSize);
  GX_VERIFY_EXIT(emStatus);

  // Set acquisition mode
  emStatus = GXSetEnumValueByString(g_hDevice, "AcquisitionMode", "Continuous");
  GX_VERIFY_EXIT(emStatus);

  // Set trigger mode
  emStatus = GXSetEnumValueByString(g_hDevice, "TriggerMode", "Off");
  GX_VERIFY_EXIT(emStatus);

  // Set user fps
  emStatus =
      GXSetEnumValueByString(g_hDevice, "AcquisitionFrameRateMode", "On");
  GX_VERIFY_EXIT(emStatus);

  // Set default 50 fps (max)
  emStatus = GXSetFloatValue(g_hDevice, "AcquisitionFrameRate", 50.0);
  GX_VERIFY_EXIT(emStatus);

  // Set buffer quantity of acquisition queue
  uint64_t nBufferNum = ACQ_BUFFER_NUM;
  emStatus = GXSetAcqusitionBufferNumber(g_hDevice, nBufferNum);
  GX_VERIFY_EXIT(emStatus);

  GX_NODE_ACCESS_MODE emStreamTransferSizeIm;
  emStatus = GXGetNodeAccessMode(g_hDevice, "StreamTransferSize",
                                 &emStreamTransferSizeIm);
  GX_VERIFY_EXIT(emStatus);
  bool bStreamTransferSize =
      ((emStreamTransferSizeIm == GX_NODE_ACCESS_MODE_RO) ||
       (emStreamTransferSizeIm == GX_NODE_ACCESS_MODE_WO) ||
       (emStreamTransferSizeIm == GX_NODE_ACCESS_MODE_RW))
          ? true
          : false;

  if (bStreamTransferSize) {
    // Set size of data transfer block
    emStatus =
        GXSetIntValue(g_hDevice, "StreamTransferSize", ACQ_TRANSFER_SIZE);
    GX_VERIFY_EXIT(emStatus);
  }

  GX_NODE_ACCESS_MODE emStreamTransferUrbIm;
  emStatus = GXGetNodeAccessMode(g_hDevice, "StreamTransferNumberUrb",
                                 &emStreamTransferUrbIm);
  GX_VERIFY_EXIT(emStatus);
  bool bStreamTransferNumberUrb =
      ((emStreamTransferUrbIm == GX_NODE_ACCESS_MODE_RO) ||
       (emStreamTransferUrbIm == GX_NODE_ACCESS_MODE_WO) ||
       (emStreamTransferUrbIm == GX_NODE_ACCESS_MODE_RW))
          ? true
          : false;

  if (bStreamTransferNumberUrb) {
    // Set qty. of data transfer block
    emStatus = GXSetIntValue(g_hDevice, "StreamTransferNumberUrb",
                             ACQ_TRANSFER_NUMBER_URB);
    GX_VERIFY_EXIT(emStatus);
  }

  // Set Balance White Mode : Continuous
  emStatus = GXSetEnumValueByString(g_hDevice, "BalanceWhiteAuto", "Once");
  GX_VERIFY_EXIT(emStatus);

  // Allocate the memory for pixel format transform
  PreForAcquisition();

  Acquire();
}

Camera::~Camera() { Release(); }

void Camera::Acquire() {
  GX_STATUS emStatus = GX_STATUS_SUCCESS;

  emStatus = GXStreamOn(g_hDevice);
  if (emStatus != GX_STATUS_SUCCESS) {
    // Release the memory allocated
    UnPreForAcquisition();
    GX_VERIFY_EXIT(emStatus);
  }

  aq_thread = std::thread(&Camera::ProcGetImage, this);
}

void Camera::SetCallback(const std::function<void(unsigned char *)> &cb) {
  std::unique_lock<std::mutex> lock(cb_mutex);
  callback = cb;
}

void Camera::Release() {
  GX_STATUS emStatus = GX_STATUS_SUCCESS;

  // Stop Acquisition thread
  g_bAcquisitionFlag = false;

  aq_thread.join();

  // Device stop acquisition
  emStatus = GXStreamOff(g_hDevice);
  if (emStatus != GX_STATUS_SUCCESS) {
    // Release the memory allocated
    UnPreForAcquisition();
    GX_VERIFY_EXIT(emStatus);
  }

  // Release the resources and stop acquisition thread
  UnPreForAcquisition();

  // Close device
  emStatus = GXCloseDevice(g_hDevice);
  if (emStatus != GX_STATUS_SUCCESS) {
    GetErrorString(emStatus);
    g_hDevice = NULL;
    GXCloseLib();
  }

  // Release libary
  emStatus = GXCloseLib();
  if (emStatus != GX_STATUS_SUCCESS) {
    GetErrorString(emStatus);
  }
}

void Camera::SetExposure(float exp) {
  GX_STATUS emStatus = GXSetFloatValue(g_hDevice, "ExposureTime", exp);
  GX_VERIFY_EXIT(emStatus);
}

void Camera::SetFrameRate(float fps) {
  GX_STATUS emStatus = GXSetFloatValue(g_hDevice, "AcquisitionFrameRate", fps);
  GX_VERIFY_EXIT(emStatus);
}

void Camera::SetGain(float gain) {
  GX_STATUS emStatus = GXSetFloatValue(g_hDevice, "Gain", gain);
  GX_VERIFY_EXIT(emStatus);
}

float Camera::GetExposure() {
  GX_FLOAT_VALUE emExposure;
  GX_STATUS emStatus = GXGetFloatValue(g_hDevice, "ExposureTime", &emExposure);
  GX_VERIFY_EXIT(emStatus);

  return emExposure.dCurValue;
}

std::pair<int32_t, int32_t> Camera::GetFrameDimensions() {
  GX_INT_VALUE emWidth, emHeight;

  GX_STATUS emStatus = GX_STATUS_SUCCESS;
  emStatus = GXGetIntValue(g_hDevice, "SensorWidth", &emWidth);
  GX_VERIFY_EXIT(emStatus);
  emStatus = GXGetIntValue(g_hDevice, "SensorHeight", &emHeight);
  GX_VERIFY_EXIT(emStatus);

  return std::make_pair(emWidth.nCurValue, emHeight.nCurValue);
}

void Camera::GetErrorString(GX_STATUS emErrorStatus) {
  char *error_info = nullptr;
  size_t size = 0;
  GX_STATUS emStatus = GX_STATUS_SUCCESS;

  // Get length of error description
  emStatus = GXGetLastError(&emErrorStatus, NULL, &size);
  if (emStatus != GX_STATUS_SUCCESS) {
    std::cout << "<Error when calling GXGetLastError>\n";
    return;
  }

  // Alloc error resources
  error_info = new char[size];
  if (error_info == nullptr) {
    std::cout << "<Failed to allocate memory>\n";
    return;
  }

  // Get error description
  emStatus = GXGetLastError(&emErrorStatus, error_info, &size);
  if (emStatus != GX_STATUS_SUCCESS) {
    std::cout << "<Error when calling GXGetLastError>\n";
  } else {
    std::cout << error_info << "\n";
  }

  // Realease error resources
  if (error_info != nullptr) {
    delete[] error_info;
    error_info = nullptr;
  }
}

void Camera::PreForAcquisition() {
  g_pRGBImageBuf = new unsigned char[g_nPayloadSize * 3];
  g_pRaw8Image = new unsigned char[g_nPayloadSize];
}

void Camera::UnPreForAcquisition() {
  // Release resources
  if (g_pRaw8Image != NULL) {
    delete[] g_pRaw8Image;
    g_pRaw8Image = NULL;
  }
  if (g_pRGBImageBuf != NULL) {
    delete[] g_pRGBImageBuf;
    g_pRGBImageBuf = NULL;
  }
}

void Camera::ProcGetImage() {
  GX_STATUS emStatus = GX_STATUS_SUCCESS;

  // Thread running flag setup
  g_bAcquisitionFlag = true;
  PGX_FRAME_BUFFER pFrameBuffer = NULL;

  while (g_bAcquisitionFlag) {
    // Get a frame from Queue
    emStatus = GXDQBuf(g_hDevice, &pFrameBuffer, 1000);
    if (emStatus != GX_STATUS_SUCCESS) {
      if (emStatus == GX_STATUS_TIMEOUT) {
        continue;
      } else {
        GetErrorString(emStatus);
        break;
      }
    }
    if (pFrameBuffer->nStatus != GX_FRAME_STATUS_SUCCESS) {
      std::cout << "<Abnormal Acquisition: Exception code: "
                << pFrameBuffer->nStatus << ">\n";
    } else {
      int nRet = PixelFormatConvert(pFrameBuffer);
      if (nRet != PIXFMT_CVT_SUCCESS) {
        std::cout << "PixelFormat Convert failed!\n";
      } else {
        std::unique_lock<std::mutex> lock(cb_mutex);
        callback(g_pRGBImageBuf);
      }
    }

    emStatus = GXQBuf(g_hDevice, pFrameBuffer);
    if (emStatus != GX_STATUS_SUCCESS) {
      GetErrorString(emStatus);
      break;
    }
  }

  std::cout << "<Acquisition thread Exit!>\n";
}

int Camera::PixelFormatConvert(PGX_FRAME_BUFFER pFrameBuffer) {
  VxInt32 emDXStatus = DX_OK;

  // Convert RAW8 or RAW16 image to RGB24 image
  switch (pFrameBuffer->nPixelFormat) {
  case GX_PIXEL_FORMAT_BAYER_GR8:
  case GX_PIXEL_FORMAT_BAYER_RG8:
  case GX_PIXEL_FORMAT_BAYER_GB8:
  case GX_PIXEL_FORMAT_BAYER_BG8: {
    // Convert to the RGB image
    emDXStatus = DxRaw8toRGB24((unsigned char *)pFrameBuffer->pImgBuf,
                               g_pRGBImageBuf, pFrameBuffer->nWidth,
                               pFrameBuffer->nHeight, RAW2RGB_NEIGHBOUR,
                               DX_PIXEL_COLOR_FILTER(g_i64ColorFilter), false);
    if (emDXStatus != DX_OK) {
      printf("DxRaw8toRGB24 Failed, Error Code: %d\n", emDXStatus);
      return PIXFMT_CVT_FAIL;
    }
    break;
  }
  case GX_PIXEL_FORMAT_BAYER_GR10:
  case GX_PIXEL_FORMAT_BAYER_RG10:
  case GX_PIXEL_FORMAT_BAYER_GB10:
  case GX_PIXEL_FORMAT_BAYER_BG10:
  case GX_PIXEL_FORMAT_BAYER_GR12:
  case GX_PIXEL_FORMAT_BAYER_RG12:
  case GX_PIXEL_FORMAT_BAYER_GB12:
  case GX_PIXEL_FORMAT_BAYER_BG12: {
    // Convert to the Raw8 image
    emDXStatus =
        DxRaw16toRaw8((unsigned char *)pFrameBuffer->pImgBuf, g_pRaw8Image,
                      pFrameBuffer->nWidth, pFrameBuffer->nHeight, DX_BIT_2_9);
    if (emDXStatus != DX_OK) {
      printf("DxRaw16toRaw8 Failed, Error Code: %d\n", emDXStatus);
      return PIXFMT_CVT_FAIL;
    }
    // Convert to the RGB24 image
    emDXStatus =
        DxRaw8toRGB24(g_pRaw8Image, g_pRGBImageBuf, pFrameBuffer->nWidth,
                      pFrameBuffer->nHeight, RAW2RGB_NEIGHBOUR,
                      DX_PIXEL_COLOR_FILTER(g_i64ColorFilter), false);
    if (emDXStatus != DX_OK) {
      printf("DxRaw8toRGB24 Failed, Error Code: %d\n", emDXStatus);
      return PIXFMT_CVT_FAIL;
    }
    break;
  }
  default: {
    printf("Error : PixelFormat of this camera is not supported\n");
    return PIXFMT_CVT_FAIL;
  }
  }
  return PIXFMT_CVT_SUCCESS;
}
} // namespace sdk_daheng