/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <windows.h>

#include "webrtc/modules/desktop_capture/desktop_geometry.h"
#include "webrtc/rtc_base/constructormagic.h"

namespace webrtc {

// Output the window rect, with the left/right/bottom frame border cropped if
// the window is maximized. |cropped_rect| is the cropped rect relative to the
// desktop. |original_rect| is the original rect returned from GetWindowRect.
// Returns true if all API calls succeeded. The returned DesktopRect is in
// system coordinates, i.e. the primary monitor on the system always starts from
// (0, 0).
bool GetCroppedWindowRect(HWND window,
                          DesktopRect* cropped_rect,
                          DesktopRect* original_rect);

// Retrieves the rectangle of the content area of |window|. The content area
// does not include borders or shadow. The returned DesktopRect is in system
// coordinates, i.e. the primary monitor on the system always starts from
// (0, 0). This function returns false if native APIs fail.
bool GetWindowContentRect(HWND window, DesktopRect* result);

typedef HRESULT (WINAPI *DwmIsCompositionEnabledFunc)(BOOL* enabled);
class AeroChecker {
 public:
  AeroChecker();
  ~AeroChecker();

  bool IsAeroEnabled();

 private:
  HMODULE dwmapi_library_;
  DwmIsCompositionEnabledFunc func_;

  RTC_DISALLOW_COPY_AND_ASSIGN(AeroChecker);
};

}  // namespace webrtc
