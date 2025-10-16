#pragma once

// Daheng camera
#include "lib/DxImageProc.h"
#include "lib/GxIAPI.h"

// Common
#include <functional>
#include <mutex>
#include <thread>

#define ACQ_BUFFER_NUM 5              ///< Acquisition Buffer Qty.
#define ACQ_TRANSFER_SIZE (64 * 1024) ///< Size of data transfer block
#define ACQ_TRANSFER_NUMBER_URB 64    ///< Qty. of data transfer block
#define FILE_NAME_LEN 50              ///< Save image file name length

#define PIXFMT_CVT_FAIL -1            ///< PixelFormatConvert fail
#define PIXFMT_CVT_SUCCESS 0          ///< PixelFormatConvert success

namespace sdk_daheng {

class Camera {
public:
  Camera();
  ~Camera();

  void Acquire();
  void SetCallback(const std::function<void(unsigned char *)> &cb);
  void Release();

  void SetExposure(float exp);
  void SetFrameRate(float fps);
  void SetGain(float gain);

  float GetExposure();
  std::pair<int32_t, int32_t> GetFrameDimensions();

private:
  inline void GX_VERIFY_EXIT(int emStatus) {
    if (emStatus != GX_STATUS_SUCCESS) {
      GetErrorString(emStatus);
      GXCloseDevice(g_hDevice);
      g_hDevice = nullptr;
      GXCloseLib();
    }
  }

  void GetErrorString(GX_STATUS emErrorStatus);
  void PreForAcquisition();
  void UnPreForAcquisition();
  void ProcGetImage();
  int PixelFormatConvert(PGX_FRAME_BUFFER pFrameBuffer);

private:
  GX_DEV_HANDLE g_hDevice = nullptr;
  bool g_bColorFilter = false;                     ///< Color filter support flag
  int64_t g_i64ColorFilter = GX_COLOR_FILTER_NONE; ///< Color filter of device
  bool g_bAcquisitionFlag = false;                 ///< Thread running flag
  unsigned char *g_pRGBImageBuf = nullptr;         ///< Memory for RAW8toRGB24
  unsigned char *g_pRaw8Image = nullptr;           ///< Memory for RAW16toRAW8
  uint32_t g_nPayloadSize = 0;                     ///< Payload size
  std::pair<int32_t, int32_t> size_;

  std::thread aq_thread;
  std::mutex cb_mutex; // guarding callback changes;
  std::function<void(unsigned char *)> callback = [](unsigned char *) {};
};

} // namespace sdk_daheng