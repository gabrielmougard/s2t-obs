#include "inference_stream.h"

#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <thread>

// TODO : define random_session_pair in util/backend_utils.h


namespace backend {

static void audio_sender_thread(std::shared_ptr<InferenceStream> self);

static void _audio_sender(InferenceStream &self);

static void read_results_loop_thread(
    InferenceStream &self,
    grpc::ClientReaderWriterInterface<StreamingRecognizeRequest, StreamingRecognizeResponse> *streamer
);

InferenceStream::InferenceStream(const InferenceStreamSettings settings) : settings(settings), session_pair(random_session_pair()) {
    spdlog::debug("InferenceStream GRPC Speech, created session pair: %s", session_pair.c_str());
}

bool InferenceStream::start(std::shared_ptr<InferenceStream> self) {
    // Requires the InferenceStream to have been made as shared_pointer and passed to itself to start.
    // Kept by each thread to ensure the object isn't deconstructed before all threads are done.

    if (self.get() != this)
        return false;

    if (started)
        return false;

    started = true;

    std::thread upstream_thread(audio_sender_thread, self);
    upstream_thread.detach();
    return true;
}

static void audio_sender_thread(std::shared_ptr<InferenceStream> self) {
    spdlog::debug("starting audio_sender_thread() thread");
    if (!self) {
        self->stop();
        spdlog::debug("audio_sender_thread() no self");
        return;
    }
    _audio_sender(*self);
    self->stop();
    spdlog::debug("finished audio_sender_thread() thread");
}

static bool write_config(
    grpc::ClientReaderWriterInterface<StreamingRecognizeRequest, StreamingRecognizeResponse> *streamer,
    const InferenceStreamSettings &settings
) {
    StreamingRecognizeRequest request;
    auto *streaming_config = request.mutable_streaming_config();
    streaming_config->set_interim_results(true);
    auto *recognition_config = streaming_config->mutable_config();

    recognition_config->set_encoding(RecognitionConfig_AudioEncoding::RecognitionConfig_AudioEncoding_LINEAR16);
    recognition_config->set_sample_rate_hertz(16000);
    recognition_config->set_language_code(settings.language);

    return streamer->Write(request);
}

static void write_audio_loop(
    grpc::ClientReaderWriterInterface<StreamingRecognizeRequest, StreamingRecognizeResponse> *streamer,
    InferenceStream &self
) {
    uint chunk_count = 0;
    StreamingRecognizeRequest request;

    while (!self.is_stopped()) {
        std::string *audio_chunk = self.dequeue_audio_data(self.settings.send_timeout_ms * 1000);
        if (audio_chunk == nullptr) {
            spdlog::debug("couldn't dequeue audio chunk in time");
            break;
        }

        if (audio_chunk->empty()) {
            spdlog::debug("got 0 size audio chunk. ignored");
            delete audio_chunk;
            continue;
        }

        request.set_audio_content(*audio_chunk);

        if (!streamer->Write(request)) {
            spdlog::debug("write_audio_loop write failed, stopping.");
            delete audio_chunk;
            break;
        }

        if (chunk_count % 20 == 0)
            spdlog::debug("sent audio chunk {}, {} bytes", chunk_count, audio_chunk->size());

        delete audio_chunk;
        chunk_count++;
    }
}

static void read_results_loop_thread(
    InferenceStream &self,
    grpc::ClientReaderWriterInterface<StreamingRecognizeRequest, StreamingRecognizeResponse> *streamer
) {
    spdlog::debug("read_results_loop_thread starting");
    StreamingRecognizeResponse response;
    std::chrono::steady_clock::time_point first_received_at;

    while(streamer->Read(&response)) {
        if (self.is_stopped())
            break;

        spdlog::info("Result size: {}", response.result_size());
        auto now = std::chrono::stead_clock::now();
        auto alternative = result.alternatives(0);
        RawResult raw_result(0, true, 1.0, alternative.transcript(), "", first_received_at, now);
        {
            std::lock_guard<std::recursive_mutex> lock(self.on_caption_cb_handle.mutex);
            if (self.on_caption_cb_handle.callback_fn) {
                self.on_caption_cb_handle.callback_fn(raw_result);
            }
        }
    }
    spdlog::debug("read_results_loop_thread stopped");
    self.stop();
}



}