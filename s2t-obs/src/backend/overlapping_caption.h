// Copyright 2022 gab
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OBS_SPEECH2TEXT_OVERLAPPING_CAPTION_H
#define OBS_SPEECH2TEXT_OVERLAPPING_CAPTION_H

#include <functional>

#include "inference_stream.h"
#include "threadsafe_cb.h"

namespace backend {

struct OverlappingCaptionStreamSettings {
    uint connect_second_after_secs_;
    uint switchover_second_after_secs_;
    uint minimum_reconnect_interval_secs_;
    InferenceStreamSettings stream_settings_;

    OverlappingCaptionStreamSettings(
        uint connect_second_after_secs,
        uint switchover_second_after_secs,
        uint minimum_reconnect_interval_secs,
        InferenceStreamSettings stream_settings
    ) : 
        connect_second_after_secs_(connect_second_after_secs),
        switchover_second_after_secs_(switchover_second_after_secs),
        minimum_reconnect_interval_secs_(minimum_reconnect_interval_secs),
        stream_settings_(stream_settings) {}
    
    bool operator==(const OverlappingCaptionStreamSettings &rhs) const {
        return connect_second_after_secs_ == rhs.connect_second_after_secs_ &&
            switchover_second_after_secs_ == rhs.switchover_second_after_secs_ &&
            minimum_reconnect_interval_secs_ == rhs.minimum_reconnect_interval_secs_ &&
            stream_settings_ == rhs.stream_settings_;
    }

    bool operator!=(const OverlappingCaptionStreamSettings &rhs) const {
        return !(*this == rhs);
    }

    void print(const char *line_prefix = "") {
        printf("%sOverlappingCaptionStreamSettings\n", line_prefix);
        printf("%s  connect_second_after_secs: %d\n", line_prefix, connect_second_after_secs_);
        printf("%s  switchover_second_after_secs: %d\n", line_prefix, switchover_second_after_secs_);
        printf("%s  minimum_reconnect_interval_secs: %d\n", line_prefix, minimum_reconnect_interval_secs_);

        stream_settings_.print((std::string(line_prefix) + "  ").c_str());
    }
};

typedef std::function<void(const RawResult &raw_caption_result, bool interrupted)> overlapping_caption_text_callback;

/*
 Provides a continuous stream of caption messages for audio data, abstracting issues like reconnects after
 network errors and API limitation workarounds.

 Minimizes impact of these regular disconnects by starting a second connection shortly before the first once
 is about to hit the limit and feeds both with the same audio for a bit before switching to the new one to avoid captioning gap.
*/
class OverlappingCaption {
public:
    ThreadsafeCb<overlapping_caption_text_callback> on_caption_cb_handle;
    
    OverlappingCaption(OverlappingCaptionStreamSettings settings);
    bool queue_audio_data(const char *data, const uint data_size);
    ~OverlappingCaption();

private:
    std::shared_ptr<InferenceStream> current_stream;
    std::shared_ptr<InferenceStream> prepared_stream;

    std::chrono::steady_clock::time_point current_started_at;
    std::chrono::steady_clock::time_point prepared_started_at;

    OverlappingCaptionStreamSettings settings;
    std::unique_ptr<RawResult> last_caption_result;

    void on_caption_text_cb(const RawResult &caption_result);
    void start_prepared();
    void clear_prepared();
    void cycle_streams();
};

}

#endif //OBS_SPEECH2TEXT_OVERLAPPING_CAPTION_H