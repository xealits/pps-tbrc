#!/usr/bin/env python3

import asyncio
import websockets

@asyncio.coroutine
def hello(websocket, path):
    print("starting server")
    while websocket.open:
        data = yield from websocket.recv()
        message = str(data).strip()
        # print(message, type(message))
        print("< {}".format(message))

        if "ADD_CLIENT:1" in message:
            reply = "SET_CLIENT_ID:9"
        else:
            reply = "Hello {}!".format(message)

        if not websocket.open:
            break
        yield from websocket.send(reply)
        print("> {}".format(reply))

start_server = websockets.serve(hello, 'localhost', 1987)

print("a;ldkf")

asyncio.get_event_loop().run_until_complete(start_server)
print("a;ldkf")
asyncio.get_event_loop().run_forever()
print("a;ldkf")
