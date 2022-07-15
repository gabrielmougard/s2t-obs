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

#ifndef OBS_SPEECH2TEXT_PLUGIN_RANDOM_H
#define OBS_SPEECH2TEXT_PLUGIN_RANDOM_H

#include <cstring>
#include <cstdlib>

#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>

namespace utils {

static std::mutex wp_rnd_mut;
static std::unique_ptr<std::mt19937> wp_rnd_ins;

static void wp_rnd_init() {
    if (wp_rnd_ins)
        return;

    try {
        std::random_device dev;
        wp_rnd_ins = std::make_unique<std::mt19937>(dev());
    } catch (...) {
        printf("wp_rnd_init fail\n");
    }
}

static unsigned int wp_rnd_num() {
    if (wp_rnd_ins)
        return wp_rnd_ins->operator()();
    return rand();
}

static std::string random_string(const int count) {
    std::lock_guard<std::mutex> lock(wp_rnd_mut);
    wp_rnd_init();

    std::string output;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < count; ++i) {
        output.push_back(alphanum[wp_rnd_num() % (sizeof(alphanum) - 1)]);
    }

    return output;
}

}

#endif //OBS_SPEECH2TEXT_PLUGIN_RANDOM_H