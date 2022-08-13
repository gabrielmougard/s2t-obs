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

#ifndef OBS_SPEECH2TEXT_SETTINGS_H
#define OBS_SPEECH2TEXT_SETTINGS_H


#include "caption.h"

struct PluginSettings {
    bool enabled;
    SourceCaptionerSettings source_cap_settings;

    PluginSettings(bool enabled, const SourceCaptionerSettings &source_cap_settings) :
            enabled(enabled),
            source_cap_settings(source_cap_settings) {}

    bool operator==(const PluginSettings &rhs) const {
        return enabled == rhs.enabled &&
               source_cap_settings == rhs.source_cap_settings;
    }

    bool operator!=(const PluginSettings &rhs) const {
        return !(rhs == *this);
    }

    void print(const char *line_prefix = "") {
        printf("%PluginSettings\n", line_prefix);
        printf("%senabled: %d\n", line_prefix, enabled);
        source_cap_settings.print((std::string(line_prefix) + "").c_str());
    }
};


#endif //OBS_SPEECH2TEXT_SETTINGS_H