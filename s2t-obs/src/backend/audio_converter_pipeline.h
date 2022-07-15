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

class AudioConverterPipeline {
    const int id;
    struct audio_convert_info converter;
    audio_t *audio_output = nullptr;
    const int bytes_per_channel;
    const int track_index;
public:
    ThreadsafeCb<audio_chunk_data_cb> on_caption_cb_handle;
    ThreadsafeCb<audio_capture_status_change_cb> on_status_cb_handle;

    AudioConverterPipeline(
        int track_index,
        audio_chunk_data_cb audio_data_cb,
        audio_capture_status_change_cb status_change_cb,
        resample_info resample_to,
        int id
    );

    void audio_capture_cb(size_t mix_idx, const struct audio_data *audio);

    ~AudioConverterPipeline();
};