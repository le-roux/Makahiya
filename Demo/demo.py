import asyncio
import websockets
import time
import twitter
from constants import constants

async def demo():
	api = twitter.Api(consumer_key='5AxCp1lqGjdQ123yDmAm6JBcw',
			  consumer_secret='1UTqusvXqMC0fJeQYCFLQbVVb1DiUyjE2p4Ru3FBd6Msa7N11D',
			  access_token_key='829427262965612553-Nvq67UVsuQad4fJVhJMYxwVM0kEdfil',
			  access_token_secret='mjvACBrmXt3I6H65YC7hV56Ja4pZaRnM8K7fYTfH3DVWK')
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
			elif msg == "touch 1 2":
				api.PostUpdate("Je suis touché(e) !")

asyncio.get_event_loop().run_until_complete(demo())

async def setColor(ws, led_id, R = 0, G = 0, B = 0, W = 0):
	msg = "set " + str(LED_R[led_id]) + " " + str(R) + " "
	msg += "set " + str(LED_G[led_id]) + " " + str(G) + " "
	msg += "set " + str(LED_B[led_id]) + " " + str(B) + " "
	if led_id == 0:
		msg += "set " + str(LED_HP_W) + " " + str(W) + " "

	msg += "set " + str(LED_ON[led_id]) + " 1"
	await ws.send(msg)

async def turnOn(ws,led_id):
	await ws.send("set " + str(LED_ON[led_id]) + " 1")

async def turnOff(ws,led_id):
	await ws.send("set " + str(LED_ON[led_id]) + " 0")

async def allOff(ws):
	msg = ""
	for i in range(6):
		msg +="set " + str(LED_ON[i]) + " 0 "
	await ws.send(msg)

async def allOffExcept(ws, led_id):
	msg = ""
	for i in range(6):
		if i != led_id:
			msg +="set " + str(LED_ON[i]) + " 0 "
	await ws.send(msg)
