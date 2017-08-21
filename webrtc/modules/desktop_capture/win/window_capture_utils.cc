/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/desktop_capture/win/window_capture_utils.h"

#include "webrtc/rtc_base/win32.h"

namespace webrtc {

bool GetWindowRect(HWND window, DesktopRect* result) {
  RECT rect;
  if (!::GetWindowRect(window, &rect)) {
    return false;
  }
  *result = DesktopRect::MakeLTRB(
      rect.left, rect.top, rect.right, rect.bottom);
  return true;
}

bool GetCroppedWindowRect(HWND window,
                          DesktopRect* cropped_rect,
                          DesktopRect* original_rect) {
  DesktopRect window_rect;
  if (!GetWindowRect(window, &window_rect)) {
    return false;
  }

  if (original_rect) {
    *original_rect = window_rect;
  }
  *cropped_rect = window_rect;

  WINDOWPLACEMENT window_placement;
  window_placement.length = sizeof(window_placement);
  if (!::GetWindowPlacement(window, &window_placement)) {
    return false;
  }

  // After Windows8, transparent borders will be added by OS at
  // left/bottom/right sides of a window. If the cropped window
  // doesn't remove these borders, the background will be exposed a bit.
  //
  // On Windows 8.1. or upper, rtc::IsWindows8OrLater(), which uses
  // GetVersionEx() may not correctly return the windows version. See
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724451(v=vs.85).aspx
  // So we always prefer to check |window_placement|.showCmd.
  if (rtc::IsWindows8OrLater() ||
      window_placement.showCmd == SW_SHOWMAXIMIZED) {
    const int width = GetSystemMetrics(SM_CXSIZEFRAME);
    const int height = GetSystemMetrics(SM_CYSIZEFRAME);
    cropped_rect->Extend(-width, 0, -width, -height);
  }

  return true;
}

bool GetWindowContentRect(HWND window, DesktopRect* result) {
  if (!GetWindowRect(window, result)) {
    return false;
  }

  RECT rect;
  // If GetClientRect() failed, do nothing and return window area instead.
  if (::GetClientRect(window, &rect)) {
    const int width = rect.right - rect.left;
    // The GetClientRect() is not expected to return a larger area than
    // GetWindowRect().
    if (width > 0 && width < result->width()) {
      // GetClientRect() always set the left / top of RECT to 0. So we shrink
      // half of the size from all four sides.
      // Note: GetClientRect() excludes the title bar, so we always estimate the
      // border width according to the window width. This value varies according
      // to the window type.
      const int shrink = ((width - result->width()) >> 1);
      result->Extend(shrink, shrink, shrink, shrink);
    }
  }

  return true;
}

AeroChecker::AeroChecker() : dwmapi_library_(nullptr), func_(nullptr) {
  // Try to load dwmapi.dll dynamically since it is not available on XP.
  dwmapi_library_ = LoadLibrary(L"dwmapi.dll");
  if (dwmapi_library_) {
    func_ = reinterpret_cast<DwmIsCompositionEnabledFunc>(
        GetProcAddress(dwmapi_library_, "DwmIsCompositionEnabled"));
  }
}

AeroChecker::~AeroChecker() {
  if (dwmapi_library_) {
    FreeLibrary(dwmapi_library_);
  }
}

bool AeroChecker::IsAeroEnabled() {
  BOOL result = FALSE;
  if (func_) {
    func_(&result);
  }
  return result != FALSE;
}

}  // namespace webrtc
