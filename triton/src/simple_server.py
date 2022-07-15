import grpc
from concurrent import futures
import time
from typing import Iterable

import proto.s2t_pb2_grpc as pb2_grpc
import proto.s2t_pb2 as pb2


class SpeechService(pb2_grpc.SpeechServicer):

    def __init__(self, *args, **kwargs):
        pass

    def StreamingRecognize(
        self, request_iterator: Iterable[pb2.StreamingRecognizeRequest],
        context: grpc.ServicerContext
    ) -> Iterable[pb2.StreamingRecognizeResponse]:

        for message in request_iterator:
            yield message

        return pb2.StreamingRecognizeResponse(**result)


def serve():
    server = grpc.server(futures.ThreadPoolExecutor())
    pb2_grpc.add_SpeechServicer_to_server(SpeechService(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    server.wait_for_termination()


if __name__ == '__main__':
    serve()