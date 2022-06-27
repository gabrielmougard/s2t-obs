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


#include "caption_main_widget.h"
#include "util/ui_utils.h"
#include "util/caption_stream_helper.h"

#include <QTimer>
#include <QThread>

namespace ui {

CaptionMainWidget::CaptionMainWidget(PluginManager &manager) :
QWidget(),
Ui_CaptionMainWidget(),
manager(manager),
caption_settings_widget(manager.get_settings()) {

    setupUi(this);

    this->captionHistoryPlainTextEdit->setPlainText("");
    this->captionLinesPlainTextEdit->setPlainText("");

    QObject::connect(this->enabledCheckbox, &QCheckBox::stateChanged, this, &CaptionMainWidget::enabled_plugin_change);
    QObject::connect(this->settingsToolButton, &QToolButton::clicked, this, &CaptionMainWidget::show_settings_widget);
    statusTextLabel->hide();
    
    QObject::connect(&caption_settings_widget, &CaptionSettingsWidget::settings_accepted, this, &CaptionMainWidget::accept_widget_settings);
    QObject::connect(&caption_settings_widget, &CaptionSettingsWidget::preview_requested, this &CaptionMainWidget::show_self);
    QObject::connect(&manager.source_captioner, &CaptionSource::caption_result_received, this, &CaptionMainWidget::handle_caption_data_cb, Qt::QueuedConnection);
    QObject::connect(&manager.source_captioner, &CaptionSource::caption_source_status_changed, this, &CaptionMainWidget::handle_caption_source_status_change, Qt::QueuedConnection);
    QObject::connect(&manager.source_captioner, &CaptionSource::settings_changed, this, &CaptionMainWidget::settings_changed_event);

    settings_changed_event(manager.plugin_settings);
}

CaptionMainWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    update_caption_text_ui();
    external_state_changed();
}

CaptionMainWidget::hideEvent(QShowEvent *event) {
    QWidget::hideEvent(event);

    update_caption_text_ui();
    external_state_changed();
}

void CaptionMainWidget::update_caption_text_ui() {
    if (!latest_caption_result_tup)
        return;

    auto latest_caption_result = std::get<0>(*latest_caption_result_tup);
    bool was_cleared = std::get<1>(*latest_caption_result_tup);
    std::string latest_caption_text_history = std::get<2>(*latest_caption_result_tup);

    if (was_cleared) {
        this->captionLinesPlainTextEdit->clear();
    } else {
        string single_caption_line;
        for (string &a_line: latest_caption_result->output_lines) {
            if (!single_caption_line.empty())
                single_caption_line.push_back('\n');

            single_caption_line.append(a_line);
        }

        this->captionLinesPlainTextEdit->setPlainText(QString::fromStdString(single_caption_line));
    }

    this->captionHistoryPlainTextEdit->setPlainText(latest_caption_text_history.c_str());
}

void CaptionMainWidget::external_state_changed() {
    manager.external_state_changed(is_streaming_live(), is_recording_live(), is_virtualcam_on());
}

void CaptionMainWidget::scene_collection_changed() {
    caption_settings_widget.hide();
}

void CaptionMainWidget::stream_started_event() {
    manager.source_captioner.stream_started_event();
    external_state_changed();
}

void CaptionMainWidget::stream_stopped_event() {
    manager.source_captioner.stream_stopped_event();
    external_state_changed();
}

void CaptionMainWidget::recording_started_event() {
    manager.source_captioner.recording_started_event();
    external_state_changed();
}

void CaptionMainWidget::recording_stopped_event() {
    manager.source_captioner.recording_stopped_event();
    external_state_changed();
}

void CaptionMainWidget::virtualcam_started_event() {
    manager.source_captioner.virtualcam_started_event();
    external_state_changed();
}

void CaptionMainWidget::virtualcam_stopped_event() {
    manager.source_captioner.virtualcam_stopped_event();
    external_state_changed();
}

void CaptionMainWidget::handle_caption_data_cb(std::shared_ptr<OutputCaptionResult> caption_result, bool cleared, std::string recent_caption_text) {
    latest_caption_result_tup = std::make_unique<CaptionResultTup>(caption_result, cleared, recent_caption_text);
    if (isVisible())
        this->update_caption_text_ui();
}

void CaptionMainWidget::show_self() {
    show();
    raise();
}

void CaptionMainWidget::show_settings_widget() {
    spdlog::debug("CaptionMainWidget show_settings_widget");
    if (caption_settings_widget.isHidden()) {
        spdlog::debug("updating caption widget settings");
        caption_settings_widget.set_settings(manager.plugin_settings);
    }

    caption_settings_widget.show();
    caption_settings_widget.raise();
}

void CaptionMainWidget::accept_widget_settings(CaptionSettings new_settings) {
    spdlog::info("CaptionMainWidget accept_widget_settings: {}", &caption_settings_widget);
    new_settings.print();
    caption_settings_widget.hide();
    manager.update_settings(new_settings);
}

void CaptionMainWidget::handle_caption_source_status_change(std::shared_ptr<CaptionSourceStatus> status) {
    spdlog::info("CaptionMainWidget handle_caption_source_status_settings");

    // conversion
    std::string status_text;
    captioning_status_string(manager.plugin_settings.enabled, status->settings.streaming_output_enabled, status->settings.recording_output_enabled, manager.captioning_state(), *status, status_text);

    this->statusTextLabel->setText(status_text.c_str());
    this->statusTextLabel->setVisible(true);
}

void CaptionMainWidget::enabled_state_checkbox_changed(int stated) {
    spdlog::info("CaptionMainWidget enabled_state_checkbox_changed");
    manager.toggle_enabled();
}

void CaptionMainWidget::settings_changed_event(CaptionSettings new_settings) {
    spdlog::info("CaptionMainWidget settings_changed_event");

    if (new_settings.enabled != enabledCheckbox->isChecked()) {
        const QSignalBlocker blocker(enabledCheckbox);
        enabledCheckbox->setChecked(new_settings.enabled);
    }
}

CaptionMainWidget::~CaptionMainWidget() {
    spdlog::info("~CaptionMainWidget");
    manager.source_captioner.stop_caption_stream(false);
}

}


