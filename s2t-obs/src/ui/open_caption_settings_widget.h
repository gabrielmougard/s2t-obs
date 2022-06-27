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

#ifndef OBS_SPEECH2TEXT_PLUGIN_OPENCAPTIONSETTINGSWIDGET_H
#define OBS_SPEECH2TEXT_PLUGIN_OPENCAPTIONSETTINGSWIDGET_H

#include "ui_open_caption_settings_widget.h"

#include "utils/utils_ui.h"

#include <QPushButton>

namespace ui {

class OpenCptionSettingsWidget : public QWidget, Ui_OpenCaptionSettingsWidget {
Q_OBJECT
public:
    OpenCaptionSettingsWidget(QWidget *parent, backend::TextSettings settings) : QWidget(parent), Ui_OpenCaptionSettingsWidget() {
        setupUi(this);
        QObject::connect(this->enableCheckBox, &QCheckBox::stateChanged, this, &OpenCaptionSettingsWidget::on_enabledCheckBox_stateChanged);
        QObject::connect(this->deleteToolButton, &QCheckBox::clicked, this, &OpenCaptionSettingsWidget::on_deleteToolButton_clicked);

        setup_combobox_capitalization(*capitalizationComboBox);

        this->enabledCheckBox->setChecked(settings.enabled);
        on_enabledCheckBox_stateChanged(settings.enabled);

        this->punctuationCheckBox->setChecked(settings.insert_punctuation);

        auto text_sources = get_text_sources();
        text_sources.insert(text_sources.begin(), "");
        setup_combobox_texts(*outputComboBox, text_sources);

        this->lineLengthSpinBox->setValue(settings.line_length);
        this->lineCountSpinBox->setValue(settings.line_count);
        combobox_set_data_int(*capitalizationComboBox, settings.capitalization, 0);

        this->outputComboBox->setCurrentText(QString::fromStdString(settings.text_source_name));
    }

    void on_enabledCheckBox_stateChanged(int new_state) {
        const bool disabled = !new_state;
        this->textSourceWidget->setDisabled(disabled);
    }

    void on_deleteToolButton_clicked() {
        emit deleteClicked(this);
    }

    backend::TextSettings getSettings() {
        backend::TextSettings settings;
        settings.enabled = this->enabledCheckBox->isChecked();
        settings.text_source_name = this->outputComboBox->currentText().toStdString();
        settings.line_count = this->lineCountSpinBox->value();
        settings.line_length = this->lineLengthSpinBox->value();
        settings.insert_punctuation = this->punctuationCheckBox->isChecked();
        settings.capitalization = (CapitalizationType) this->capitalizationComboBox->currentData().toInt();
        return settings;
    }

signals:
    void deleteClicked(OpenCaptionSettingsWidget *self);
};

class OpenCaptionSettingsList : public QWidget {
Q_OBJECT
private:
    std::vector<OpenCaptionSettingsWidget> widgets;
public:
    OpenCaptionSettingsList(QWidget *parent) : QWidget(parent) {
        auto layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);

        auto button = new QPushButton("Add Text Output");
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QObject::connect(button, &QPushButton::clicked, this, &OpenCaptionSettingsList::addDefault);
        layout->addWidget(button);

        layout->setSpacing(0);
        this->setLayout(layout);
    }

    void addDefault() {
        addSettingsWidget(default_TextSettings());
    }

    void addSettingsWidget(const backend::TextSettings &settings) {
        auto widget = new OpenCaptionSettingsWidget(this, settings);
        QObject::connect(widget, &OpenCaptionSettingsWidget::deleteClicked,
                         this, &OpenCaptionSettingsList::deleteSettingsWidget);
        this->layout()->addWidget(widget);
    }

    void setSettings(const vector<backend::TextSettings> &settings) {
        clearLayout(this->layout(), 1);

        for (const auto &i: settings) {
            addSettingsWidget(i);
        }
    }

    std::vector<backend::TextSettings> getSettings() {
        std::vector<backend::TextSettings> settings;
        for (int i = 1; i < this->layout()->count(); i++) {
            auto item = this->layout()->itemAt(i);
            if (!item)
                continue;

            auto widget = item->widget();
            if (!widget)
                continue;

            auto *settingsWidget = dynamic_cast<OpenCaptionSettingsWidget *>(widget);
            if (!settingsWidget)
                continue;

            settings.push_back(settingsWidget->getSettings());
        }
        return settings;
    }

public slots:
    void deleteSettingsWidget(OpenCaptionSettingsWidget *target) {
        if (!target)
            return;
        for (int i = 1; i < this->layout()->count(); i++) {
            auto item = this->layout()->itemAt(i);
            if (!item)
                continue;

            auto widget = item->widget();
            if (!widget || widget != target)
                continue;

            this->layout()->removeItem(item);
            widget->hide();
            widget->setParent(nullptr);
            delete widget;
            return;
        }
    };
};

}

#endif //OBS_SPEECH2TEXT_PLUGIN_OPENCAPTIONSETTINGSWIDGET_H