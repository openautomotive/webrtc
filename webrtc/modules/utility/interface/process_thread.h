/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_UTILITY_INCLUDE_PROCESS_THREAD_H_
#define WEBRTC_MODULES_UTILITY_INCLUDE_PROCESS_THREAD_H_

#pragma message("WARNING: utility/interface is DEPRECATED; use utility/include")

#include "webrtc/typedefs.h"
#include "webrtc/base/scoped_ptr.h"

namespace webrtc {
class Module;

class ProcessTask {
 public:
  ProcessTask() {}
  virtual ~ProcessTask() {}

  virtual void Run() = 0;
};

class ProcessThread {
 public:
  virtual ~ProcessThread();

  static rtc::scoped_ptr<ProcessThread> Create(const char* thread_name);

  // Starts the worker thread.  Must be called from the construction thread.
  virtual void Start() = 0;

  // Stops the worker thread.  Must be called from the construction thread.
  virtual void Stop() = 0;

  // Wakes the thread up to give a module a chance to do processing right
  // away.  This causes the worker thread to wake up and requery the specified
  // module for when it should be called back. (Typically the module should
  // return 0 from TimeUntilNextProcess on the worker thread at that point).
  // Can be called on any thread.
  virtual void WakeUp(Module* module) = 0;

  // Queues a task object to run on the worker thread.  Ownership of the
  // task object is transferred to the ProcessThread and the object will
  // either be deleted after running on the worker thread, or on the
  // construction thread of the ProcessThread instance, if the task did not
  // get a chance to run (e.g. posting the task while shutting down or when
  // the thread never runs).
  virtual void PostTask(rtc::scoped_ptr<ProcessTask> task) = 0;

  // Adds a module that will start to receive callbacks on the worker thread.
  // Can be called from any thread.
  virtual void RegisterModule(Module* module) = 0;

  // Removes a previously registered module.
  // Can be called from any thread.
  virtual void DeRegisterModule(Module* module) = 0;
};

}  // namespace webrtc

#endif // WEBRTC_MODULES_UTILITY_INCLUDE_PROCESS_THREAD_H_
