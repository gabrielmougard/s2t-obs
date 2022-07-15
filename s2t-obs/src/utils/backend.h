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

#ifndef OBS_SPEECH2TEXT_PLUGIN_UTILS_BACKEND_H
#define OBS_SPEECH2TEXT_PLUGIN_UTILS_BACKEND_H

#define AUDIO_OUTPUT_CAPTURE_ALL_AUDIO_SOURCE_NAME "______________________________________audio_output_capture___all_audio_source_lets_hope_no_one_ever_names_a_obs_source_list_this_"

#include <string.h>

#include <string>
#include <functional>

namespace utils {

static bool is_all_audio_output_capture_source_data(const std::string &data) {
    return data.find(AUDIO_OUTPUT_CAPTURE_ALL_AUDIO_SOURCE_NAME) == 0;
}

static int all_audio_output_capture_source_track_index(const std::string &data) {
    if (!is_all_audio_output_capture_source_data(data))
        return -1;

    const std::string rest = data.substr(strlen(AUDIO_OUTPUT_CAPTURE_ALL_AUDIO_SOURCE_NAME));
    try {
        return std::stoi(rest, nullptr);
    }
    catch (...) {
        return -1;
    }
}

static const std::string all_audio_output_capture_source_name(const int track_index) {
    return "Stream Audio (Track " + std::to_string(track_index + 1) + ")";
}

static const std::string all_audio_output_capture_source_data(const int track_index) {
    return AUDIO_OUTPUT_CAPTURE_ALL_AUDIO_SOURCE_NAME + std::to_string(track_index);
}

static const std::string corrected_streaming_audio_output_capture_source_name(const std::string &data) {
    const int track_index = all_audio_output_capture_source_track_index(data);
    if (track_index >= 0) {
        return all_audio_output_capture_source_name(track_index);
    }

    return data;
}

enum audio_source_capture_status {
    AUDIO_SOURCE_CAPTURING = 1,
    AUDIO_SOURCE_MUTED = 2,
    AUDIO_SOURCE_NOT_STREAMED = 3,
};

enum source_capture_config {
    MUTED_SOURCE_DISCARD_WHEN_MUTED,
    MUTED_SOURCE_REPLACE_WITH_ZERO,
    MUTED_SOURCE_STILL_CAPTURE,
};

typedef std::function<void(const int id, const uint8_t *, const size_t)> audio_chunk_data_cb;
typedef std::function<void(const int id, const audio_source_capture_status status)> audio_capture_status_change_cb;

}

#endif //OBS_SPEECH2TEXT_PLUGIN_UTILS_BACKEND_H