from transformers import MBart50Tokenizer, Speech2Text2Processor, SpeechEncoderDecoderModel, pipeline, Wav2Vec2Processor, Wav2Vec2FeatureExtractor
from datasets import load_dataset

import time
import wave
import numpy
import torch

# Read file to get buffer                                                                                               
ifile = wave.open("output.wav")
samples = ifile.getnframes()
audio = ifile.readframes(samples)

# Convert buffer to float32 using NumPy                                                                                 
audio_as_np_int16 = numpy.frombuffer(audio, dtype=numpy.int16)
audio_as_np_float32 = audio_as_np_int16.astype(numpy.float32)

# Normalise float32 array so that values are between -1.0 and +1.0                                                      
max_int16 = 2**15
arc audio_normalised = audio_as_np_float32 / max_int16

# print(audio_normalised)
# print(type(audio_normalised))
# print(len(audio_normalised))

model = SpeechEncoderDecoderModel.from_pretrained("facebook/wav2vec2-xls-r-300m-21-to-en")
processor = Wav2Vec2Processor.from_pretrained("facebook/wav2vec2-xls-r-300m-21-to-en")

# dummy dataset to test
# ds = load_dataset("patrickvonplaten/librispeech_asr_dummy", "clean", split="validation")

# print(ds[0]["audio"]["array"])
# print(type(ds[0]["audio"]["array"]))
# print(len(ds[0]["audio"]["array"]))
start = time.time()
inputs = processor(audio_normalised, sampling_rate=16000, return_tensors="pt")
generated_ids = model.generate(inputs["input_values"], attention_mask=inputs["attention_mask"])
transcription = processor.batch_decode(generated_ids)
print(f"{transcription} (inference time: {time.time() - start})")

# TODO : optimization with ONNX : https://colab.research.google.com/github/vasudevgupta7/gsoc-wav2vec2/blob/main/notebooks/wav2vec2_onnx.ipynb#scrollTo=R1IQnWhN-s_7

# other resources :

# * Streaming wav2vec2 inference : https://github.com/oliverguhr/wav2vec2-live
# * VAD (Voice activity Detection using DL) https://github.com/nicklashansen/voice-activity-detection/blob/master/vad.ipynb
# * We'll probably use the python backend for Triton inference server to pre and post process ONNX model with the Wav2Vec2 processor
# * https://github.com/triton-inference-server/python_backend
# * VAD might be implemenented on client side in cpp as it's not super complicated (not the DL one ofc)
# * Tensor outputed to the client is the unicode encoded string, decoded on client.