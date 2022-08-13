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

#include "audio_converter_pipeline.h"

#include <utility>

#include "util/platform.h" // OBS platform utils
#include "spdlog/spdlog.h"
namespace backend {

static void audio_captured(void *param, size_t mix_idx, struct audio_data *audio) {
    auto session = reinterpret_cast<AudioConverterPipeline *>(param);
    if (session)
        session->audio_capture_cb(mix_idx, audio);
}

AudioConverterPipeline::AudioConverterPipeline(
        int track_index,
        audio_chunk_data_cb audio_data_cb,
        audio_capture_status_change_cb status_change_cb,
        resample_info resample_to,
        int id
) :
        on_caption_cb_handle(audio_data_cb),
        on_status_cb_handle(status_change_cb),
        bytes_per_channel(get_audio_bytes_per_channel(resample_to.format)),
        track_index(track_index),
        id(id) {

    spdlog::debug("AudioConverterPipeline");

    obs_audio_info backend_audio_settings;
    if (!obs_get_audio_info(&backend_audio_settings))
        throw std::string("Failed to get OBS audio info");

    spdlog::debug("output audio_info track %d: %d, %d", track_index, backend_audio_settings.samples_per_sec, backend_audio_settings.speakers);

    if (!bytes_per_channel)
        throw std::string("Failed to get frame bytes size per channel");

    converter.speakers = resample_to.speakers;
    converter.format = resample_to.format;
    converter.samples_per_sec = resample_to.samples_per_sec;

    audio_output = obs_get_audio();
    if (!audio_output) {
        throw std::string("couldn't get output audio");
    }

    audio_output_connect(audio_output, track_index, &converter, audio_captured, this);
}

void AudioConverterPipeline::audio_capture_cb(size_t mix_idx, const struct audio_data *audio) {
    if (!on_caption_cb_handle.callback_fn)
        return;

    if (!audio || !audio->frames)
        return;

    unsigned int size = audio->frames * bytes_per_channel;
    {
        std::lock_guard<std::recursive_mutex> lock(on_caption_cb_handle.mutex);
        if (on_caption_cb_handle.callback_fn)
            on_caption_cb_handle.callback_fn(id, audio->data[0], size);
    }
}

AudioConverterPipeline::~OutputAudioCaptureSession() {
    on_caption_cb_handle.clear();
    on_status_cb_handle.clear();

    audio_output_disconnect(audio_output, track_index, audio_captured, this);
    debug_log("AudioConverterPipeline() deaded");
}

}