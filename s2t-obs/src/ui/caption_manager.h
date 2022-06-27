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

namespace ui {

struct CaptionState {
    bool external_is_streaming = false;
    bool external_is_recording = false;
    bool external_is_virtualcam_on = false;
    bool external_is_preview_open = false;
    std::string external_scene_collection_name = "";

    bool is_captioning = false;
    bool is_captioning_streaming = false;
    bool is_captioning_recording = false;
    bool is_captioning_virtualcam = false;
    bool is_captioning_preview = false;
    bool is_captioning_text_output = false;
    std::string captioning_scene_collection_name = "";
};

class CaptionManager : public QObject {
Q_OBJECT

public:
    CaptionManager(CaptionPluginSettings &initial_settings);

    void external_state_changed(bool is_live, bool is_preview_open, bool is_recording, bool is_streaming, const std::string &scene_collection_name);
    void update_settings(const CaptionPluginSettings &new_settings);
    bool toggle_enabled();

    CaptionPluginSettings plugin_settings;
    CaptionSource source_captioner;
    CaptionState state;

public slots:
    void set_settings(const CaptionPluginSettings &new_settings);

signals:
    void settings_changed(CaptionPluginSettings &new_settings);

private:
    int update_count = 0;
};
}
