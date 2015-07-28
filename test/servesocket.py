#!/usr/bin/env python3

import asyncio
import websockets

@asyncio.coroutine
def hello(websocket, path):
    while True:
        name = yield from websocket.recv()
        print("< {}".format(name))
        greeting = "Hello {}!".format(name)
        if not websocket.open:
        	break
        yield from websocket.send(greeting)
        print("> {}".format(greeting))

start_server = websockets.serve(hello, 'localhost', 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()

