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

CaptionManager::CaptionManager(const CaptionPluginSettings &initial_settings) : plugin_settings(initial_settings), source_captioner() {}

void CaptionManager::external_state_changed(bool is_live, bool is_preview_open, bool is_recording, bool is_streaming, const std::string &scene_collection_name) {
    state.external_is_streaming = is_live;
    state.external_is_recording = is_recording;
    state.external_is_preview_open = is_preview_open;
    state.external_is_virtualcam_on = is_virtualcam_on;
    state.external_scene_collection_name = scene_collection_name;

    update_settings(plugin_settings); 
}

void CaptionManager::set_settings(const CaptionPluginSettings &new_settings) {
    update_settings(new_settings);
}

void CaptionManager::update_settings(const CaptionPluginSettings &mew_settings) {
    const CaptionSourceSettings &source_settings = new_settings.caption_source_settings;
    
}

}