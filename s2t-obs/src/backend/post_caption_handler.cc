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

#include "post_caption_handler.h"

#include <sstream>
#include <cctype>
#include <iostream>
#include <vector>
#include <utils.h>

#include "utils/strings.h"

namespace backend {

PostCaptionHandler::PostCaptionHandler(CaptionFormatSettings settings) : settings(settings) {}

std::shared_ptr<OutputCaptionResult> PostCaptionHandler::prepare_caption_output(
    const RawResult &caption_result,
    const bool fillup_with_previous,
    const bool insert_newlines,
    const bool punctuation,
    const uint line_length,
    const uint targeted_line_count,
    const CapitalizationType capitalization,
    const bool interrupted,
    const std::vector<std::shared_ptr<OutputCaptionResult>> &result_history
) {
    std::shared_ptr<OutputCaptionResult> output_result = std::make_shared<OutputCaptionResult>(caption_result, interrupted);

    try {
        const uint max_length = targeted_line_count * line_length;
        std::string cleaned_line = caption_result.caption_text;

        if (settings.replacer.has_replacements()) {
            try {
                std::string tmp = settings.replacer.get_replacer().replace(caption_result.caption_text);

                if (caption_result.caption_text != tmp) {
                    spdlog::info("modified string '%s' -> '%s'", caption_result.caption_text.c_str(), tmp.c_str());
                    cleaned_line = tmp;
                }
            }
            catch (exception ex) {
                spdlog::error("string replacement error %s: '%s'", ex.what(), caption_result.caption_text.c_str());
            }
            catch (...) {
                spdlog::error("string replacement error '%s'", caption_result.caption_text.c_str());
            }
        }
        utils::lstrip(cleaned_line);
        output_result->clean_caption_text = cleaned_line;


        std::vector<std::string> all_lines;
        if (fillup_with_previous) {
            std::string filled_line = cleaned_line;
            if (punctuation && capitalization == CAPITALIZATION_NORMAL && !filled_line.empty() && isascii(filled_line[0]))
                filled_line[0] = toupper(filled_line[0]);

            if (filled_line.size() < max_length && !result_history.empty()) {
                for (auto i = result_history.rbegin(); i != result_history.rend(); ++i) {
                    if (!*i)
                        break;

                    if (!(*i)->caption_result.final)
                        // had interruption here, ignore
                        break;

                    if (settings.caption_timeout_enabled) {
                        double secs_since_last = std::chrono::duration_cast<std::chrono::duration<double >>
                                (std::chrono::steady_clock::now() - (*i)->caption_result.received_at).count();

                        if (secs_since_last > settings.caption_timeout_seconds)
                            break;
                    }

                    if (punctuation)
                        filled_line.insert(0, ". ");
                    else
                        filled_line.insert(0, 1, ' ');

                    filled_line.insert(0, (*i)->clean_caption_text);

                    if (punctuation && capitalization == CAPITALIZATION_NORMAL && !filled_line.empty() && isascii(filled_line[0]))
                        filled_line[0] = toupper(filled_line[0]);

                    if (filled_line.size() >= max_length)
                        break;
                }
            }
            utils::string_capitalization(filled_line, capitalization);
            utils::split_into_lines(all_lines, filled_line, line_length);
        } else {
            utils::string_capitalization(cleaned_line, capitalization);
            utils::split_into_lines(all_lines, cleaned_line, line_length);
        }

        const uint use_lines_cnt = all_lines.size() > targeted_line_count ? targeted_line_count : all_lines.size();
        output_result->output_lines.insert(output_result->output_lines.end(), all_lines.end() - use_lines_cnt, all_lines.end());

        if (!output_result->output_lines.empty()) {
            std::string join_char = insert_newlines ? "\n" : " ";
            utils::join_strings(output_result->output_lines, join_char, output_result->output_line);
        }

        return output_result;

    } catch (std::string &ex) {
        spdlog::info("couldn't parse caption message. Error: '%s'. Messsage: '%s'", ex.c_str(), caption_result.caption_text.c_str());
        return nullptr;
    } catch (...) {
        spdlog::info("couldn't parse caption message. Messsage: '%s'", caption_result.caption_text.c_str());
        return nullptr;
    }
}

}