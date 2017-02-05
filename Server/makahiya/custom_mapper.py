import asyncio
import inspect
import websockets
from aiopyramid.config import AsyncioMapperBase
from aiopyramid.websocket.config.gunicorn import SwitchProtocolsResponse

class CustomWebsocketMapper(AsyncioMapperBase):

    def launch_websocket_view(self, view):

        def websocket_view(context, request):

            if inspect.isclass(view):
                view_callable = view(context, request)
            else:
                view_callable = view

            @asyncio.coroutine
            def _ensure_ws_close(ws):
                yield from view_callable(ws)
                yield from ws.close()

            def switch_protocols():
                # TODO: Determine if there is a more standard way to do this
                ws_protocol = websockets.WebSocketCommonProtocol(timeout=0)
                ws_protocol.matchdict = request.matchdict
                transport = request.environ['async.writer'].transport

                http_protocol = request.environ['async.protocol']
                http_protocol.connection_lost(None)

                transport._protocol = ws_protocol
                ws_protocol.connection_made(transport)
                asyncio.async(_ensure_ws_close(ws_protocol))

            response = SwitchProtocolsResponse(
                request.environ,
                switch_protocols,
            )
            # convert iterator to avoid eof issues
            response.body = response.body

            return response

        return websocket_view

    def __call__(self, view):
        """ Accepts a view_callable class. """
        return self.launch_websocket_view(view)
