#ifndef OBS_SPEECH2TEXT_PLUGIN_CAPTION_HELPER_H
#define OBS_SPEECH2TEXT_PLUGIN_CAPTION_HELPER_H

#include <QComboBox>
#include <utils.h>
#include <util/util.hpp>
#include <util/platform.h>

#include "backend/settings.h.h"
#include "backend/inference_stream.h"
#include "backend/overlapping_caption.h"
#include "ui.h"
#include "storage.h"

namespace utils {

static InferenceStreamSettings default_InferenceStreamSettings() {
    uint download_start_delay_ms = 4000;
    download_start_delay_ms = 40;
    return {
        5000,
        5000,
        180'000,
        50,
        download_start_delay_ms,
        "en-US",
        0,
        ""
    };
}

static backend::OverlappingCaptionStreamSettings default_OverlappingCaptionStreamSettings() {
    return {
        280,
        2,
        10,
        default_InferenceStreamSettings()
    };
}

static DefaultReplacer makeDefaultReplacer(const std::vector<Filter> &userReplacements) {
    return DefaultReplacer({"niger", "nigger", "nigga", "niggas", "\\bfag\\b", "faggot", "chink"},
                           userReplacements);
}

static DefaultReplacer emptyDefaultReplacer() {
    return makeDefaultReplacer(vector<Filter>());
}

static CaptionFormatSettings default_CaptionFormatSettings() {
    return {
        32,
        3,
        CAPITALIZATION_NORMAL,
        false,
        false,
        emptyDefaultReplacer(),
        true,
        15.0,
    };
}

static CaptionSourceSettings default_CaptionSourceSettings() {
    return {
        "",
        CAPTION_SOURCE_MUTE_TYPE_FROM_OWN_SOURCE,
        ""
    };
}

static SceneCollectionSettings default_SceneCollectionSettings() {
    return {
            default_CaptionSourceSettings()
    };
}

static TranscriptOutputSettings default_TranscriptOutputSettings() {
    return {
            false,
            "",
            "srt",

            "recording",
            "",
            "append",

            "datetime",
            "",
            "append",

            "datetime",
            "",
            "append",

            8,
            44,
            true,
            false,
            CAPITALIZATION_NORMAL,
            true,
            true,
            true,
    };
}

static SourceCaptionerSettings default_SourceCaptionerSettings() {
    return SourceCaptionerSettings(
            true,
            false,
            default_TranscriptOutputSettings(),
            default_SceneCollectionSettings(),
            default_CaptionFormatSettings(),
            default_OverlappingCaptionStreamSettings()
    );
};

}

#endif //OBS_SPEECH2TEXT_PLUGIN_CAPTION_HELPER_H