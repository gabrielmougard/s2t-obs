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

#include <chrono>
#include <mutex>
#include <utility>

#include "spdlog/spdlog.h"

#include "overlapping_caption.h"

namespace backend {

OverlappingCaption::OverlappingCaption(OverlappingCaptionStreamSettings settings) : current_stream(nullptr), prepared_stream(nullptr), settings(settings) {}

bool OverlappingCaption::queue_audio_data(const char *data, const uint data_size) {
    if (!data_size)
        return false;

    if (!current_stream) {
        spdlog::debug("first time, no current stream cycling");
        cycle_streams();
    }

    if (!current_stream) {
        spdlog::error("Error: no upstream !");
        return false;
    }

    double secs_since_start = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::steady_clock::now() - current_started_at
    ).count();

    if (current_stream->is_stopped()) {
        if (settings.minimum_reconnect_interval_secs_ && secs_since_start < settings.minimum_reconnect_interval_secs_) {
            // current stream dead, not reconnecting yet, too soon
            return false;
        }
        spdlog::debug("current stream dead, cycling, %f", secs_since_start);
        cycle_streams();
    }

    if (settings.switchover_second_after_secs_ && secs_since_start < settings.minimum_reconnect_interval_secs_) {
        if (prepared_stream) {
            if (prepared_stream->is_stopped()) {
                spdlog::debug("trying to swittch to prepared stream but dead, recreating");
                clear_prepared();
                start_prepared();
            } else {
                spdlog::debug("switching over to prepared stream, %f > %u", secs_since_start, settings.switchover_second_after_secs_);
                cycle_streams();
            }
        }
    } else if (settings.connect_second_after_secs_ && secs_since_start > settings.connect_second_after_secs_) {
        if (!prepared_stream || prepared_stream->is_stopped()) {
            spdlog::debug("starting second stream %f", secs_since_start);
            start_prepared();
        } else {
            prepared_stream->queue_audio_data(data, data_size);
        }
    }

    return current_stream->queue_audio_data(data, data_size);
}


void OverlappingCaption::start_prepared() {
    spdlog::debug("starting second prepared connection");
    clear_prepared();
    prepared_stream =  std::make_shared<InferenceStream>(settings.stream_settings_);
    if (!prepared_stream->start(prepared_stream)) {
        spdlog::error("FAILED starting prepared connection");
    }

    prepared_started_at = std::chrono::steady_clock::now();
}

void OverlappingCaption::clear_prepared() {
    if (!prepared_stream)
        return;

    spdlog::debug("clearing prepared connection");
    prepared_stream->stop();
    prepared_stream = nullptr;
}

void OverlappingCaption::cycle_streams() {
    spdlog::debug("cycling streams");

    if (current_stream) {
        current_stream->stop();
        current_stream = nullptr;
    }

    if (prepared_stream && prepared_stream->is_stopped()) {
        prepared_stream->stop();
        prepared_stream = nullptr;
    }

    if (last_caption_result && !last_caption_result->final) {
        spdlog::debug("stream interrupted, last result was not final, sending copy of last with fixed final=true");
        last_caption_result->final = true;
        {
            std::lock_guard<std::recursive_mutex> lock(on_caption_cb_handle.mutex);
            if (on_caption_cb_handle.callback_fn) {
                on_caption_cb_handle.callback_fn(*last_caption_result, true);
            }
        }
    }
    last_caption_result = nullptr;

    caption_text_callback cb = std::bind(&OverlappingCaption::on_caption_text_cb, this, std::placeholders::_1);

    if (prepared_stream) {
        spdlog::debug("cycling streams, using prepared connection");
        current_stream = prepared_stream;
        current_started_at = prepared_started_at;
        current_stream->on_caption_cb_handle.set(cb);
    } else {
        spdlog::debug("cycling streams, creating new connection");
        current_stream = std::make_shared<InferenceStream>(settings.stream_settings_);
        current_stream->on_caption_cb_handle.set(cb);
        if (!current_stream->start(current_stream))
            spdlog::error("FAILED starting new connection");
        current_started_at = std::chrono::steady_clock::now();
    }
    prepared_stream = nullptr;
}

void OverlappingCaption::on_caption_text_cb(const RawResult &caption_result) {
    // got caption data
    {
        std::lock_guard<std::recursive_mutex> lock(on_caption_cb_handle.mutex);
        
        last_caption_result = std::make_unique<RawResult>();
        *last_caption_result = caption_result;

        if (on_caption_cb_handle.callback_fn) {
            on_caption_cb_handle.callback_fn(caption_result, false);
        }
    }
}

OverlappingCaption::~OverlappingCaption() {
    spdlog::debug("~OverlappingCaption");
    on_caption_cb_handle.clear();

    if (current_stream) {
        current_stream->stop();
    }

    if (prepared_stream) {
        prepared_stream->sstop();
    }
}

}