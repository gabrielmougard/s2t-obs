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

// TODO

namespace utils {

bool is_streaming_live() {

}

bool is_recording_live() {

}

bool is_virtualcam_on() {

}

typedef std::tuple<std::string, std::string, uint32_t> ObsSourceTup;

static std::vector<std::string> get_audio_sources() {
    auto sources = get_obs_sources();
    std::vector<std::string> audio_sources;
    for (auto &source: sources) {
        if (std::get<2>(source) & OBS_SOURCE_AUDIO) {
            audio_sources.push_back(std::get<0>(source));
        }
    }
    return audio_sources;
}

}