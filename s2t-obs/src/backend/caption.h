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

#ifndef OBS_SPEECH2TEXT_PLUGIN_CAPTION_H
#define OBS_SPEECH2TEXT_PLUGIN_CAPTION_H

#include "inference_stream.h"
#include "audio_capture_pipeline.h"
#include "audio_converter_pipeline.h"
#include "post_caption_handler.h"

#include <QObject>
#include <QTimer>

namespace backend {

typedef unsigned int uint;

Q_DECLARE_METATYPE(std::string)

Q_DECLARE_METATYPE(shared_ptr<OutputCaptionResult>)

Q_DECLARE_METATYPE(CaptionResult)

#define MAX_HISTORY_VIEW_LENGTH 1000
#define HISTORY_ENTRIES_LOW_WM 40
#define HISTORY_ENTRIES_HIGH_WM 80

enum CaptionSourceMuteType {
    CAPTION_SOURCE_MUTE_TYPE_FROM_OWN_SOURCE,
    CAPTION_SOURCE_MUTE_TYPE_ALWAYS_CAPTION,
    CAPTION_SOURCE_MUTE_TYPE_USE_OTHER_MUTE_SOURCE,
};

struct CaptionSourceSettings {
    std::string caption_source_name;
    CaptionSourceMuteType mute_when;
    std::string mute_source_name;

    bool operator==(const CaptionSourceSettings &rhs) const {
        return caption_source_name == rhs.caption_source_name &&
               mute_when == rhs.mute_when &&
               mute_source_name == rhs.mute_source_name;
    }

    bool operator!=(const CaptionSourceSettings &rhs) const {
        return !(rhs == *this);
    }

    std::string active_mute_source_name() const {
        if (mute_when == CAPTION_SOURCE_MUTE_TYPE_USE_OTHER_MUTE_SOURCE)
            return mute_source_name;

        return caption_source_name;
    }

    void print(const char *line_prefix = "") {
        printf("%sCaptionSourceSettings\n", line_prefix);
        printf("%s  caption_source_name: %s\n", line_prefix, caption_source_name.c_str());
        printf("%s  mute_when: %d\n", line_prefix, mute_when);
        printf("%s  mute_source_name: %s\n", line_prefix, mute_source_name.c_str());
    }
};

struct TextOutputSettings {
    bool enabled;
    std::string text_source_name;
    uint line_length;
    uint line_count;
    bool insert_punctuation;
    CapitalizationType capitalization;

    void print(const char *line_prefix = "") const {
        printf("%sTextOutputSettings\n", line_prefix);
        printf("%s  enabled: %d\n", line_prefix, enabled);
        printf("%s  text_source_name: %s\n", line_prefix, text_source_name.c_str());
        printf("%s  line_length: %d\n", line_prefix, line_length);
        printf("%s  line_count: %d\n", line_prefix, line_count);
        printf("%s  insert_punctuation: %d\n", line_prefix, insert_punctuation);
        printf("%s  capitalization: %d\n", line_prefix, capitalization);
    }

    bool isValid() const {
        return !text_source_name.empty() && line_count && line_length;
    }

    bool isValidEnabled() const {
        return enabled && isValid();
    }

    bool operator==(const TextOutputSettings &rhs) const {
        return enabled == rhs.enabled &&
               text_source_name == rhs.text_source_name &&
               line_length == rhs.line_length &&
               line_count == rhs.line_count &&
               insert_punctuation == rhs.insert_punctuation &&
               capitalization == rhs.capitalization;
    }

    bool operator!=(const TextOutputSettings &rhs) const {
        return !(rhs == *this);
    }
};

struct SceneCollectionSettings {
    CaptionSourceSettings caption_source_settings;
    std::vector<TextOutputSettings> text_outputs;

    bool operator==(const SceneCollectionSettings &rhs) const {
        return caption_source_settings == rhs.caption_source_settings &&
               text_outputs == rhs.text_outputs;
    }

    bool operator!=(const SceneCollectionSettings &rhs) const {
        return !(rhs == *this);
    }
};

struct TranscriptOutputSettings {
    bool enabled;
    std::string output_path;
    std::string format;

    std::string recording_filename_type;
    std::string recording_filename_custom;
    std::string recording_filename_exists;

    std::string streaming_filename_type;
    std::string streaming_filename_custom;
    std::string streaming_filename_exists;

    std::string virtualcam_filename_type;
    std::string virtualcam_filename_custom;
    std::string virtualcam_filename_exists;

    uint srt_target_duration_secs;
    uint srt_target_line_length;
    bool srt_add_punctuation;
    bool srt_split_single_sentences;
    CapitalizationType srt_capitalization;

    bool streaming_transcripts_enabled;
    bool recording_transcripts_enabled;
    bool virtualcam_transcripts_enabled;


    TranscriptOutputSettings(bool enabled, const std::string &outputPath, const std::string &format,
                             const std::string &recordingFilenameType,
                             const std::string &recordingFilenameCustom, const std::string &recordingFilenameExists,
                             const std::string &streamingFilenameType, const std::string &streamingFilenameCustom,
                             const std::string &streamingFilenameExists, const std::string &virtualcamFilenameType,
                             const std::string &virtualcamFilenameCustom, const std::string &virtualcamFilenameExists,
                             uint srtTargetDurationSecs, uint srtTargetLineLength,
                             bool srtAddPunctuation, bool srtSplitSingleSentences,
                             CapitalizationType srtCapitalization,
                             bool streamingTranscriptsEnabled, bool recordingTranscriptsEnabled,
                             bool virtualcamTranscriptsEnabled)
            : enabled(enabled), output_path(outputPath), format(format),
              recording_filename_type(recordingFilenameType),
              recording_filename_custom(recordingFilenameCustom),
              recording_filename_exists(recordingFilenameExists),
              streaming_filename_type(streamingFilenameType),
              streaming_filename_custom(streamingFilenameCustom),
              streaming_filename_exists(streamingFilenameExists),
              virtualcam_filename_type(virtualcamFilenameType),
              virtualcam_filename_custom(virtualcamFilenameCustom),
              virtualcam_filename_exists(virtualcamFilenameExists),
              srt_target_duration_secs(srtTargetDurationSecs),
              srt_target_line_length(srtTargetLineLength),
              srt_add_punctuation(srtAddPunctuation),
              srt_split_single_sentences(srtSplitSingleSentences),
              srt_capitalization(srtCapitalization),
              streaming_transcripts_enabled(streamingTranscriptsEnabled),
              recording_transcripts_enabled(recordingTranscriptsEnabled),
              virtualcam_transcripts_enabled(virtualcamTranscriptsEnabled) {}


    bool operator==(const TranscriptOutputSettings &rhs) const {
        return enabled == rhs.enabled &&
               output_path == rhs.output_path &&
               format == rhs.format &&
               recording_filename_type == rhs.recording_filename_type &&
               recording_filename_custom == rhs.recording_filename_custom &&
               recording_filename_exists == rhs.recording_filename_exists &&
               streaming_filename_type == rhs.streaming_filename_type &&
               streaming_filename_custom == rhs.streaming_filename_custom &&
               streaming_filename_exists == rhs.streaming_filename_exists &&
               virtualcam_filename_type == rhs.virtualcam_filename_type &&
               virtualcam_filename_custom == rhs.virtualcam_filename_custom &&
               virtualcam_filename_exists == rhs.virtualcam_filename_exists &&
               srt_target_duration_secs == rhs.srt_target_duration_secs &&
               srt_target_line_length == rhs.srt_target_line_length &&
               srt_add_punctuation == rhs.srt_add_punctuation &&
               srt_split_single_sentences == rhs.srt_split_single_sentences &&
               srt_capitalization == rhs.srt_capitalization &&
               streaming_transcripts_enabled == rhs.streaming_transcripts_enabled &&
               recording_transcripts_enabled == rhs.recording_transcripts_enabled &&
               virtualcam_transcripts_enabled == rhs.virtualcam_transcripts_enabled;
    }

    bool operator!=(const TranscriptOutputSettings &rhs) const {
        return !(rhs == *this);
    }

    bool hasBaseSettings() const;

    void print(const char *line_prefix = "") {
        printf("%sTranscriptSettings\n", line_prefix);
        printf("%s  enabled: %d\n", line_prefix, enabled);
        printf("%s  output_path: %s\n", line_prefix, output_path.c_str());
        printf("%s  format: %s\n", line_prefix, format.c_str());

        printf("%s  recording_filename_type: %s\n", line_prefix, recording_filename_type.c_str());
        printf("%s  recording_filename_custom: %s\n", line_prefix, recording_filename_custom.c_str());
        printf("%s  recording_filename_exists: %s\n", line_prefix, recording_filename_exists.c_str());

        printf("%s  streaming_filename_type: %s\n", line_prefix, streaming_filename_type.c_str());
        printf("%s  streaming_filename_custom: %s\n", line_prefix, streaming_filename_custom.c_str());
        printf("%s  streaming_filename_exists: %s\n", line_prefix, streaming_filename_exists.c_str());

        printf("%s  virtualcam_filename_type: %s\n", line_prefix, virtualcam_filename_type.c_str());
        printf("%s  virtualcam_filename_custom: %s\n", line_prefix, virtualcam_filename_custom.c_str());
        printf("%s  virtualcam_filename_exists: %s\n", line_prefix, virtualcam_filename_exists.c_str());

        printf("%s  srt_target_duration_secs: %d\n", line_prefix, srt_target_duration_secs);
        printf("%s  srt_target_line_length: %d\n", line_prefix, srt_target_line_length);
        printf("%s  srt_add_punctuation: %d\n", line_prefix, srt_add_punctuation);
        printf("%s  srt_split_single_sentences: %d\n", line_prefix, srt_split_single_sentences);
        printf("%s  srt_capitalization: %d\n", line_prefix, srt_capitalization);
        printf("%s  streaming_transcripts_enabled: %d\n", line_prefix, streaming_transcripts_enabled);
        printf("%s  recording_transcripts_enabled: %d\n", line_prefix, recording_transcripts_enabled);
        printf("%s  virtualcam_transcripts_enabled: %d\n", line_prefix, virtualcam_transcripts_enabled);
    }
};

struct SourceCaptionerSettings {
    bool streaming_output_enabled;
    bool recording_output_enabled;

    TranscriptOutputSettings transcript_settings;

//    std::map<string, SceneCollectionSettings> scene_collection_settings_map;
    SceneCollectionSettings scene_collection_settings;

    CaptionFormatSettings format_settings;
    ContinuousCaptionStreamSettings stream_settings;

    SourceCaptionerSettings();

    SourceCaptionerSettings(
            bool streaming_output_enabled,
            bool recording_output_enabled,

            const TranscriptOutputSettings &transcript_settings,
            const SceneCollectionSettings &scene_collection_settings,
            const CaptionFormatSettings &format_settings,
            const ContinuousCaptionStreamSettings &stream_settings
    ) :
            streaming_output_enabled(streaming_output_enabled),
            recording_output_enabled(recording_output_enabled),

            transcript_settings(transcript_settings),
            scene_collection_settings(scene_collection_settings),
            format_settings(format_settings),
            stream_settings(stream_settings) {}

    bool operator==(const SourceCaptionerSettings &rhs) const {
        return streaming_output_enabled == rhs.streaming_output_enabled &&
               recording_output_enabled == rhs.recording_output_enabled &&

               transcript_settings == rhs.transcript_settings &&
               scene_collection_settings == rhs.scene_collection_settings &&
               format_settings == rhs.format_settings &&
               stream_settings == rhs.stream_settings;
//               scene_collection_settings_map == rhs.scene_collection_settings_map &&
    }


    bool operator!=(const SourceCaptionerSettings &rhs) const {
        return !(rhs == *this);
    }

    void print(const char *line_prefix = "") {
        printf("%sSourceCaptionerSettings\n", line_prefix);
        printf("%s  streaming_output_enabled: %d\n", line_prefix, streaming_output_enabled);
        printf("%s  recording_output_enabled: %d\n", line_prefix, recording_output_enabled);
        printf("%s  Scene Collection Settings:\n", line_prefix);
        scene_collection_settings.caption_source_settings.print((string(line_prefix) + "    ").c_str());
        printf("%s  Text Output Settings (%lu):\n", line_prefix, scene_collection_settings.text_outputs.size());

        int cnt = 0;
        for (const auto &i: scene_collection_settings.text_outputs) {
            printf("%s     #%d:\n", line_prefix, cnt++);
            i.print((string(line_prefix) + "      ").c_str());
        }

        stream_settings.print((string(line_prefix) + "  ").c_str());
        format_settings.print((string(line_prefix) + "  ").c_str());
        transcript_settings.print((string(line_prefix) + "  ").c_str());
    }

    const SceneCollectionSettings &get_scene_collection_settings(const string &scene_collection_name) const {
        return scene_collection_settings;
    }

    void update_setting(const string &scene_collection_name, const SceneCollectionSettings &new_settings) {
        scene_collection_settings = new_settings;
    }

};

struct CaptionOutput {
    std::shared_ptr<OutputCaptionResult> output_result;
    bool is_clearance;

    CaptionOutput(std::shared_ptr<OutputCaptionResult> output_result, bool is_clearance) :
            output_result(output_result),
            is_clearance(is_clearance) {};

    CaptionOutput() : is_clearance(false) {}
};

template<typename T>
struct CaptionOutputControl {
    moodycamel::BlockingConcurrentQueue<CaptionOutput> caption_queue;
    volatile bool stop = false;
    T arg;

    CaptionOutputControl(T arg) : arg(arg) {}

    void stop_soon();

    ~CaptionOutputControl();
};

template<typename T>
struct OutputWriter {
    std::recursive_mutex control_change_mutex;
    std::shared_ptr<CaptionOutputControl<T>> control;

    void clear() {
        std::lock_guard<recursive_mutex> lock(control_change_mutex);
        if (control) {
            control->stop_soon();
            control = nullptr;
        }
    }

    void set_control(const std::shared_ptr<CaptionOutputControl<T>> &set_control) {
        std::lock_guard<recursive_mutex> lock(control_change_mutex);
        clear();
        this->control = set_control;
    }

    bool enqueue(const CaptionOutput &output) {
        std::lock_guard<recursive_mutex> lock(control_change_mutex);
        if (control && !control->stop) {
            control->caption_queue.enqueue(output);
            return true;
        }
        return false;
    }
};

enum SourceCaptionerStatusEvent {
    SOURCE_CAPTIONER_STATUS_EVENT_STOPPED,

    SOURCE_CAPTIONER_STATUS_EVENT_STARTED_OK,
    SOURCE_CAPTIONER_STATUS_EVENT_STARTED_ERROR,

    SOURCE_CAPTIONER_STATUS_EVENT_NEW_SETTINGS_STOPPED,

    SOURCE_CAPTIONER_STATUS_EVENT_AUDIO_CAPTURE_STATUS_CHANGE,
};

struct SourceCaptionerStatus {
    // TODO: add starting error msg string
    SourceCaptionerStatusEvent event_type;

    bool settings_changed;
    bool stream_settings_changed;
    SourceCaptionerSettings settings;
    string scene_collection_name;

    audio_source_capture_status audio_capture_status;

    bool active;

    SourceCaptionerStatus(SourceCaptionerStatusEvent eventType, bool settingsChanged, bool streamSettingsChanged,
                          const SourceCaptionerSettings &settings, string scene_collection_name,
                          audio_source_capture_status audioCaptureStatus, bool active)
            : event_type(eventType), settings_changed(settingsChanged), stream_settings_changed(streamSettingsChanged), settings(settings),
              scene_collection_name(scene_collection_name), audio_capture_status(audioCaptureStatus), active(active) {}
};

static void caption_output_writer_loop(std::shared_ptr<CaptionOutputControl<int>> control, bool to_stream) {
    std::string to_what(to_stream ? "streaming" : "recording");
    spdlog::info("caption_output_writer_loop {} starting", to_what.c_str());

    std::string previous_line;
    CaptionOutput caption_output;
    int active_delay_sec;
    bool got_item;
    obs_output_t *output = nullptr;

    double waited_left_secs = 0;
    while (!control->stop) {
        control->caption_queue.wait_dequeue(caption_output);
        if (control->stop)
            break;
        if (!caption_output.output_result) {
            spdlog::info("got empty CaptionOutput.output_result???");
            continue;
        }
        if (output) {
            obs_output_release(output);
            output = nullptr;
        }
        if (to_stream)
            output = obs_frontend_get_streaming_output();
        else
            output = obs_frontend_get_recording_output();

        if (!output) {
            spdlog::info("built caption lines, no output 1, not sending, not {}?: '{}'", to_what.c_str(),
                     caption_output.output_result->output_line.c_str());
            continue;
        }
        if (!caption_output.is_clearance && caption_output.output_result->output_line.empty()) {
            spdlog::log("ingoring empty non clearance, {}", to_what.c_str());
            continue;
        }
        if (caption_output.output_result->output_line == previous_line) {
            continue;
        }

        previous_line = caption_output.output_result->output_line;
        waited_left_secs = 0;
        active_delay_sec = obs_output_get_active_delay(output);
        if (active_delay_sec) {
            auto since_creation = chrono::steady_clock::now() - caption_output.output_result->caption_result.received_at;
            chrono::seconds wanted_delay(active_delay_sec);
            auto wait_left = wanted_delay - since_creation;
            if (wait_left > wanted_delay) {
                spdlog::info("capping delay, wtf, negative duration?, got {}, max {}",
                    std::chrono::duration_cast<std::chrono::duration<double >>(wait_left).count(),
                    std::chrono::duration_cast<std::chrono::duration<double >>(wanted_delay).count()
                );
                wait_left = wanted_delay;
            }

            waited_left_secs = std::chrono::duration_cast<std::chrono::duration<double >>(wait_left).count();
            spdlog::debug("caption_output_writer_loop {} sleeping for {} seconds", to_what.c_str(), waited_left_secs);
    
            // release output before possible long thread sleep to not block a possible shutdown (or output change?)
            obs_output_release(output);
            output = nullptr;
            std::this_thread::sleep_for(wait_left);
            if (control->stop)
                break;

            if (to_stream)
                output = obs_frontend_get_streaming_output();
            else
                output = obs_frontend_get_recording_output();

            if (!output) {
                spdlog::info("built caption lines, no output 2, not sending, not {}?: '{}'", to_what.c_str(), caption_output.output_result->output_line.c_str());
                continue;
            }
        }

        spdlog::debug("sending caption {} line now, waited {}: '{}'", to_what.c_str(), waited_left_secs, caption_output.output_result->output_line.c_str());
        obs_output_output_caption_text2(output, caption_output.output_result->output_line.c_str(), 0.0);
    }
    if (output) {
        obs_output_release(output);
        output = nullptr;
    }
    spdlog::info("caption_output_writer_loop {} done", to_what.c_str());
}

Q_DECLARE_METATYPE(std::shared_ptr<SourceCaptionerStatus>)

class SourceCaptioner : public QObject {
Q_OBJECT

    bool base_enabled;
    std::unique_ptr<SourceAudioCaptureSession> source_audio_capture_session;
    std::unique_ptr<OutputAudioCaptureSession> output_audio_capture_session;
    std::unique_ptr<ContinuousCaptions> continuous_captions;
    uint audio_chunk_count = 0;

    SourceCaptionerSettings settings;
    string selected_scene_collection_name;

    std::unique_ptr<CaptionResultHandler> caption_result_handler;
    std::recursive_mutex settings_change_mutex;

    std::chrono::steady_clock::time_point last_caption_at;
    bool last_caption_cleared;
    QTimer timer;

    std::vector<std::shared_ptr<OutputCaptionResult>> results_history; // final ones + last ones before interruptions
    std::shared_ptr<OutputCaptionResult> held_nonfinal_caption_result;

    OutputWriter<int> streaming_output;
    OutputWriter<int> recording_output;

    OutputWriter<TranscriptOutputSettings> transcript_streaming_output;
    OutputWriter<TranscriptOutputSettings> transcript_recording_output;
    OutputWriter<TranscriptOutputSettings> transcript_virtualcam_output;

    std::tuple<string, string> last_text_source_set;

    int audio_capture_id = 0;

    string last_caption_text;
    bool last_caption_final = false;

    void caption_was_output();

    void output_caption_writers(
            const CaptionOutput &output,
            bool to_stream,
            bool to_recoding,
            bool to_transcript_streaming,
            bool to_transcript_recording,
            bool to_transcript_virtualcam,
            bool is_clearance
    );

    void store_result(shared_ptr<OutputCaptionResult> output_result);

    void prepare_recent(string &recent_captions_output);

    void on_audio_data_callback(const int id, const uint8_t *data, const size_t size);

    void on_audio_capture_status_change_callback(const int id, const audio_source_capture_status status);

    void on_caption_text_callback(const CaptionResult &caption_result, bool interrupted);

    bool _start_caption_stream(bool restart_stream);

    void process_caption_result(const CaptionResult, bool interrupted);

    void process_audio_capture_status_change(const int id, const int new_status);

    void set_text_source_text(const string &text_source_name, const string &caption_text);

private slots:

    void clear_output_timer_cb();

//    void send_caption_text(const string text, int send_in_secs);

signals:

    void received_caption_result(const CaptionResult, bool interrupted);

    void caption_result_received(
            shared_ptr<OutputCaptionResult> caption,
            bool cleared,
            string recent_caption_text);

    void audio_capture_status_changed(const int id, const int new_status);

    void source_capture_status_changed(shared_ptr<SourceCaptionerStatus> status);

public:

    SourceCaptioner(const bool enabled, const SourceCaptionerSettings &settings, const string &scene_collection_name, bool start);

    ~SourceCaptioner();

    bool set_settings(const SourceCaptionerSettings &new_settings, const string &scene_collection_name);

    bool start_caption_stream(const SourceCaptionerSettings &new_settings, const string &scene_collection_name);

    void stop_caption_stream(bool send_signal = true);


    void stream_started_event();
    void stream_stopped_event();
    void recording_started_event();
    void recording_stopped_event();
    void virtualcam_started_event();
    void virtualcam_stopped_event();

    void set_enabled(const bool enabled) {
        base_enabled = enabled;
    }

};

}

#endif //OBS_SPEECH2TEXT_PLUGIN_CAPTION_H


