<!--
 Copyright 2022 gab
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
     http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
-->

# Inference

* Use Nvidia Triton server for inference
* With ONNX format (if possible)
* Generate proto for python
* Impl of gRPC server --> Triton server --> Triton server loop (with support huggingface/Transformers) --> send output back to client

## Resources

* https://towardsdatascience.com/hugging-face-transformer-inference-under-1-millisecond-latency-e1be0057a51c  (inference under 1ms with Triton server)
* https://github.com/googleapis/googleapis/blob/master/google/cloud/speech/v1/cloud_speech.proto (proto definition example of google speech API)