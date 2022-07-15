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

#pragma once

#include <obs.h>
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <media-io/audio-resampler.h>
#include <obs.hpp>
#include <functional>
#include <mutex>

#include "threadsafe_cb.h"
#include "util/utils_backend.h"

class AudioCapturePipeline {
    OBSSource audio_source;
    OBSSource muting_source;
    source_capture_config muted_handling;

    audio_resampler_t *resampler = nullptr;
    audio_source_capture_status capture_status;
    bool use_muting_cb_signal = true;
    const int id;
    const int bytes_per_channel;
public:
    ThreadsafeCb<audio_chunk_data_cb> on_caption_cb_handle;
    ThreadsafeCb<audio_capture_status_change_cb> on_status_cb_handle;

    AudioCapturePipeline(
        obs_source_t *audio_source,
        obs_source_t *muting_source,
        audio_chunk_data_cb audio_data_cb,
        audio_capture_status_change_cb status_change_cb,
        resample_info resample_to,
        source_capture_config muted_handling,
        bool send_startup_change_signal,
        int id
    );

    void audio_capture_cb(obs_source_t *source, const struct audio_data *audio, bool muted);

    ~AudioCapturePipeline();

    void state_changed_check(bool always_signal = false);

    audio_source_capture_status get_current_capture_status();

    audio_source_capture_status check_source_status();
};