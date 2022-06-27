from importlib.metadata import requires
from onnxruntime.quantization.quantize import quantize
from transformers import Wav2Vec2ForCTC
import torch
import argparse

def convert_to_onnx(model_path, onnx_model_name):
    print(f"Converting {model_path} to onnx...")
    model = Wav2Vec2ForCTC(model_path)
    audio_len = 250000

    x = torch.randn(1, audio_len, requires_grad=True)

    torch.onnx.export(model,                    # model being run
        x,                                      # model input (or a tuple for multiple inputs)        
        onnx_model_name,                        # where to save the model (can be a file or file-like object)
        export_params=True,                     # store the trained parameter weights inside the model file
        opset_version=11,                       # the ONNX version to export the model to
        do_constant_folding=True,               # whether to execute constant folding for optimization
        input_names=['input'],                  # the model's input names
        output_names=['output'],                # the model's output names
        dynamic_axes={'input': {1: 'audio_len'},# variable length axes
            'output': {1: 'audio_len'}})


def quantize_onnx_model(onnx_model_path, quantized_model_path):
    print("Starting quantization...")
    from onnxruntime.quantization import quantize_dynamic, QuantType

    quantize_dynamic(onnx_model_path, quantized_model_path, weight_type=QuantType.QUInt8)
    print(f"Quantized model saved to: {quantized_model_path}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--model",
        type=str,
        help="Model path that will be converted to ONNX",
    )
    parser.add_argument(
        "--quantize",
        action="store_true",
        help="Whether to quantize the model or not",
    )
    args = parser.parse_args()

    model_path = args.model
    onnx_model_name = model_path.split("/")[-1] + ".onnx"
    convert_to_onnx(model_path, onnx_model_name)
    if (args.quantize):
        quantized_onnx_model = model_path.split("/")[-1] + ".quant.onnx"
        quantize_onnx_model(onnx_model_name, quantized_onnx_model)
