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

#ifndef OBS_SPEECH2TEXT_PLUGIN_WORD_H
#define OBS_SPEECH2TEXT_PLUGIN_WORD_H

#include <string>
#include <vector>
#include <regex>

#include <QString>

namespace utils {

class Filter {
private:
    std::string type;
    std::string from;
    std::string to;

public:
    Filter(const std::string &type, const std::string &from, const std::string &to) :
            type(type), from(from), to(to) {}

    bool operator==(const Filter &rhs) const {
        return type == rhs.type &&
               from == rhs.from &&
               to == rhs.to;
    }

    bool operator!=(const Filter &rhs) const {
        return !(rhs == *this);
    }

    const std::string &get_type() const {
        return type;
    }

    const std::string &get_from() const {
        return from;
    }

    const std::string &get_to() const {
        return to;
    }

    friend class Replacer;
};

class Rep {
private:
    bool use_text;
    std::regex reg;
    std::string reg_to;
    QString text_from;
    QString text_to;
    bool text_case_sensitive;
public:
    Rep(const regex &reg, const std::string &to) : use_text(false), reg(reg), reg_to(to) {}
    Rep(const std::string &from, const std::string &to, bool case_sensitive) :
        use_text(true), text_from(QString::fromStdString(from)), text_to(QString::fromStdString(to)),
        text_case_sensitive(case_sensitive) {}

    std::string replace(const std::string &input) const {
        if (use_text) {
            if (text_from.isEmpty())
                return input;

            if (text_case_sensitive)
                return QString::fromStdString(input).replace(text_from, text_to, Qt::CaseSensitive).toStdString();
            else
                return QString::fromStdString(input).replace(text_from, text_to, Qt::CaseInsensitive).toStdString();
        } else {
            return std::regex_replace(input, reg, reg_to);
        }
    }
};

class Replacer {
    std::vector<Rep> regs;

public:
    Replacer(const std::vector<Filter> &replacements, bool ignore_invalid) {
        set_replacements(replacements, ignore_invalid);
    }

    bool has_replacements() const {
        return !regs.empty();
    }

    std::string replace(const std::string &input) const {
        std::string res(input);
        for (const auto &rep: regs) {
            res = rep.replace(res);
        }

        return res;
    }

private:
    void set_replacements(const std::vector<Filter> &reps, bool ignore_invalid) {
        addReps(reps, ignore_invalid);
    }

    void addReps(const std::vector<Filter> &reps, bool ignore_invalid) {
        for (Filter rep: reps) {
            if (rep.from.empty())
                continue;

            try {
                if (rep.type == "text_case_sensitive") {
                    regs.push_back(Rep(rep.from, rep.to, true));
                } else if (rep.type == "text_case_insensitive") {
                    regs.push_back(Rep(rep.from, rep.to, false));
                } else if (rep.type == "regex_case_sensitive") {
                    regs.push_back(Rep(regex(rep.from), rep.to));
                } else if (rep.type == "regex_case_insensitive") {
                    regs.push_back(Rep(regex(rep.from, regex::icase), rep.to));
                } else {
                    throw std::string("invalid replacement type: " + rep.type);
                }
            }
            catch (...) {
                if (!ignore_invalid)
                    throw;
            }
        }
    }

    void addRepsRegex(const std::vector<Filter> &reps, bool ignore_invalid) {
        std::regex escape_reg(R"([/|[\]{}()\\^$*+?.])");

        for (Filter rep: reps) {
            if (rep.from.empty())
                continue;

            try {
                if (rep.type == "text_case_sensitive") {
                    std::string escaped = std::regex_replace(rep.from, escape_reg, "\\$0");
                    regs.push_back(Rep(regex(escaped), rep.to));
                } else if (rep.type == "text_case_insensitive") {
                    std::string escaped = std::regex_replace(rep.from, escape_reg, "\\$0");
                    regs.push_back(Rep(regex(escaped, regex::icase), rep.to));
                } else if (rep.type == "regex_case_sensitive") {
                    regs.push_back(Rep(regex(rep.from), rep.to));
                } else if (rep.type == "regex_case_insensitive") {
                    regs.push_back(Rep(regex(rep.from, regex::icase), rep.to));
                } else {
                    throw std::string("invalid replacement type: " + rep.type);
                }
            }
            catch (...) {
                if (!ignore_invalid)
                    throw;
            }
        }
    }
};

static std::vector<Filter> wordRepsFromStrs(const std::string &type, const std::vector<std::string> &strings) {
    auto words = std::vector<Filter>();

    for (auto from: strings) {
        if (from.empty())
            continue;

        words.push_back(Filter(type, from, ""));
    }
    return words;
}


static std::vector<Filter> combineWordReps(
        const std::vector<Filter> &manualReplacements,
        const std::vector<Filter> &defaultReplacements) {
    auto reps = std::vector<Filter>();

    for (const auto &i: defaultReplacements)
        reps.push_back(i);

    for (const auto &i: manualReplacements)
        reps.push_back(i);

    return reps;
}

}

#endif //OBS_SPEECH2TEXT_PLUGIN_WORD_H