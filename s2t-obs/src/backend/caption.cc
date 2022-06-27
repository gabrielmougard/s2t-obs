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

#include <memory>

#include "caption.h"
#include "transcript.h"

typedef std::tuple<std::string, std::string> TextOutputTup;

void set_text_source_text(const std::string &text_source_name, const std::string &caption_text) {
    obs_source_t *text_source = obs_get_source_by_name(text_source_name.c_str());
    if (!text_source){
        spdlog::debug("text source: {} not found, can't set caption text", text_source_name.c_str());
        return;
    }

    obs_data_t *text_settings = obs_data_create();
    obs_data_set_string(text_settings, "text", caption_text.c_str());
    obs_source_update(text_source, text_settings);
    obs_data_release(text_settings);
    obs_source_release(text_source);
}

SourceCaptioner::SourceCaptioner(const bool enabled, const SourceCaptionerSettings &settings, const std::string &scene_collection_name, bool start) :
        QObject(),
        base_enabled(enabled),
        settings(settings),
        selected_scene_collection_name(scene_collection_name),
        last_caption_at(std::chrono::steady_clock::now()),
        last_caption_cleared(true) {
    
    QObject::connect(&timer, &QTimer::timeout, this, &SourceCaptioner::clear_output_timer_cb);
    QObject::connect(this, &SourceCaptioner::received_caption_result, this, &SourceCaptioner::process_caption_result, Qt::QueuedConnection);
    QObject::connect(this, &SourceCaptioner::audio_capture_status_changed, this, &SourceCaptioner::process_audio_capture_status_change);
    timer.start(1000);

    const SceneCollectionSettings &scene_col_settings = this->settings.get_scene_collection_settings(scene_collection_name);
    spdlog::debug("SourceCaptioner, source '{}'", scene_col_settings.caption_source_settings.caption_source_name.c_str());

    if (start)
        start_caption_stream(settings, scene_collection_name);
}

void SourceCaptioner::stop_caption_stream(bool send_signal) {
    if (!send_signal) {
        std::lock_guard<recursive_mutex> lock(settings_change_mutex);
        source_audio_capture_session = nullptr;
        output_audio_capture_session = nullptr;
        caption_result_handler = nullptr;
        continuous_captions = nullptr;
        audio_capture_id++;
        return;
    }

    settings_change_mutex.lock();

    SourceCaptionerSettings cur_settings = settings;
    string cur_scene_collection_name = this->selected_scene_collection_name;

    source_audio_capture_session = nullptr;
    output_audio_capture_session = nullptr;
    caption_result_handler = nullptr;
    continuous_captions = nullptr;
    audio_capture_id++;

    settings_change_mutex.unlock();

    if (send_signal) {
        emit source_capture_status_changed(std::make_shared<SourceCaptionerStatus>(
            SOURCE_CAPTIONER_STATUS_EVENT_STOPPED,
            false,
            false,
            cur_settings,
            cur_scene_collection_name,
            AUDIO_SOURCE_NOT_STREAMED,
            false
        ));
    }
}

bool SourceCaptioner::set_settings(const SourceCaptionerSettings &new_settings, const string &scene_collection_name) {
    bool settings_equal;
    bool stream_settings_equal;
    {
        std::lock_guard<recursive_mutex> lock(settings_change_mutex);
        stop_caption_stream(false);

        settings_equal = settings == new_settings;
        stream_settings_equal = settings.stream_settings == new_settings.stream_settings;

        settings = new_settings;
        selected_scene_collection_name = scene_collection_name;
    }

    emit source_capture_status_changed(std::make_shared<SourceCaptionerStatus>(
        SOURCE_CAPTIONER_STATUS_EVENT_NEW_SETTINGS_STOPPED,
        !settings_equal,
        !stream_settings_equal,
        new_settings,
        scene_collection_name,
        AUDIO_SOURCE_NOT_STREAMED,
        false
    ));

    return true;
}

bool SourceCaptioner::start_caption_stream(const SourceCaptionerSettings &new_settings, const string &scene_collection_name) {
    bool started_ok;
    bool settings_equal;
    bool stream_settings_equal;
    audio_source_capture_status audio_cap_status = AUDIO_SOURCE_NOT_STREAMED;

    {
        std::lock_guard<recursive_mutex> lock(settings_change_mutex);
        settings_equal = settings == new_settings;
        stream_settings_equal = settings.stream_settings == new_settings.stream_settings;

        settings = new_settings;
        selected_scene_collection_name = scene_collection_name;

        source_audio_capture_session = nullptr;
        output_audio_capture_session = nullptr;
        caption_result_handler = nullptr;
        audio_capture_id++;

        started_ok = _start_caption_stream(!stream_settings_equal);

        if (!started_ok)
            stop_caption_stream(false);

        if (started_ok) {
            if (source_audio_capture_session)
                audio_cap_status = source_audio_capture_session->get_current_capture_status();
            else if (output_audio_capture_session)
                audio_cap_status = AUDIO_SOURCE_CAPTURING;
            else {
                stop_caption_stream(false);
                started_ok = false;
            }
        }
    }

    if (started_ok) {
        emit source_capture_status_changed(std::make_shared<SourceCaptionerStatus>(
            SOURCE_CAPTIONER_STATUS_EVENT_STARTED_OK,
            !settings_equal,
            !stream_settings_equal,
            new_settings,
            scene_collection_name,
            audio_cap_status,
            true
        ));
    } else {
        emit source_capture_status_changed(std::make_shared<SourceCaptionerStatus>(
            SOURCE_CAPTIONER_STATUS_EVENT_STARTED_ERROR,
            !settings_equal,
            !stream_settings_equal,
            new_settings,
            scene_collection_name,
            AUDIO_SOURCE_NOT_STREAMED,
            false
        ));
    }

    return started_ok;
}

bool SourceCaptioner::_start_caption_stream(bool restart_stream) {
    bool caption_settings_equal;
    {
        const SceneCollectionSettings &scene_col_settings = this->settings.get_scene_collection_settings(selected_scene_collection_name);
        const CaptionSourceSettings &selected_caption_source_settings = scene_col_settings.caption_source_settings;

        spdlog::debug("SourceCaptioner start_caption_stream, source '{}'", selected_caption_source_settings.caption_source_name.c_str());

        if (selected_caption_source_settings.caption_source_name.empty()) {
            spdlog::warn("SourceCaptioner start_caption_stream, empty source given.");
            return false;
        }

        const bool use_output_audio = is_all_audio_output_capture_source_data(selected_caption_source_settings.caption_source_name);

        OBSSource caption_source;
        OBSSource mute_source;

        if (!use_output_audio) {
            caption_source = obs_get_source_by_name(selected_caption_source_settings.caption_source_name.c_str());
            obs_source_release(caption_source);
            if (!caption_source) {
                spdlog::warn("SourceCaptioner start_caption_stream, no caption source with name: '{}'",
                         selected_caption_source_settings.caption_source_name.c_str());
                return false;
            }

            if (selected_caption_source_settings.mute_when == CAPTION_SOURCE_MUTE_TYPE_USE_OTHER_MUTE_SOURCE) {
                mute_source = obs_get_source_by_name(selected_caption_source_settings.mute_source_name.c_str());
                obs_source_release(mute_source);

                if (!mute_source) {
                    spdlog::warn("SourceCaptioner start_caption_stream, no mute source with name: '{}'",
                             selected_caption_source_settings.mute_source_name.c_str());
                    return false;
                }
            }
        }

        spdlog::debug("caption_settings_equal: {}, {}", caption_settings_equal, continuous_captions != nullptr);
        if (!continuous_captions || restart_stream) {
            try {
                auto caption_cb = std::bind(&SourceCaptioner::on_caption_text_callback, this, std::placeholders::_1, std::placeholders::_2);
                continuous_captions = std::make_unique<ContinuousCaptions>(settings_copy);
                continuous_captions->on_caption_cb_handle.set(caption_cb, true);
            }
            catch (...) {
                spdlog::warn("couldn't create ContinuousCaptions");
                return false;
            }
        }
        caption_result_handler = std::make_unique<CaptionResultHandler>(settings.format_settings);

        try {
            resample_info resample_to = {16000, AUDIO_FORMAT_16BIT, SPEAKERS_MONO};
            audio_chunk_data_cb audio_cb = std::bind(&SourceCaptioner::on_audio_data_callback, this,
                                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

            auto audio_status_cb = std::bind(&SourceCaptioner::on_audio_capture_status_change_callback, this,
                                             std::placeholders::_1, std::placeholders::_2);

            if (use_output_audio) {
                int track_index = all_audio_output_capture_source_track_index(selected_caption_source_settings.caption_source_name);
                if (track_index < 0)
                    track_index = 0;

                output_audio_capture_session = std::make_unique<OutputAudioCaptureSession>(
                    track_index,
                    audio_cb, audio_status_cb,
                    resample_to,
                    audio_capture_id
                );
            } else {
                source_audio_capture_session = std::make_unique<SourceAudioCaptureSession>(
                    caption_source, mute_source, audio_cb,
                    audio_status_cb,
                    resample_to,
                    MUTED_SOURCE_REPLACE_WITH_ZERO,
                    false,
                    audio_capture_id
                );
            }
        }
        catch (std::string err) {
            spdlog::warn("couldn't create AudioCaptureSession, {}", err.c_str());
            return false;
        }
        catch (...) {
            spdlog::warn("couldn't create AudioCaptureSession");
            return false;
        }

    }
    return true;
}

void SourceCaptioner::on_audio_capture_status_change_callback(const int id, const audio_source_capture_status status) {
    emit audio_capture_status_changed(id, status);
}

void SourceCaptioner::process_audio_capture_status_change(const int cb_audio_capture_id, const int new_status) {
    settings_change_mutex.lock();

    bool is_old_audio_session = cb_audio_capture_id != audio_capture_id;
    SourceCaptionerSettings cur_settings = settings;
    string cur_scene_collection_name = selected_scene_collection_name;
    bool active = continuous_captions != nullptr;

    settings_change_mutex.unlock();

    if (is_old_audio_session) {
        spdlog::debug("ignoring old audio capture status!!");
        return;
    }

    emit source_capture_status_changed(std::make_shared<SourceCaptionerStatus>(
        SOURCE_CAPTIONER_STATUS_EVENT_AUDIO_CAPTURE_STATUS_CHANGE,
        false,
        false,
        cur_settings,
        cur_scene_collection_name,
        (audio_source_capture_status) new_status,
        active
    ));
}

void SourceCaptioner::on_audio_data_callback(const int id, const uint8_t *data, const size_t size) {
    if (continuous_captions) {
        // safe without locking as continuous_captions only ever gets updated when there's no AudioCaptureSession running
        continuous_captions->queue_audio_data((char *) data, size);
    }
    audio_chunk_count++;
}

void SourceCaptioner::clear_output_timer_cb() {
    bool to_stream, to_recording, to_transcript_streaming, to_transcript_recording;
    std::vector<string> text_source_names;
    {
        std::lock_guard<recursive_mutex> lock(settings_change_mutex);
        if (!this->settings.format_settings.caption_timeout_enabled || this->last_caption_cleared)
            return;

        double secs_since_last_caption = std::chrono::duration_cast<std::chrono::duration<double >>(
                std::chrono::steady_clock::now() - this->last_caption_at).count();

        if (secs_since_last_caption <= this->settings.format_settings.caption_timeout_seconds)
            return;

        spdlog::info("last caption line was sent {} secs ago, > {}, clearing", secs_since_last_caption, this->settings.format_settings.caption_timeout_seconds);

        this->last_caption_cleared = true;
        to_stream = settings.streaming_output_enabled;
        to_recording = settings.recording_output_enabled;
        to_transcript_streaming = settings.transcript_settings.enabled && settings.transcript_settings.streaming_transcripts_enabled;
        to_transcript_recording = settings.transcript_settings.enabled && settings.transcript_settings.recording_transcripts_enabled;

        const SceneCollectionSettings &scene_col_settings = this->settings.get_scene_collection_settings(selected_scene_collection_name);
        for (const auto &text_out: scene_col_settings.text_outputs) {
            if (!text_out.isValidEnabled())
                continue;
            text_source_names.push_back(text_out.text_source_name);
        }
    }

    auto now = std::chrono::steady_clock::now();
    auto clearance = CaptionOutput(std::make_shared<OutputCaptionResult>(CaptionResult(0, false, 0, "", "", now, now), false), true);
    output_caption_writers(
        clearance,
        to_stream,
        to_recording,
        false,
        false,
        false,
        true
    );

    for (const auto &to_clear: text_source_names) {
        set_text_source_text(to_clear, " ");
    }
    emit caption_result_received(nullptr, true, "");
}

void SourceCaptioner::store_result(shared_ptr<OutputCaptionResult> output_result) {
    if (!output_result)
        return;

    if (output_result->caption_result.final) {
        results_history.push_back(output_result);
        held_nonfinal_caption_result = nullptr;
        spdlog::debug("final, adding to history: {} {}", (int) results_history.size(), output_result->clean_caption_text.c_str());
    } else {
        held_nonfinal_caption_result = output_result;
    }

    if (results_history.size() > HISTORY_ENTRIES_HIGH_WM) {
        results_history.erase(results_history.begin(), results_history.begin() + HISTORY_ENTRIES_LOW_WM);
    }
}

void SourceCaptioner::prepare_recent(string &recent_captions_output) {
    for (auto i = results_history.rbegin(); i != results_history.rend(); ++i) {
        if (!(*i))
            break;

        if (!(*i)->caption_result.final)
            break;

        if (recent_captions_output.size() + (*i)->clean_caption_text.size() >= MAX_HISTORY_VIEW_LENGTH)
            break;

        if ((*i)->clean_caption_text.empty())
            continue;

        if (recent_captions_output.empty()) {
            recent_captions_output.insert(0, 1, '.');
        } else {
            recent_captions_output.insert(0, ". ");
        }

        recent_captions_output.insert(0, (*i)->clean_caption_text);
    }

    if (held_nonfinal_caption_result) {
        if (!recent_captions_output.empty())
            recent_captions_output.push_back(' ');
        recent_captions_output.append("    >> ");
        recent_captions_output.append(held_nonfinal_caption_result->clean_caption_text);
    }
}

void SourceCaptioner::on_caption_text_callback(const CaptionResult &caption_result, bool interrupted) {
    // emit qt signal to avoid possible thread deadlock
    // this callback comes from the captioner thread, result processing needs settings_change_mutex, so does clearing captioner,
    // but that waits for the captioner callback to finish which might be waiting on the lock otherwise.

    emit received_caption_result(caption_result, interrupted);
}

void SourceCaptioner::process_caption_result(const CaptionResult caption_result, bool interrupted) {
    std::shared_ptr<OutputCaptionResult> native_output_result;
    std::string recent_caption_text;
    bool to_stream, to_recording, to_transcript_streaming, to_transcript_recording, to_transcript_virtualcam;

    if (this->last_caption_text == caption_result.caption_text && this->last_caption_final == caption_result.final) {
        return;
    }
    this->last_caption_text = caption_result.caption_text;
    this->last_caption_final = caption_result.final;

    std::vector<TextOutputTup> text_source_sets;
    {
        std::lock_guard<recursive_mutex> lock(settings_change_mutex);

        if (!caption_result_handler) {
            spdlog::warn("no caption_result_handler, shouldn't happen, there should be no AudioCaptureSession running");
            return;
        }

        native_output_result = caption_result_handler->prepare_caption_output(
            caption_result,
            true,
            settings.format_settings.caption_insert_newlines,
            settings.format_settings.caption_insert_punctuation,
            settings.format_settings.caption_line_length,
            settings.format_settings.caption_line_count,
            settings.format_settings.capitalization,
            interrupted,
            results_history
        );
        if (!native_output_result)
            return;

        const SceneCollectionSettings &scene_col_settings = this->settings.get_scene_collection_settings(selected_scene_collection_name);
        for (const auto &text_out: scene_col_settings.text_outputs) {
            if (!text_out.isValidEnabled())
                continue;

            auto text_output_result = caption_result_handler->prepare_caption_output(
                caption_result,
                true,
                true,
                text_out.insert_punctuation,
                text_out.line_length,
                text_out.line_count,
                text_out.capitalization,
                interrupted,
                results_history
            );

            text_source_sets.emplace_back(text_out.text_source_name, text_output_result->output_line);
        }

        store_result(native_output_result);
        prepare_recent(recent_caption_text);

        to_stream = settings.streaming_output_enabled;
        to_recording = settings.recording_output_enabled;
        to_transcript_streaming = settings.transcript_settings.enabled && settings.transcript_settings.streaming_transcripts_enabled;
        to_transcript_recording = settings.transcript_settings.enabled && settings.transcript_settings.recording_transcripts_enabled;
        to_transcript_virtualcam = settings.transcript_settings.enabled && settings.transcript_settings.virtualcam_transcripts_enabled;
    }

    this->output_caption_writers(
        CaptionOutput(native_output_result, false),
        to_stream,
        to_recording,
        to_transcript_streaming,
        to_transcript_recording,
        to_transcript_virtualcam,
        false
    );

    for (const auto &text_out: text_source_sets) {
        set_text_source_text(std::get<0>(text_out), std::get<1>(text_out));
    }

    emit caption_result_received(native_output_result, false, recent_caption_text);
}

void SourceCaptioner::output_caption_writers(
        const CaptionOutput &output,
        bool to_stream,
        bool to_recoding,
        bool to_transcript_streaming,
        bool to_transcript_recording,
        bool to_transcript_virtualcam,
        bool is_clearance) {

    bool sent_stream = false;
    if (to_stream) {
        sent_stream = streaming_output.enqueue(output);
    }

    bool sent_recording = false;
    if (to_recoding) {
        sent_recording = recording_output.enqueue(output);
    }

    bool sent_transcript_streaming = false;
    if (to_transcript_streaming) {
        sent_transcript_streaming = transcript_streaming_output.enqueue(output);
    }

    bool sent_transcript_recording = false;
    if (to_transcript_recording) {
        sent_transcript_recording = transcript_recording_output.enqueue(output);
    }

    bool sent_transcript_virtualcam = false;
    if (to_transcript_virtualcam) {
        sent_transcript_virtualcam = transcript_virtualcam_output.enqueue(output);
    }

    if (!is_clearance)
        caption_was_output();
}

void SourceCaptioner::caption_was_output() {
    this->last_caption_at = std::chrono::steady_clock::now();
    this->last_caption_cleared = false;
}

void SourceCaptioner::stream_started_event() {
    settings_change_mutex.lock();
    SourceCaptionerSettings cur_settings = settings;
    settings_change_mutex.unlock();

    auto control_output = std::make_shared<CaptionOutputControl<int>>(0);
    streaming_output.set_control(control_output);
    std::thread th(caption_output_writer_loop, control_output, true);
    th.detach();

    if (this->base_enabled && cur_settings.transcript_settings.enabled && cur_settings.transcript_settings.streaming_transcripts_enabled) {
        auto control_transcript = std::make_shared<CaptionOutputControl<TranscriptOutputSettings>>(cur_settings.transcript_settings);
        transcript_streaming_output.set_control(control_transcript);

        std::thread th2(transcript_writer_loop, control_transcript, "stream", cur_settings.transcript_settings);
        th2.detach();
    }
}

void SourceCaptioner::stream_stopped_event() {
    streaming_output.clear();
    transcript_streaming_output.clear();
}

void SourceCaptioner::recording_started_event() {
    settings_change_mutex.lock();
    SourceCaptionerSettings cur_settings = settings;
    settings_change_mutex.unlock();

    auto control_output = std::make_shared<CaptionOutputControl<int>>(0);
    recording_output.set_control(control_output);
    std::thread th(caption_output_writer_loop, control_output, false);
    th.detach();

    if (this->base_enabled && cur_settings.transcript_settings.enabled && cur_settings.transcript_settings.recording_transcripts_enabled) {
        auto control_transcript = std::make_shared<CaptionOutputControl<TranscriptOutputSettings>>(cur_settings.transcript_settings);
        transcript_recording_output.set_control(control_transcript);

        std::thread th2(transcript_writer_loop, control_transcript, "recording", cur_settings.transcript_settings);
        th2.detach();
    }
}

void SourceCaptioner::recording_stopped_event() {
    recording_output.clear();
    transcript_recording_output.clear();
}

void SourceCaptioner::virtualcam_started_event() {
    settings_change_mutex.lock();
    SourceCaptionerSettings cur_settings = settings;
    settings_change_mutex.unlock();

    if (this->base_enabled && cur_settings.transcript_settings.enabled && cur_settings.transcript_settings.virtualcam_transcripts_enabled) {
        auto control_transcript = std::make_shared<CaptionOutputControl<TranscriptOutputSettings>>(cur_settings.transcript_settings);
        transcript_virtualcam_output.set_control(control_transcript);

        std::thread th2(transcript_writer_loop, control_transcript, "virtualcam", cur_settings.transcript_settings);
        th2.detach();
    }
}

void SourceCaptioner::virtualcam_stopped_event() {
    transcript_virtualcam_output.clear();
}

void SourceCaptioner::set_text_source_text(const string &text_source_name, const string &caption_text) {
    if (std::get<0>(last_text_source_set) == caption_text && std::get<1>(last_text_source_set) == text_source_name) {
        return;
    }

    ::set_text_source_text(text_source_name, caption_text);
    last_text_source_set = std::tuple<string, string>(caption_text, text_source_name);
}

SourceCaptioner::~SourceCaptioner() {
    stream_stopped_event();
    recording_stopped_event();
    stop_caption_stream(false);
}

template<class T>
void CaptionOutputControl<T>::stop_soon() {
    spdlog::debug("CaptionOutputControl stop_soon()");
    stop = true;
    caption_queue.enqueue(CaptionOutput());
}

template<class T>
CaptionOutputControl<T>::~CaptionOutputControl() {
    spdlog::debug("~CaptionOutputControl");
}

bool TranscriptOutputSettings::hasBaseSettings() const {
    if (!enabled || output_path.empty() || format.empty())
        return false;

    return true;
}
