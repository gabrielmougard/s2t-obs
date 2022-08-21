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

#include <iostream>
#include <moodycamel/blockingconcurrentqueue.h>

#include <spdlog/spdlog.h>
#include <obs-frontend-api.h>

#include "ui/caption_main_widget.h"
#include "ui/caption_manager.h"

ui::CaptionMainWidget *caption_widget = nullptr;
ui::CaptionManager *caption_manager = nullptr;

bool frontend_finished_loading = false;
bool saving = false;

static void obs_event(enum obs_frontend_event ev, void *) {
    if (ev == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
        ev_finished_loading();
    } else if (ev == OBS_FRONTEND_EVENT_STREAMING_STARTED) {
        ev_streaming_started();
    } else if (ev == OBS_FRONTEND_EVENT_STREAMING_STOPPED) {
        ev_streaming_stopped();
    } else if (ev == OBS_FRONTEND_EVENT_RECORDING_STARTED) {
        ev_recording_started();
    } else if (ev == OBS_FRONTEND_EVENT_RECORDING_STOPPED) {
        ev_recording_stopped();
    } else if (ev == OBS_FRONTEND_EVENT_EXIT) {
        ev_exit();
    }
}

void ev_finished_loading() {
    frontend_finished_loading = true;
    if (caption_widget)
        caption_widget->show();
}

void ev_streaming_started() {
    if (caption_widget)
        caption_widget->streaming_started_ev();
}

void ev_streaming_stopped() {
    if (caption_widget)
        caption_widget->streaming_stopped_ev();
}

void ev_recording_started() {
    if (caption_widget)
        caption_widget->recording_started_ev();
}

void ev_recording_stopped() {
    if (caption_widget)
        caption_widget->recording_stopped_ev();
}

void ev_exit() {
    spdlog::info("frontend exit, closing caption...");
    
    if (caption_widget) {
        delete caption_widget;
        caption_widget = nullptr;
    }

    if (caption_manager) {
        delete caption_manager;
        caption_manager = nullptr;
    }
    spdlog::info("frontend exit, caption closed.");
} 

static void save_and_load_event_callback(obs_data_t *save_data, bool saving, void *) {
    if (saving && caption_manager) {
        save_caption_manager(save_data, caption_manager->get_settings());
    }

    if (!saving) { // load the plugin at the opening of OBS
        auto settings = load_caption_manager(save_data);
        if (caption_manager && caption_widget) {
            caption_manager->update_settings(settings);
        } else if (caption_manager || caption_widget) {
            error_log("only one of caption_manager and caption_widget is alive. Fatal error.");
        } else {
            caption_manager = CaptionManger(settings);
            caption_widget = CaptionWidget(*caption_manager);
            load_UI();
        }
    }
}

bool obs_module_load(void) {
    spdlog::info("obs-speech2text-plugin unloaded.");
    obs_frontend_add_event_callback(obs_event, nullptr);
    obs_frontend_add_save_callback(save_and_load_event_callback, nullptr);
    return true;
}

bool obs_module_unload(void) {
    spdlog::info("obs-speech2text-plugin unloaded.");
}