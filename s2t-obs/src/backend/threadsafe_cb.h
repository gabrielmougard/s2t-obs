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

#ifndef OBS_SPEECH2TEXT_PLUGIN_THREADSAFE_CB_H
#define OBS_SPEECH2TEXT_PLUGIN_THREADSAFE_CB_H

#include <thread>
#include <mutex>
#include <functional>

namespace backend {

template<typename T>
class ThreadsafeCb {
public:
    T callback_fn;
    bool lock_to_clear = true;
    std::recursive_mutex mutex;

    ThreadsafeCb() {};
    ThreadsafeCb(T callback_fn) : callback_fn(callback_fn), lock_to_clear(lock_to_clear) {}

    void clear() {
        if (lock_to_clear) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            callback_fn = nullptr;
        } else {
            callback_fn = nullptr;
        }
    }

    void set(T new_callback_fn, bool lock_to_clear = true) {
        clear();
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            this->callback_fn = new_callback_fn;
            this->lock_to_clear = lock_to_clear;
        }
    }

    ~ThreadsafeCb() {
        clear();
    }
};

}

#endif // OBS_SPEECH2TEXT_PLUGIN_THREADSAFE_CB_H