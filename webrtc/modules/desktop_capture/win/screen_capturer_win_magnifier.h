/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_DESKTOP_CAPTURE_WIN_SCREEN_CAPTURER_WIN_MAGNIFIER_H_
#define WEBRTC_MODULES_DESKTOP_CAPTURE_WIN_SCREEN_CAPTURER_WIN_MAGNIFIER_H_

#include <memory>

#include <windows.h>
#include <magnification.h>
#include <wincodec.h>

#include "webrtc/base/constructormagic.h"
#include "webrtc/modules/desktop_capture/screen_capture_frame_queue.h"
#include "webrtc/modules/desktop_capture/screen_capturer.h"
#include "webrtc/modules/desktop_capture/screen_capturer_helper.h"
#include "webrtc/modules/desktop_capture/shared_desktop_frame.h"
#include "webrtc/modules/desktop_capture/win/scoped_thread_desktop.h"
#include "webrtc/system_wrappers/include/atomic32.h"

namespace webrtc {

class DesktopFrame;
class DesktopRect;

// Captures the screen using the Magnification API to support window exclusion.
// Each capturer must run on a dedicated thread because it uses thread local
// storage for redirecting the library callback. Also the thread must have a UI
// message loop to handle the window messages for the magnifier window.
//
// This class does not detect DesktopFrame::updated_region(), the field is
// always set to the entire frame rectangle. ScreenCapturerDifferWrapper should
// be used if that functionality is necessary.
class ScreenCapturerWinMagnifier : public ScreenCapturer {
 public:
  // |fallback_capturer| will be used to capture the screen if a non-primary
  // screen is being captured, or the OS does not support Magnification API, or
  // the magnifier capturer fails (e.g. in Windows8 Metro mode).
  explicit ScreenCapturerWinMagnifier(
      std::unique_ptr<ScreenCapturer> fallback_capturer);
  virtual ~ScreenCapturerWinMagnifier();

  // Overridden from ScreenCapturer:
  void Start(Callback* callback) override;
  void SetSharedMemoryFactory(
      std::unique_ptr<SharedMemoryFactory> shared_memory_factory) override;
  void Capture(const DesktopRegion& region) override;
  bool GetScreenList(ScreenList* screens) override;
  bool SelectScreen(ScreenId id) override;
  void SetExcludedWindow(WindowId window) override;

 private:
  typedef BOOL(WINAPI* MagImageScalingCallback)(HWND hwnd,
                                                void* srcdata,
                                                MAGIMAGEHEADER srcheader,
                                                void* destdata,
                                                MAGIMAGEHEADER destheader,
                                                RECT unclipped,
                                                RECT clipped,
                                                HRGN dirty);
  typedef BOOL(WINAPI* MagInitializeFunc)(void);
  typedef BOOL(WINAPI* MagUninitializeFunc)(void);
  typedef BOOL(WINAPI* MagSetWindowSourceFunc)(HWND hwnd, RECT rect);
  typedef BOOL(WINAPI* MagSetWindowFilterListFunc)(HWND hwnd,
                                                   DWORD dwFilterMode,
                                                   int count,
                                                   HWND* pHWND);
  typedef BOOL(WINAPI* MagSetImageScalingCallbackFunc)(
      HWND hwnd,
      MagImageScalingCallback callback);

  static BOOL WINAPI OnMagImageScalingCallback(HWND hwnd,
                                               void* srcdata,
                                               MAGIMAGEHEADER srcheader,
                                               void* destdata,
                                               MAGIMAGEHEADER destheader,
                                               RECT unclipped,
                                               RECT clipped,
                                               HRGN dirty);

  // Captures the screen within |rect| in the desktop coordinates. Returns true
  // if succeeded.
  // It can only capture the primary screen for now. The magnification library
  // crashes under some screen configurations (e.g. secondary screen on top of
  // primary screen) if it tries to capture a non-primary screen. The caller
  // must make sure not calling it on non-primary screens.
  bool CaptureImage(const DesktopRect& rect);

  // Helper method for setting up the magnifier control. Returns true if
  // succeeded.
  bool InitializeMagnifier();

  // Called by OnMagImageScalingCallback to output captured data.
  void OnCaptured(void* data, const MAGIMAGEHEADER& header);

  // Makes sure the current frame exists and matches |size|.
  void CreateCurrentFrameIfNecessary(const DesktopSize& size);

  // Start the fallback capturer and select the screen.
  void StartFallbackCapturer();

  static Atomic32 tls_index_;

  std::unique_ptr<ScreenCapturer> fallback_capturer_;
  bool fallback_capturer_started_ = false;
  Callback* callback_ = nullptr;
  std::unique_ptr<SharedMemoryFactory> shared_memory_factory_;
  ScreenId current_screen_id_ = kFullDesktopScreenId;
  std::wstring current_device_key_;
  HWND excluded_window_ = NULL;

  // Queue of the frames buffers.
  ScreenCaptureFrameQueue<SharedDesktopFrame> queue_;

  ScopedThreadDesktop desktop_;

  // Used for getting the screen dpi.
  HDC desktop_dc_ = NULL;

  HMODULE mag_lib_handle_ = NULL;
  MagInitializeFunc mag_initialize_func_ = nullptr;
  MagUninitializeFunc mag_uninitialize_func_ = nullptr;
  MagSetWindowSourceFunc set_window_source_func_ = nullptr;
  MagSetWindowFilterListFunc set_window_filter_list_func_ = nullptr;
  MagSetImageScalingCallbackFunc set_image_scaling_callback_func_ = nullptr;

  // The hidden window hosting the magnifier control.
  HWND host_window_ = NULL;
  // The magnifier control that captures the screen.
  HWND magnifier_window_ = NULL;

  // True if the magnifier control has been successfully initialized.
  bool magnifier_initialized_ = false;

  // True if the last OnMagImageScalingCallback was called and handled
  // successfully. Reset at the beginning of each CaptureImage call.
  bool magnifier_capture_succeeded_ = true;

  RTC_DISALLOW_COPY_AND_ASSIGN(ScreenCapturerWinMagnifier);
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_DESKTOP_CAPTURE_WIN_SCREEN_CAPTURER_WIN_MAGNIFIER_H_
