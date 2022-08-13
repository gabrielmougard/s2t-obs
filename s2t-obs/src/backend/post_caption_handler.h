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

#ifndef OBS_SPEECH2TEXT_POST_CAPTION_HANDLER_H
#define OBS_SPEECH2TEXT_POST_CAPTION_HANDLER_H

#include <utility>
#include <string>

#include "inference_stream.h"
#include "utils/word.h"

namespace backend {

enum CapitalizationType {
    CAPITALIZATION_NORMAL = 0,
    CAPITALIZATION_ALL_CAPS = 1,
    CAPITALIZATION_ALL_LOWERCASE = 2,
};

class DefaultReplacer {
private:
    utils::Replacer text_replacer;
    std::vector<std::string> default_replacements;
    std::vector<utils::Filter> manual_replacements;

public:
    const std::vector<utils::Filter> &user_replacements() const {
        return manual_replacements;
    }

    bool has_replacements() const {
        return text_replacer.has_replacements();
    }

    DefaultReplacer(
        const std::vector<std::string> &default_replacements,
        const std::vector<utils::Filter> &manula_replacements
    ) :
        default_replacements(default_replacements),
        manual_replacements(manual_replacements),
        text_replacer(
            utils::Replacer(
                combineWordReps(
                    wordRepsFromStrs(
                        "regex_casse_insensitive",
                        default_replacements
                    ),
                    manual_replacements
                ),
                true
            )
        ) {}

    bool operator==(const DefaultReplacer &rhs) const {
        return manual_replacements == rhs.manual_replacements &&
            default_replacements == rhs.default_replacements;
    }

    bool operator!=(const DefaultReplacer &rhs) const {
        return !(rhs == *this);
    }

    const utils::Replacer &get_replacer() const {
        return text_replacer;
    }
};

struct CaptionFormatSettings {
    uint caption_line_length;   
    uint caption_line_count;
    CapitalizationType capitalization;
    bool caption_insert_newlines;
    bool caption_insert_punctuation;

    bool caption_timeout_enabled;
    double caption_timeout_seconds;
    DefaultReplacer replacer;

    CaptionFormatSettings(
            uint caption_line_length,
            uint caption_line_count,
            CapitalizationType capitalization,
            bool caption_insert_newlines,
            bool caption_insert_punctuation,
            const DefaultReplacer &replacer,
            bool caption_timeout_enabled,
            double caption_timeout_seconds
    ) :
            caption_line_length(caption_line_length),
            caption_line_count(caption_line_count),
            capitalization(capitalization),
            caption_insert_newlines(caption_insert_newlines),
            caption_insert_punctuation(caption_insert_punctuation),
            replacer(replacer),
            caption_timeout_enabled(caption_timeout_enabled),
            caption_timeout_seconds(caption_timeout_seconds) {}

    void print(const char *line_prefix = "") {
        printf("%sCaptionFormatSettings\n", line_prefix);
        printf("%s  caption_line_length: %d\n", line_prefix, caption_line_length);
        printf("%s  caption_line_count: %d\n", line_prefix, caption_line_count);
        printf("%s  capitalization: %d\n", line_prefix, capitalization);
        printf("%s  caption_insert_newlines: %d\n", line_prefix, caption_insert_newlines);
        printf("%s  caption_insert_punctuation: %d\n", line_prefix, caption_insert_punctuation);
        printf("%s  user_replacements: %lu\n", line_prefix, replacer.user_replacements().size());
        for (auto &word : replacer.user_replacements())
            printf(
                "%s        %s '%s' -> '%s'\n",
                line_prefix, word.get_type().c_str(), word.get_from().c_str(), word.get_to().c_str()
            );
    }

    bool operator==(const CaptionFormatSettings &rhs) const {
        return caption_line_length == rhs.caption_line_length &&
            caption_line_count == rhs.caption_line_count &&
            capitalization == rhs.capitalization &&
            caption_insert_newlines == rhs.caption_insert_newlines &&
            caption_insert_punctuation == rhs.caption_insert_punctuation &&
            replacer == rhs.replacer &&
            caption_timeout_enabled == rhs.caption_timeout_enabled &&
            caption_timeout_seconds == rhs.caption_timeout_seconds;
    }

    bool operator!=(const CaptionFormatSettings &rhs) const {
        return !(rhs == *this);
    }
};

struct OutputCaptionResult {
    RawResult caption_result;
    bool interrupted;
    std::string clean_caption_text;

    std::vector<std::string> output_lines; //optionally cleaned and filled with history
    std::string output_line; // joined output_lines

    explicit OutputCaptionResult(
        const RawResult &caption_result,
        const bool interrupted
    ) : caption_result(caption_result), interrupted(interrupted) {}
};

class PostCaptionHandler {
public:
    explicit PostCaptionHandler(CaptionFormatSettings settings);

    std::shared_ptr<OutputCaptionResult> prepare_caption_output(
        const RawResult &caption_result,
        const bool fillup_with_previous,
        const bool insert_newlines,
        const bool punctuation,
        const uint line_length,
        const uint targeted_line_count,
        const CapitalizationType capitalization,
        const bool interrupted,
        const std::vector<std::shared_ptr<OutputCaptionResult>> &result_history);
private:
    CaptionFormatSettings settings;
};

}

#endif //OBS_SPEECH2TEXT_POST_CAPTION_HANDLER_H