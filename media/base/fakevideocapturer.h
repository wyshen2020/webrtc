/*
 *  Copyright (c) 2004 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MEDIA_BASE_FAKEVIDEOCAPTURER_H_
#define MEDIA_BASE_FAKEVIDEOCAPTURER_H_

#include <string.h>

#include <memory>
#include <vector>

#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "media/base/videocapturer.h"
#include "media/base/videocommon.h"
#include "rtc_base/event.h"
#include "rtc_base/task_queue.h"
#include "rtc_base/timeutils.h"

namespace cricket {

// Fake video capturer that allows the test to manually pump in frames.
class FakeVideoCapturer : public cricket::VideoCapturer {
 public:
  explicit FakeVideoCapturer(bool is_screencast);
  FakeVideoCapturer();

  ~FakeVideoCapturer() override;

  void ResetSupportedFormats(const std::vector<cricket::VideoFormat>& formats);
  virtual bool CaptureFrame();
  virtual bool CaptureCustomFrame(int width, int height, uint32_t fourcc);
  virtual bool CaptureCustomFrame(int width,
                                  int height,
                                  int64_t timestamp_interval,
                                  uint32_t fourcc);

  sigslot::signal1<FakeVideoCapturer*> SignalDestroyed;

  cricket::CaptureState Start(const cricket::VideoFormat& format) override;
  void Stop() override;
  bool IsRunning() override;
  bool IsScreencast() const override;
  bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;

  void SetRotation(webrtc::VideoRotation rotation);

  webrtc::VideoRotation GetRotation();

 private:
  bool running_;
  int64_t initial_timestamp_;
  int64_t next_timestamp_;
  const bool is_screencast_;
  webrtc::VideoRotation rotation_;
};

// Inherits from FakeVideoCapturer but adds a TaskQueue so that frames can be
// delivered on a TaskQueue as expected by VideoSinkInterface implementations.
class FakeVideoCapturerWithTaskQueue : public FakeVideoCapturer {
 public:
  explicit FakeVideoCapturerWithTaskQueue(bool is_screencast);
  FakeVideoCapturerWithTaskQueue();

  bool CaptureFrame() override;
  bool CaptureCustomFrame(int width, int height, uint32_t fourcc) override;
  bool CaptureCustomFrame(int width,
                          int height,
                          int64_t timestamp_interval,
                          uint32_t fourcc) override;

 protected:
  template <class Closure>
  void RunSynchronouslyOnTaskQueue(Closure&& closure) {
    if (task_queue_.IsCurrent()) {
      closure();
      return;
    }
    rtc::Event event(false, false);
    task_queue_.PostTask([&closure, &event]() {
      closure();
      event.Set();
    });
    event.Wait(rtc::Event::kForever);
  }

  rtc::TaskQueue task_queue_{"FakeVideoCapturerWithTaskQueue"};
};

}  // namespace cricket

#endif  // MEDIA_BASE_FAKEVIDEOCAPTURER_H_
