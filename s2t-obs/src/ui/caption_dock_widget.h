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

#ifndef OBS_SPEECH2TEXT_PLUGIN_CAPTIONDOCKWIDGET_H
#define OBS_SPEECH2TEXT_PLUGIN_CAPTIONDOCKWIDGET_H

#include "ui_caption_dock_widget.h"

#include "backend/caption_source.h"
#include "caption_manager.h"
#include "caption_main_widget.h"

#include <QDockWidget>
#include <spdlog/spdlog.h>

namespace ui {

class CaptionDockWidget : public QDockWidget, Ui_CaptionDockWidget {
Q_OBJECT
private:
    CaptionManager &manager;
    CaptionMainWidget &caption_main_widget;
    std::string last_output_line;

    void handle_caption_data_cb(std::shared_ptr<OutputCaptionResult> caption_result, bool cleared, std::string recent_caption_text);

private slots:
    void on_settingsToolButton_clicked();

public:
    CaptionDockWidget(const QString &title, CaptionManager &manager, CaptionMainWidget &caption_main_widget);
    void handle_source_capture_status_change(std::shared_ptr<CaptionSourceStatus> status);==
};

}

#endif //OBS_SPEECH2TEXT_PLUGIN_CAPTIONDOCKWIDGET_H