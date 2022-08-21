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

#ifndef OBS_SPEECH2TEXT_PLUGIN_CAPTIONMAINWIDGET_H
#define OBS_SPEECH2TEXT_PLUGIN_CAPTIONMAINWIDGET_H

#include "ui_caption_main_widget.h"

#include "backend/caption.h"
#include "backend/settings.h"
#include "caption_manager.h"
#include "caption_settings_widget.h"

#include <QWidget>
#include <spdlog/spdlog.h>
#include <blockingconcurrentqueue.h>


namespace ui {

typedef std::tuple<std::shared_ptr<CaptionResult>, bool, std::string> CaptionResultTup;

class CaptionMainWidget : public QWidget, Ui_CaptionMainWidget {
Q_OBJECT
    CaptionManager &manager;
    CaptionSettingsWidget caption_settings_widget;
    std::unique_ptr<CaptionResultTup> latest_caption_result_tup;

signals:
    void process_item_queue();

private:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QShowEvent *event) override;
    void do_process_item_queue();
    void accept_widget_settings(CaptionSettings new_settings);
    void handle_caption_data_cb(std::shared_ptr<OutputCaptionResult> caption_result, bool cleared, std::string recent_catpion_text);
    void handle_caption_source_status_change(std::shared_ptr<CaptionSourceStatus> status);
    void settings_changed_event(CaptionSettings &settings);

public slots:
    void show_settings_widget();
    void enabled_state_checkbox_changed(int state);
    
public:
    MainCaptionWidget(CaptionManager &manager);
    void show_self();
    virtual ~MainCaptionWidget();
    void handle_caption_result_tup(CaptionResultTup &tup);
    void update_caption_text_ui();
    void external_state_changed();
    void scene_collection_changed();
    void stream_started_event();
    void stream_stopped_event();
    void recording_started_event();
    void recording_stopped_event();
    void virtualcam_started_event();
    void virtualcam_stopped_event();
};

}

#endif //OBS_SPEECH2TEXT_PLUGIN_CAPTIONMAINWIDGET_H