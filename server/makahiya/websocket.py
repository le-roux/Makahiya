import asyncio

var = asyncio.Condition()
message = None

async def led_producer(id, color, value):
	global message
	message = str(id) + ' ' + color + ' ' + str(value)
	await var.acquire()
	var.notify()
	var.release()

async def led(websocket, path):
	while True:
		await var.acquire()
		await var.wait()
		var.release()
		await websocket.send(message)
