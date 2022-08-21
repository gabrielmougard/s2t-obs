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

#include "caption_settings_widget.h"

#include "backend/settings.h"

#include <QLineEdit>
#include <QFileDialog>

namespace ui {

#define WORD_REPLACEMENT_TYPE_INDEX 0
#define WORD_REPLACEMENT_FROM_INDEX 1
#define WORD_REPLACEMENT_TO_INDEX 2
#define WORD_REPLACEMENT_DELETE_INDEX 3

#define AUDIO_TRACK_COUNT 6

static void setup_combobox_texts_data(QComboBox &combo_box, const std::vector<std::string> &items) {
    while (combo_box.count())
        combo_box.removeItem(0);

    for (auto &item : items) {
        auto str = QString::fromStdString(item);
        combo_box.addItem(str, str);
    }
}

static int update_combobox_with_current_scene_collections(QComboBox &combo_box) {
    while (combo_box.count())
        combo_box.removeItem(0);

    char **names = obs_frontend_get_scene_collections();
    char **name = names;

    int c = 0;
    while (name && *name) {
        combo_box.addItem(QString(*name));
        name++;
        c++;
    }
    bfree(names); // obs buffer free memory
    return c;
}

static int combobox_set_data_str(QComboBox &combo_box, const char *data, int default_index) {
    int index = combo_box.finalData(data);
    if (index == -1)
        index = default_index;

    combo_box.setCurrentIndex(index);
    return index;
}

QComboBox *wordReplacementTypeComboBox(const char *default_text) {
    auto *cb = new QComboBox();
    cb->addItem("Text", "text_case_insensitive"); // Ignore case
    cb->addItem("Regex", "regex_case_insensitive"); // Ignore case
    combobox_set_data_str(*cb, default_text, 0);
    return cb;
}

QPushButton *wordReplacementDeletePushButton(QTableWidget *table_widget) {
    auto *button = new QPushButton("X");
    QObject::connect(button, &QPushButton::clicked, table_widget, [=]() {
        int found_at = -1;
        for (int row = 0; row < table_widget->rowCount(); row++) {
            const QWidget *wid = table_widget->cellWidget(row, WORD_REPLACEMENT_DELETE_INDEX);
            if (wid == button) {
                found_at = row;
                break;
            }
        }

        if (found_at != -1) {
            table_widget->removeRow(found_at);
            spdlog::debug("remove index {}", found_at);
        } else {
            spdlog::debug("can't remove, index not found");
        }
    });
    return button;
}

void setupReplacementTableWidget(QTableWidget *table_widget, const std::vector<backend::Filter> &filters) {
    table_widget->setRowCount(0);
    int row = 0;
    for (auto &filter : filters) {
        table_widget->setRowCount(row + 1);
        table_widget->setCellWidget(row, WORD_REPLACEMENT_TYPE_INDEX, wordReplacementTypeComboBox(filter.get_type().c_str()));

        auto from = new QTableWidgetItem(QString::fromStdString(filter.get_from()));
        table_widget->setItem(row, WORD_REPLACEMENT_FROM_INDEX, from);

        auto to = new QTableWidgetItem(QString::fromStdString(filter.get_to()));
        table_widget->setItem(row, WORD_REPLACEMENT_TO_INDEX, to);

        table_widget->setCellWidget(row, WORD_REPLACEMENT_DELETE_INDEX, wordReplacementDeletePushButton(table_widget));
        row++;
    }
}

void getFilters(QTableWidget *table_widget, std::vector<backend::Filter> &filters) {
    filters.clear();
    for (int row = 0; row < table_widget->rowCount(); row++) {
        std::string type;
        std::string from;
        std::string to;

        QComboBox *type_box = dynamic_cast<QComboBox *>(table_widget->cellWidget(row, WORD_REPLACEMENT_TYPE_INDEX));
        if (type_box) {
            auto type_str = type_box->currentData().toString().toStdString();
            if (isValidWordReplacementTypeString(type_str))
                type = type_str;
        }

        if (QTableWidgetItem *item = table_widget->item(row, WORD_REPLACEMENT_FROM_INDEX)) {
            from = item->text().toStdString();
        }

        if (QTableWidgetItem *item = table_widget->item(row, WORD_REPLACEMENT_TO_INDEX)) {
            to = item->text().toStdString();
        }

        if (!type.empty() && !from.empty()) {
            filters.push_back(backend::Filter(type, from, to));
        }
    }
}

CaptionSettingsWidget::CaptionSettingsWidget(const CaptionSettings &latests_settings)
        : QWidget(), Ui_CaptionSettingsWidgets(), current_settings(latests_settings), caption_settings_widget(nullptr) {
    
    setupUI(this);
    caption_settings_widget = new OpenCaptionSettingsLists(this);
    this->textOutputsListScrollArea->setWidget(caption_settings_widget);

    QString with_version = bottomTextBrowser->toPlainText().replace("${VERSION}", VERSION_STRING);
    bottomTextBrowser->setPlainText(with_version);

    this->verticalLayout->setAlignment(Qt::AlignTop);
    this->errorLabel->hide();
    this->updateUi();

    captionWhenComboBox->addItem("Caption source is heard on stream", "own_source");
    captionWhenComboBox->addItem("Mute source is heard on stream", "other_mute_source");

    //setup_combobox_languages(*languageComboBox);
    setup_combobox_profanity(*profanityFilterComboBox);
    setup_combobox_capitalization(*capitalizationComboBox);
    setup_combobox_capitalization(*srtCapitalizationComboBox);
    setup_combobox_output_target(*outputTargetComboBox);
    setup_combobox_transcript_format(*transcriptFormatComboBox);

    setup_combobox_recording_filename(*recordingTranscriptFilenameComboBox);
    setup_combobox_streaming_filename(*streamingTranscriptFilenameComboBox);
    setup_combobox_virtualcam_filename(*virtualcamTranscriptFilenameComboBox);
    
    setup_combobox_transcript_file_exists(*recordingTranscriptCustomNameExistsCombobox);
    setup_combobox_transcript_file_exists(*streamingTranscriptCustomNameExistsCombobox);
    setup_combobox_transcript_file_exists(*virtualcamTranscriptCustomNameExistsCombobox);

    QObject::connect(this->cancelPushButton, &QPushButton::clicked, this, &CaptionSettingsWidget::hide);
    QObject::connect(this->savePushButton, &QPushButton::clicked, this, &CaptionSettingsWidget::accept_current_settings);
    
    QObject::connect(captionWhenComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CaptionSettingsWidget::caption_when_index_change);
    QObject::connect(transcriptFormatComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CaptionSettingsWidget::transcript_format_index_change);
    //QObject::connect(languageComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CaptionSettingsWidget::language_index_change);
    QObject::connect(recordingTranscriptFilenameComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CaptionSettingsWidget::recording_name_index_change);
    QObject::connect(streamingTranscriptFilenameComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CaptionSettingsWidget::streaming_name_index_change);
    QObject::connect(virtualcamTranscriptFilenameComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CaptionSettingsWidget::virtualcam_name_index_change);
    QObject::connect(sourcesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CaptionSettingsWidget::sources_combo_index_change);

    QStringList labels = {"Type", "Replace this", "With this", ""};
    this->wordReplacementTableWidget->setHorizontalHeaderLabels(labels);
    this->worldReplacementTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    this->worldReplacementTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    this->worldReplacementTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    this->worldReplacementTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    this->worldReplacementTableWidget->setColumnWidth(0, 80);
    this->worldReplacementTableWidget->setColumnWidth(3, 25);
}

void CaptionSettingsWidget::on_previewPushButton_clicked() {
    emit preview_requested();
}

void CaptionSettingsWidget::sources_combo_index_change(int new_index) {
    this->update_sources_visibilities();
}

void CaptionSettingsWidget::caption_when_index_change(int new_index) {
    this->update_sources_visibilities();
}

void CaptionSettingsWidget::update_sources_visibilities() {
    MuteType mute_state = string_to_mute_setting(captionWhenComboBox->currentData().toString().toStdString(), CAPTION_SOURCE_MUTE_TYPE_FROM_OWN_SOURCE);

    bool caption_when_visible = false;
    bool mute_source_visible = false;

    bool all_audio_capture_enabled = is_all_audio_output_capture_source_data(sourcesComboBox->currentData().toString().toStdString());
    if (all_audio_capture_enabled) {
        caption_when_visible = false;
        mute_source_visible = false;
    } else {
        caption_when_visible = true;
        mute_source_visible = mute_state == CAPTION_SOURCE_MUTE_TYPE_USE_OTHER_MUTE_SOURCE;
    }

    this->muteSourceComboBox->setVisible(mute_source_visible);
    this->muteSourceLabel->setVisible(mute_source_visible);

    captionWhenComboBox->setVisible(caption_when_visible);
    captionWhenLabel->setVisible(caption_when_visible);
}

void CaptionSettingsWidget::apply_ui_scene_collection_settings() {
    std::string current_scene_collection_name = scene_collection_name;
    spdlog::debug("apply_ui_scene_collection_settings {}", current_scene_collection_name.c_str());

    SceneSettings scene_col_settings;
    CaptionSettings &caption_settings = scene_col_settings.caption_settings;

    caption_settings.caption_source_name = sourcesComboBox->currentData().toString().toStdString();
    caption_settings.mute_source_name = muteSourceComboBox->currentText().toStdString();

    std::string when_str = captionWhenComboBox->currentData().toString().toStdString();
    caption_settings.mute_when = string_to_mute_setting(when_str, CAPTION_SOURCE_MUTE_TYPE_FROM_OWN_SOURCE);

    scene_col_settings.text_outputs = this->caption_settings_widget->getSettings();
    current_settings.source_cap_settings.update_setting(current_scene_collection_name, scene_col_settings);
}

void CaptionSettingsWidget::update_scene_collection_ui(const std::string &use_scene_collection_name) {
    CaptionSourceSettings &source_settings = current_settings.source_cap_settings;
    SceneSettings use_settings = default_SceneSettings();
    
    use_settings = source_settings.get_scene_settings(use_scene_collection_name);
    spdlog::debug("update_scene_collection_ui using specific settings, '{}'", use_scene_collection_name.c_str());

    // get audio input sources
    auto audio_sources = util::get_audio_sources();
    audio_sources.insert(audio_sources.begin(), "");
    audio_sources.insert(audio_sources.end(), "");

    const QSignalBlocker blocker1(sourcesComboBox);
    setup_combobox_texts_data(*sourcesComboBox, audio_sources);

    for (int i = 0; i < AUDIO_TRACK_COUNT; i++) {
        sourcesComboBox->addItem(QString::fromStdString(all_audio_output_capture_source_name(i)),
                                 QString::fromStdString(all_audio_output_capture_source_data(i)));
    }

    const QSignalBlocker blocker2(muteSourceComboBox, audio_sources);
    setup_combobox_texts(*muteSourceComboBox, audio_sources);

    const QSignalBlocker blocker3(captionWhenComboBox);

    combobox_set_data_str(*sourcesComboBox, use_settings.caption_source_settings.caption_source_name.c_str(), 0);
    muteSourceComboBox->setCurrentText(QString::fromStdString(use_settings.caption_source_settings.mute_when, "own_source"));

    this->update_sources_visibilities();
    this->caption_settings_widget->setSettings(use_settings.text_outputs);
}

void CaptionSettingsWidget::transcript_format_index_change(int new_index) {
    const bool isSrt = transcriptFormatComboBox->currentData().toString().toStdString() == "srt";
    transcriptSrtSettingsWidget->setVisible(isSrt);

    const auto format = transcriptFormatComboBox->currentData().toString().toStdString();
    const QString extension = QString::fromStdString(transcript_format_extension(format, "[ext]"));
    update_combobox_recording_filename(*recordingTranscriptFilenameComboBox, extension);
    update_combobox_streaming_filename(*streamingTranscriptFilenameComboBox, extension);
    update_combobox_virtualcam_filename(*virtualcamTranscriptFilenameComboBox, extension);
}

void CaptionSettingsWidget::language_index_change(int new_index) {
    string lang_str = languageComboBox->currentData().toString().toStdString();

    const bool native_output = supports_native_output(lang_str);
    languageWarningLabel->setVisible(!native_output);

    // this->nativeForceLinebreaksLabel->setVisible(native_output);
    // this->nativeLineCountLabel->setVisible(native_output);
    // this->nativeLinebreaksNotRecommendendLabel->setVisible(native_output);
    // this->nativeOutputToLabel->setVisible(native_output);

    // this->outputTargetComboBox->setVisible(native_output);
    // this->lineCountSpinBox->setVisible(native_output);
    // this->insertLinebreaksCheckBox->setVisible(native_output);
}

void CaptionSettingsWidget::scene_collection_combo_index_change(int new_index) {
    // std::string current_scene_collection_name = this->sceneCollectionComboBox->currentText().toStdString();
    // spdlog::debug("scene_collection_combo_index_change {}", current_scene_collection_name.c_str());
    // update_scene_collection_ui(current_scene_collection_name);
}

void CaptionSettingsWidget::recording_name_index_change(int new_index) {
    bool isCustom = this->recordingTranscriptFilenameComboBox->currentData().toString() == "custom";

    recordingTranscriptCustomNameWidget->setVisible(isCustom);
    recordingTranscriptCustomNameOverwriteLineEdit->setVisible(isCustom);
}

void CaptionSettingsWidget::streaming_name_index_change(int new_index) {
    bool isCustom = this->streamingTranscriptFilenameComboBox->currentData().toString() == "custom";

    streamingTranscriptCustomNameWidget->setVisible(isCustom);
    streamingTranscriptCustomNameOverwriteLineEdit->setVisible(isCustom);
}


void CaptionSettingsWidget::virtualcam_name_index_change(int new_index) {
    bool isCustom = this->virtualcamTranscriptFilenameComboBox->currentData().toString() == "custom";

    virtualcamTranscriptCustomNameWidget->setVisible(isCustom);
    virtualcamTranscriptCustomNameOverwriteLineEdit->setVisible(isCustom);
}

void CaptionSettingsWidget::accept_current_settings() {
    CaptionSourceSettings &source_settings = current_settings.source_cap_settings;
    
    std::string lang_str = languageComboBox->currentData().toString().toStdString();
    source_settings.stream_settings.stream_settings.language = lang_str;
    spdlog::debug("language: {}", lang_str.c_str());

    const int profanity_filter = profanityFilterComboBox->currentData().toInt();
    source_settings.stream_settings.stream_settings.profanity_filter = profanity_filter;
    spdlog::debug("profanity filter: {}", profanity_filter);

    source_settings.format_settings.caption_line_count = lineCountSpinBox->value();
    source_settings.format_settings.capitalization = (CapitalizationType) capitalizationComboBox->currentData().toInt();
    source_settings.format_settings.caption_insert_newlines = insertLinebreaksCheckBox->isChecked();
    source_settings.format_settings.caption_insert_punctuation = addPunctuationCheckBox->isChecked();
    current_settings.enabled = enabledCheckBox->isChecked();

    // just for now as the foreign langs list might have errors or some only partially use non native charsets
    const bool still_try_foreign_langs = true;

    if (still_try_foreign_langs || supports_native_output(lang_str)) {
        const int output_combobox_val = outputTargetComboBox->currentData().toInt();
        if (!set_streaming_recording_enabled(output_combobox_val,
                                             source_settings.streaming_output_enabled, source_settings.recording_output_enabled)) {
            spdlog::error("invalid output target combobox value: {}", output_combobox_val);
        }
    } else {
        source_settings.streaming_output_enabled = false;
        source_settings.recording_output_enabled = false;
    }

    source_settings.format_settings.caption_timeout_enabled = this->captionTimeoutEnabledCheckBox->isChecked();
    source_settings.format_settings.caption_timeout_seconds = this->captionTimeoutDoubleSpinBox->value();

    auto filters = std::vector<backend::Filter>();
    getFilters(wordReplacementTableWidget, filters);
    source_settings.format_settings.filter = makeDefaultFilter(filters);

    auto &transcript_settings = source_settings.transcript_settings;
    transcript_settings.enabled = saveTranscriptsCheckBox->isChecked();
    transcript_settings.streaming_transcripts_enabled =  transcriptsEnabledStreamCheckBox->isChecked();
    transcript_settings.recording_transcripts_enabled =  transcriptsEnabledRecordingCheckBox->isChecked();
    transcript_settings.virtualcam_transcripts_enabled =  transcriptsEnabledVirtualcamCheckBox->isChecked();

    transcript_settings.output_path = transcriptFolderPathLineEdit->text().toStdString();
    transcript_settings.format = transcriptFormatComboBox->currentData().toString().toStdString();
    transcript_settings.srt_target_duration_secs = srtDurationSpinBox->value();
    transcript_settings.srt_target_line_length = srtLineLengthSpinBox->value();
    transcript_settings.srt_add_punctuation = srtAddPunctuationCheckBox->isChecked();
    transcript_settings.srt_split_single_sentences = srtSplitSentencesCheckBox->isChecked();
    transcript_settings.srt_capitalization = (CapitalizationType) srtCapitalizationComboBox->currentData().toInt();

    transcript_settings.recording_filename_type = recordingTranscriptFilenameComboBox->currentData().toString().toStdString();
    transcript_settings.recording_filename_custom = recordingTranscriptCustomNameOverwriteLineEdit->text().toStdString();
    transcript_settings.recording_filename_exists = recordingTranscriptCustomNameExistsCombobox->currentData().toString().toStdString();

    transcript_settings.streaming_filename_type = streamingTranscriptFilenameComboBox->currentData().toString().toStdString();
    transcript_settings.streaming_filename_custom = streamingTranscriptCustomNameOverwriteLineEdit->text().toStdString();
    transcript_settings.streaming_filename_exists = streamingTranscriptCustomNameExistsCombobox->currentData().toString().toStdString();

    transcript_settings.virtualcam_filename_type = virtualcamTranscriptFilenameComboBox->currentData().toString().toStdString();
    transcript_settings.virtualcam_filename_custom = virtualcamTranscriptCustomNameOverwriteLineEdit->text().toStdString();
    transcript_settings.virtualcam_filename_exists = virtualcamTranscriptCustomNameExistsCombobox->currentData().toString().toStdString();

    apply_ui_scene_collection_settings();
    spdlog::debug("accepting changes");
    emit settings_accepted(current_settings);
}

void CaptionSettingsWidget::updateUi() {
    CaptionSourceSettings &source_settings = current_settings.source_cap_settings;

    this->generalSceneCollectionWidget->hide();

    update_scene_collection_ui();

    combobox_set_data_str(*languageComboBox, source_settings.stream_settings.stream_settings.language.c_str(), 0);
    language_index_change(0);
    combobox_set_data_int(*profanityFilterComboBox, source_settings.stream_settings.stream_settings.profanity_filter, 0);

    lineCountSpinBox->setValue(source_settings.format_settings.caption_line_count);
    insertLinebreaksCheckBox->setChecked(source_settings.format_settings.caption_insert_newlines);
    addPunctuationCheckBox->setChecked(source_settings.format_settings.caption_insert_punctuation);
    combobox_set_data_int(*capitalizationComboBox, source_settings.format_settings.capitalization, 0);

    enabledCheckBox->setChecked(current_settings.enabled);
    on_enabledCheckBox_stateChanged(0);
    update_combobox_output_target(*outputTargetComboBox,
                                  source_settings.streaming_output_enabled,
                                  source_settings.recording_output_enabled,
                                  0);

    this->captionTimeoutEnabledCheckBox->setChecked(source_settings.format_settings.caption_timeout_enabled);
    this->captionTimeoutDoubleSpinBox->setValue(source_settings.format_settings.caption_timeout_seconds);

    setupReplacementTableWidget(this->wordReplacementTableWidget, source_settings.format_settings.replacer.user_replacements());

    saveTranscriptsCheckBox->setChecked(source_settings.transcript_settings.enabled);
    on_saveTranscriptsCheckBox_stateChanged(0);

    this->transcriptsEnabledStreamCheckBox->setChecked(source_settings.transcript_settings.streaming_transcripts_enabled);
    this->transcriptsEnabledRecordingCheckBox->setChecked(source_settings.transcript_settings.recording_transcripts_enabled);
    this->transcriptsEnabledVirtualcamCheckBox->setChecked(source_settings.transcript_settings.virtualcam_transcripts_enabled);

    transcriptFolderPathLineEdit->setText(QString::fromStdString(source_settings.transcript_settings.output_path));
    combobox_set_data_str(*transcriptFormatComboBox, source_settings.transcript_settings.format.c_str(), 0);
    transcript_format_index_change(0);
    srtDurationSpinBox->setValue(source_settings.transcript_settings.srt_target_duration_secs);
    srtLineLengthSpinBox->setValue(source_settings.transcript_settings.srt_target_line_length);
    srtAddPunctuationCheckBox->setChecked(source_settings.transcript_settings.srt_add_punctuation);
    srtSplitSentencesCheckBox->setChecked(source_settings.transcript_settings.srt_split_single_sentences);
    combobox_set_data_int(*srtCapitalizationComboBox, source_settings.transcript_settings.srt_capitalization, 0);

    combobox_set_data_str(*recordingTranscriptFilenameComboBox, source_settings.transcript_settings.recording_filename_type.c_str(), 0);
    combobox_set_data_str(*recordingTranscriptCustomNameExistsCombobox,
                          source_settings.transcript_settings.recording_filename_exists.c_str(), 0);
    recordingTranscriptCustomNameOverwriteLineEdit->setText(
            QString::fromStdString(source_settings.transcript_settings.recording_filename_custom));

    combobox_set_data_str(*streamingTranscriptFilenameComboBox, source_settings.transcript_settings.streaming_filename_type.c_str(), 0);
    combobox_set_data_str(*streamingTranscriptCustomNameExistsCombobox,
                          source_settings.transcript_settings.streaming_filename_exists.c_str(), 0);
    streamingTranscriptCustomNameOverwriteLineEdit->setText(
            QString::fromStdString(source_settings.transcript_settings.streaming_filename_custom));

    combobox_set_data_str(*virtualcamTranscriptFilenameComboBox, source_settings.transcript_settings.virtualcam_filename_type.c_str(), 0);
    combobox_set_data_str(*virtualcamTranscriptCustomNameExistsCombobox,
                          source_settings.transcript_settings.virtualcam_filename_exists.c_str(), 0);
    virtualcamTranscriptCustomNameOverwriteLineEdit->setText(
            QString::fromStdString(source_settings.transcript_settings.virtualcam_filename_custom));

    recording_name_index_change(0);
    streaming_name_index_change(0);
    virtualcam_name_index_change(0);
}

void CaptionSettingsWidget::set_settings(const CaptionSettings &new_settings) {
    spdlog::debug("CaptionSettingsWidget set_settings");
    current_settings = new_settings;
    scene_collection_name = current_scene_collection_name();
    updateUi();
}

void CaptionSettingsWidget::on_replacementWordsAddWordPushButton_clicked() {
    const int row = wordReplacementTableWidget->rowCount();
    wordReplacementTableWidget->setRowCount(row + 1);
    wordReplacementTableWidget->setCellWidget(row, WORD_REPLACEMENT_TYPE_INDEX,
                                              wordReplacementTypeComboBox("text_case_insensitive"));
    wordReplacementTableWidget->setCellWidget(row, WORD_REPLACEMENT_DELETE_INDEX,
                                              wordReplacementDeletePushButton(wordReplacementTableWidget));
}

void CaptionSettingsWidget::on_transcriptFolderPickerPushButton_clicked() {
    QString dir_path = QFileDialog::getExistingDirectory(this,
                                                         tr("Open Directory"),
                                                         transcriptFolderPathLineEdit->text(),
                                                         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir_path.length()) {
        transcriptFolderPathLineEdit->setText(dir_path);
    }
}

void CaptionSettingsWidget::on_saveTranscriptsCheckBox_stateChanged(int new_state) {
    const bool disabled = !saveTranscriptsCheckBox->isChecked();
    this->transcripWidget1->setDisabled(disabled);
    this->transcripWidget2->setDisabled(disabled);
    this->transcripWidget3->setDisabled(disabled);
    this->transcripWidget4->setDisabled(disabled);
    this->transcripWidget5->setDisabled(disabled);
    this->transcriptSrtSettingsWidget->setDisabled(disabled);
}

void CaptionSettingsWidget::on_enabledCheckBox_stateChanged(int new_state) {
//    const bool disabled = !enabledCheckBox->isChecked();
//    this->tabWidget->setDisabled(disabled);
}

}