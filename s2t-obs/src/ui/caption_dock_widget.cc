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

#include "caption_dock_widget.h"

#include "utils/ui.h"

#include <QLabel>
#include <QLineEdit>

namespace ui {

CaptionDockWidget::CaptionDockWidget(const QString &title, CaptionManager &manager, CaptionMainWidget &caption_main_widget)
        : QDockWidget(title), manager(manager), caption_main_widget(caption_main_widget) {
    setupUi(this);
    setWindowTitle(title);
    captionLinesPlainTextEdit->clear();

    setFeatures(QDockWidget::AllDockWidgetFeatures);
    setFloating(true);

    QObject::connect(&manager.source_captioner, &SourceCaptioner::caption_source_status_changed, this, &CaptionDockWidget::handle_source_capture_status_change, Qt::QueuedConnection);
    QObject::connect(&manager.source_captioner, &SourceCaptioner::caption_result_received, this, &CaptionDockWidget::handle_caption_data_cb, Qt::QueuedConnection);

    QFontMetrics fm = this->captionLinesPlainTextEdit->fontMetrics();
    spdlog::info("dock: {} {} fs: {}", this->minimumWidth(), this->maximumWidth(), this->captionLinesPlainTextEdit->font().pointSize());

    const int target_width = fm.width("BASELINE") + 30;
    spdlog::info("target {} {}", target_width, 0);
    if (target_width >= 150 && target_width <= 350) {
        this->setMaximumWidth(target_width);
        this->setMinimumWidth(target_width);
    }
    this->verticalLayout->setAlignment(Qt::AlignTop);
}

void CaptionDockWidget::handle_source_capture_status_change(std::shared_ptr<SourceCaptionerStatus> status) {
    if (!status)
        return;

    std::string status_text;
    captioning_status_string(
        manager.plugin_settings.enabled,
        status->settings.streaming_output_enabled,
        status->settings.recording_output_enabled,
        manager.captioning_state(),
        *status,
        status_text);

    this->statusTextLabel->setText(QString::fromStdString(status_text));
}

void CaptionDockWidget::handle_caption_data_cb(
    std::shared_ptr<OutputCaptionResult> caption_result,
    bool cleared,
    std::string recent_caption_text
) {
    if (cleared) {
        captionLinesPlainTextEdit->setPlainText("");
        last_output_line = "";
        return;
    }

    if (!caption_result)
        return;

    if (caption_result->output_line == last_output_line)
        return;

    std::string single_caption_line;
    for (std::string &a_line : caption_result->output_lines) {
        if (!single_caption_line.empty())
            single_caption_line.push_back('\n');
        single_caption_line.append(a_line);
    }

    this->captionLinesPlainTextEdit->setPlainText(QString::fromStdString(single_caption_line));
    last_output_line = caption_result->output_line;
}

void CaptionDockWidget::on_settingsToolButton_clicked() {
    caption_main_widget.show_settings_widget();
}

}
