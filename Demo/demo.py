import asyncio
import websockets
import time

async def demo():
	async with websockets.connect('ws://makahiya.rfc1149.net/ws/clients/0') as websocket0:
		while(True):
			msg = await websocket0.recv()
			print (msg)
			if msg == "touch 1 1":
				print("Callback")
				async with websockets.connect('ws://makahiya.rfc1149.net/ws/clients/1') as websocket1:
					await websocket1.send("set 33 12")
					await websocket1.send("set 11 0")
					await websocket1.send("set 10 0")
					await websocket1.send("set 50 1")
					time.sleep(1)
					await websocket1.send("set 50 0")

asyncio.get_event_loop().run_until_complete(demo())

