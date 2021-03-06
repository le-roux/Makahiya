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

async def flash(ws, ledId, r, g, b, length):
	await setColor(ws, ledId, r, g, b)
	await asyncio.sleep(length)
	await turnOff(ws, ledId)

async def fadeIn(ws, ledId, h, length):
	await turnOff(ws, ledId)
	for i in range (1, 11):
		await asyncio.sleep(length / 10)
		await setHSV(ws, ledId, h, 1, i / 10)

async def fadeOut(ws, ledId, h, length):
	for i in range (0, 11):
		await asyncio.sleep(length / 10)
		await setHSV(ws, ledId, h, 1, (10 - i) / 10)

async def fade(ws, ledId, h, lengthIn, lengthStay, lengthOut):
	await fadeIn(ws, ledId, h, lengthIn)
	await asyncio.sleep(lengthStay)
	await fadeOut(ws, ledId, h, lengthOut)
	
async def wheel(ws, ledId, start, length):
	h = start
	duration = 0
	while(duration < length):
		await setHSV(ws, ledId, h, 1, 1)
		duration += 0.2
		await asyncio.sleep(0.2)
		h = (h + 6) % 360

async def demo():
	api = twitter.Api(consumer_key='5AxCp1lqGjdQ123yDmAm6JBcw',
			  consumer_secret='1UTqusvXqMC0fJeQYCFLQbVVb1DiUyjE2p4Ru3FBd6Msa7N11D',
			  access_token_key='829427262965612553-Nvq67UVsuQad4fJVhJMYxwVM0kEdfil',
			  access_token_secret='mjvACBrmXt3I6H65YC7hV56Ja4pZaRnM8K7fYTfH3DVWK')
	async with websockets.connect('ws://makahiya.rfc1149.net/ws/clients/2883619') as ws0:
		await allOff(ws0)
		await flash(ws0, 1, 255, 255, 255, 0.2)
		red_task = asyncio.ensure_future(fade(ws0, 1, 0, 2, 1, 2))
		cyan_task = asyncio.ensure_future(fade(ws0, 2, 180, 2, 1, 2))
		await asyncio.wait([red_task, cyan_task], return_when=asyncio.FIRST_COMPLETED)
		await wheel(ws0, 1, 0, 6)
		await allOff(ws0)
		'''while(True):
			msg = await ws0.recv()
			print (msg)
			if msg == "touch 1 1":
				print("Callback")
				async with websockets.connect('ws://makahiya.rfc1149.net/ws/clients/3342371') as ws1:
					setColor(ws1, 1, 12, 0, 0)
			elif msg == "touch 1 2":
				api.PostUpdate("Je suis touché(e) !")'''

asyncio.get_event_loop().run_until_complete(demo())
