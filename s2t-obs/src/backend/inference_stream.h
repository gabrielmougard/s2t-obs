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

#ifndef OBS_SPEECH2TEXT_PLUGIN_INFERENCE_STREAM_H
#define OBS_SPEECH2TEXT_PLUGIN_INFERENCE_STREAM_H

#include <cstdint>
#include <functional>

#include <moodycamel/blockingconcurrentqueue.h>

#include "raw_result.h"
#include "threadsafe_cb.h"

namespace backend {

typedef std::function<void(const RawResult &raw_result)> caption_text_callback;
typedef unsigned int uint;

struct InferenceStreamSettings {
    uint connect_timeout_ms;
    uint send_timeout_ms;
    uint recv_timeout_ms;

    uint max_queue_depth;

    std::string language;

    InferenceStreamSettings(
        uint connect_timeout_ms,
        uint send_timeout_ms,
        uint recv_timeout_ms,
        uint max_queue_depth,
        std::string language
    ) :
        connect_timeout_ms(connect_timeout_ms),
        send_timeout_ms(send_timeout_ms),
        recv_timeout_ms(recv_timeout_ms),
        max_queue_depth(max_queue_depth),
        language(language) {}
    
    bool operator==(const InferenceStreamSettings &rhs) const {
        return connect_timeout_ms == rhs.connect_timeout_ms &&
            send_timeout_ms == rhs.send_timeout_ms &&
            recv_timeout_ms == rhs.recv_timeout_ms &&
            max_queue_depth == rhs.max_queue_depth &&
            language == rhs.language;
    }

    bool operator!=(const InferenceStreamSettings &rhs) const {
        return !(rhs == *this);
    }

    void print(const char *line_prefix = "") {
        printf("%sInferenceStreamSettings\n", line_prefix);
        printf("%s connect_timeout_ms: %d\n", line_prefix, connect_timeout_ms);
        printf("%s send_timeout_ms: %d\n", line_prefix, send_timeout_ms);
        printf("%s recv_timeout_ms: %d\n", line_prefix, recv_timeout_ms);
        printf("%s max_queue_depth: %d\n", line_prefix, max_queue_depth);
    }
};

class InferenceStream {
    std::string session_pair;
    moodycamel::BlockingConcurrentQueue<std::string *> audio_queue;

    bool started = false;
    bool stopped = false;

public:
    const InferenceStreamSettings settings;
    ThreadsafeCb<caption_text_callback> on_caption_cb_handle;
    InferenceStream(const InferenceStreamSettings settings);
    bool start(std::shared_ptr<InferenceStream> self);
    void stop();
    bool is_stopped();
    bool queue_audio_data(const char *data, const uint data_size);
    std::string *dequeue_audio_data(const std::int64_t timeout_us);
    ~InferenceStream();
};
}

#endif // OBS_SPEECH2TEXT_PLUGIN_INFERENCE_STREAM_H