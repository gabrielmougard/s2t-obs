# Speech2Text plugin for OBS studio (find a better name later)

The goal is to generate subtitles in different languages taht could be displayed as captions in OBS and then streamed to the twitch viewers that can pick the language.

## Build

* clone repo with `--recursive` option
* cd into the repo and try `git submodule update --recursive --merge --init` in the `obs-studio` as there are nested submodules
* Then do `make build` (you might want to grab a coffee if this is the first build and also depending on your machine)


## Description of plugin

The plugin contains the following:

* A loadable `.so/.dylib/.dll` file that will need to be placed in the OBS studio libraries folder. OBS will automatically load it at runtime. This shared library will at least contain two things:
    * The user interface to manage the plugin (a couple of widgets).
    * The data processing pipeline. It will take care of handling the new OBS source (splitting the audio stream into chunks and retrieving the translated text output). It will also be possible to apply filtering rules (some words could be displayed differently)

* But then, as described above, you can already realise that a key feature is missing: **The speech2text model itself**. However, we know that we can execute inference on both a CPU and a GPU, the latter being way faster. However, one might not have a very powerful hardware locally but have a remote machine that suits the inference workload better (e.g: a powerful GPU/CPU server in the cloud). **That is why I think we should offer two options to the end user:**
    * (1) Being able to run the inference workload locally (Own CPU or GPU)
    * (2) Being able to run the inference workload remotly (distant server or even local server)

* In the case of option (1), the inference workload is part of the same `.so/.dylib/.dll` file and run either on the system CPU or GPU (configurable via the `settings` widget). We'll use an ONNX model which might be quantized to better fit the installed hardware and thus provide greater performance. We will then need to use the C API of Nvidia Triton with a C++ ONNX runtime/ TensorRT C++ SDK. We'll also need to reimplement the `encode` and `decode` of the `Wav2Vec2Processor` class, part of huggingface's Transformer python library (Pheew)

* In the case of option (2), things are easier. We still need to generate the ONNX model but we can use the hugging face Transformer ecosystem for the pre and post-processing with a Nvidia Triton python server (with an ONNX runtime backend). The only drawback is that we need to use gRPC communications between the client (runtime in the `.so/.dylib/.dll` file) and this server which will be embedded in a docker container.


## Resources 


* https://learnopencv.com/how-to-run-inference-using-tensorrt-c-api/ (How to run inference using TensorRT C++ API)
* https://learnopencv.com/how-to-convert-a-model-from-pytorch-to-tensorrt-and-speed-up-inference/ (How to convert a model from pytorch to TensorRT and speed up inference)
