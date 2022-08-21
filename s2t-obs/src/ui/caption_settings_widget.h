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

#ifndef OBS_SPEECH2TEXT_PLUGIN_CAPTIONSETTINGSWIDGET_H
#define OBS_SPEECH2TEXT_PLUGIN_CAPTIONSETTINGSWIDGET_H

#include <QWidget>

#include "ui_caption_settings_widget.h"
#include "open_caption_settings_widget.h"

#include "backend/caption_stream.h"
#include "backend/audio_capture_pipeline.h"
#include "backend/caption.h"
#include "backend/settings.h"


namespace ui {

class CaptionSettingsWidget : public QObject, Ui_CaptionSettingsWidget {
Q_OBJECT

    CaptionSettingsWidget current_settings;
    std::string scene_collection_name;
    OpenCaptionSettingsList *caption_settings_widget;

    void accept_current_settings();

private slots:
    void transcript_format_index_change(int index);
    void language_index_change(int index);
    void recording_name_index_change(int index);
    void streaming_name_index_change(int index);
    void virtualcam_name_index_change(int index);
    void sources_combo_index_change(int index);
    void caption_when_index_change(int index);
    void scene_collection_combo_index_change(int index);
    void on_previewPushButton_clicked();
    void on_replacementWordsAddWordPushButton_clicked();
    void on_transcriptFolderPickerPushButton_clicked();
    void on_saveTranscriptsCheckBox_stateChanged(int new_state);
    void on_enabledCheckBox_stateChanged(int new_state);
    void apply_ui_scene_collection_settings();
    void update_scene_collection_ui(const std::string &use_scene_collection_name);

signals:
    void settings_accepted(CaptionSettings settings);
    void preview_requested();

public:
    CaptionSettingsWidget(const CaptionSettings &settings);
    void set_settings(const CaptionSettings &new_settings);
    void update_ui();
    void update_sources_visibilities();

};

}

#endif //OBS_SPEECH2TEXT_PLUGIN_CAPTIONSETTINGSWIDGET_H