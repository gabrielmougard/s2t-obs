#include "inference_stream.h"

#include <memory>
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

static void write_config(
    grpc::ClientReaderWriterInterface<StreamingRecognizeRequest, StreamingRecognizeResponse> *streamer,
    const InferenceStreamSettings &settings
) {
    StreamingRecognizeRequest request;
    auto *streaming_config = request.mutable_streaming_config();
}


}