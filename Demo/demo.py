import asyncio
import websockets
import time
import twitter
from constants import constants

async def setColor(ws, ledId, R = 0, G = 0, B = 0, W = 0):
	msg = "set " + str(constants.LED_R[ledId]) + " " + str(R) + " "
	msg += "set " + str(constants.LED_G[ledId]) + " " + str(G) + " "
	msg += "set " + str(constants.LED_B[ledId]) + " " + str(B) + " "
	if ledId == 0:
		msg += "set " + str(constants.LED_HP_W) + " " + str(W) + " "

	msg += "set " + str(constants.LED_ON[ledId]) + " 1"
	await ws.send(msg)

async def turnOn(ws,ledId):
	await ws.send("set " + str(constants.LED_ON[ledId]) + " 1")

async def turnOff(ws,ledId):
	await ws.send("set " + str(constants.LED_ON[ledId]) + " 0")

async def allOff(ws):
	msg = ""
	for i in range(6):
		msg +="set " + str(constants.LED_ON[i]) + " 0 "
	await ws.send(msg)

async def allOffExcept(ws, ledId):
	msg = ""
	for i in range(6):
		if i != ledId:
			msg +="set " + str(constants.LED_ON[i]) + " 0 "
	await ws.send(msg)

async def setHSV(ws, ledId, h, s, v):
	t = int((h / 60)) % 6
	f = float(h) / 60 - t
	l = v * (1 - s)
	m = v * (1 - f * s)
	n = v * (1 - (1 - f) * s)
	if t == 0:
		r = v
		g = n
		b = l
	elif t == 1:
		r = m
		g = v
		b = l
	elif t == 2:
		r = l
		g = v
		b = n
	elif t == 3:
		r = l
		g = m
		b = v
	elif t == 4:
		r = n
		g = l
		b = v
	elif t == 5:
		r = v
		g = l
		b = m
	r = round(r * 255)
	g = round(g * 255)
	b = round(b * 255)
	await setColor(ws, ledId, r, g, b)

async def demo():
	api = twitter.Api(consumer_key='5AxCp1lqGjdQ123yDmAm6JBcw',
			  consumer_secret='1UTqusvXqMC0fJeQYCFLQbVVb1DiUyjE2p4Ru3FBd6Msa7N11D',
			  access_token_key='829427262965612553-Nvq67UVsuQad4fJVhJMYxwVM0kEdfil',
			  access_token_secret='mjvACBrmXt3I6H65YC7hV56Ja4pZaRnM8K7fYTfH3DVWK')
	async with websockets.connect('ws://makahiya.rfc1149.net/ws/clients/0') as websocket0:
		h = 0
		while(True):
			for i in range (1, 6):
				await setHSV(websocket0, i, (h + 72 * i) % 360, 1, 1)
			h = (h + 6) % 360
			time.sleep(0.2)
		while(True):
			msg = await websocket0.recv()
			print (msg)
			if msg == "touch 1 1":
				print("Callback")
				async with websockets.connect('ws://makahiya.rfc1149.net/ws/clients/1') as websocket1:
					setColor(websocket1, 1, 12, 0, 0)
			elif msg == "touch 1 2":
				api.PostUpdate("Je suis touché(e) !")

asyncio.get_event_loop().run_until_complete(demo())
