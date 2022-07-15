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

#ifndef OBS_SPEECH2TEXT_PLUGIN_STORAGE_H
#define OBS_SPEECH2TEXT_PLUGIN_STORAGE_H

#include "backend/caption.h"

namespace utils {

static void string_to_words(const string &input, vector<string> &words) {
    string clean(input);
    std::replace(clean.begin(), clean.end(), '\n', ' ');
    std::replace(clean.begin(), clean.end(), '\t', ' ');
    istringstream stream(clean);

    string word;
    while (getline(stream, word, ' ')) {
        if (word.empty())
            continue;

        words.push_back(word);
    }
}

static void words_to_string(const vector<string> &words, string &output) {
    for (auto &word: words) {
        if (!output.empty())
            output.push_back(' ');
        output.append(word);
    }
}

static vector<string>  string_to_banned_words(const string &input_line) {
    string copy(input_line);
    std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);

    std::vector<string> banned_words;
    string_to_words(copy, banned_words);
    return banned_words;
}

static CaptionSourceMuteType string_to_mute_setting(const string &setting_string, CaptionSourceMuteType default_val) {
    CaptionSourceMuteType mute_setting = default_val;
    if (setting_string == "own_source")
        mute_setting = CAPTION_SOURCE_MUTE_TYPE_FROM_OWN_SOURCE;
    else if (setting_string == "other_mute_source")
        mute_setting = CAPTION_SOURCE_MUTE_TYPE_USE_OTHER_MUTE_SOURCE;
    else
        info_log("unknown mute when setting: '%s'", setting_string.c_str());

    return mute_setting;
}

static string mute_setting_to_string(const CaptionSourceMuteType mute_setting, const string &default_val) {
    string setting_str(default_val);
    if (mute_setting == CAPTION_SOURCE_MUTE_TYPE_FROM_OWN_SOURCE)
        setting_str = "own_source";
    else if (mute_setting == CAPTION_SOURCE_MUTE_TYPE_USE_OTHER_MUTE_SOURCE)
        setting_str = "other_mute_source";

    return setting_str;
}

}

#endif //OBS_SPEECH2TEXT_PLUGIN_STORAGE_H