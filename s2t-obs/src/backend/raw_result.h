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

#ifndef OBS_SPEECH2TEXT_PLUGIN_RAW_RESULT_H
#define OBS_SPEECH2TEXT_PLUGIN_RAW_RESULT_H

#include <string>
#include <chrono>

namespace backend {

struct RawResult {
    int index = 0;
    bool final = false;
    double stability = 0.0;
    std::string caption_text;
    std::string raw_message;

    std::chrono::steady_clock::time_point first_received_at;
    std::chrono::steady_clock::time_point received_at;

    RawResult() {};
    RawResult(
        int index,
        bool final,
        double stability,
        std::string caption_text,
        std::string raw_message,
        std::chrono::steady_clock::time_point first_received_at,
        std::chrono::steady_clock::time_point received_at
    ) :
        index(index),
        final(final),
        stability(stability),
        caption_text(caption_text),
        raw_message(raw_message),
        first_received_at(first_received_at),
        received_at(received_at) {
    }
};
}

#endif // OBS_SPEECH2TEXT_PLUGIN_RAW_RESULT_H