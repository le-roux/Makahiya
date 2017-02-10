import asyncio
import websockets
import time
import twitter
import random
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

async def flash(ws, ledId, length, r, g, b, w = 0):
	await setColor(ws, ledId, r, g, b, w)
	await asyncio.sleep(length)
	await turnOff(ws, ledId)

async def flashAll(ws, length):
	colour = random.randint(0, 7)
	if colour == 0:
		colour = 7
	r = 255 * int(colour / 4)
	g = 255 * (int(colour / 2) % 2)
	b = 255 * (colour % 2)
	if colour == 7:
		led0 = asyncio.ensure_future(flash(ws, 0, length, 0, 0, 0, 255))
	else:
		led0 = asyncio.ensure_future(flash(ws, 0, length, r, g, b))
	led1 = asyncio.ensure_future(flash(ws, 1, length, r, g, b))
	led2 = asyncio.ensure_future(flash(ws, 2, length, r, g, b))
	led3 = asyncio.ensure_future(flash(ws, 3, length, r, g, b))
	led4 = asyncio.ensure_future(flash(ws, 4, length, r, g, b))
	led5 = asyncio.ensure_future(flash(ws, 5, length, r, g, b))
	await asyncio.wait([led0, led1, led2, led3, led4, led5], return_when=asyncio.ALL_COMPLETED)

async def fadeIn(ws, ledId, h, length):
	await turnOff(ws, ledId)
	for i in range (1, 21):
		await asyncio.sleep(length / 20)
		await setHSV(ws, ledId, h, 1, i / 20)

async def fadeOut(ws, ledId, h, length):
	for i in range (0, 21):
		await asyncio.sleep(length / 20)
		await setHSV(ws, ledId, h, 1, (20 - i) / 20)

async def fade(ws, ledId, h, lengthChange, lengthStay):
	await fadeIn(ws, ledId, h, lengthChange)
	await asyncio.sleep(lengthStay)
	await fadeOut(ws, ledId, h, lengthChange)

async def fadeAllOut(ws, h, lengthChange):
	led1 = asyncio.ensure_future(fadeOut(ws, 1, h, lengthChange))
	led2 = asyncio.ensure_future(fadeOut(ws, 2, h, lengthChange))
	led3 = asyncio.ensure_future(fadeOut(ws, 3, h, lengthChange))
	led4 = asyncio.ensure_future(fadeOut(ws, 4, h, lengthChange))
	led5 = asyncio.ensure_future(fadeOut(ws, 5, h, lengthChange))
	await asyncio.wait([led1, led2, led3, led4, led5], return_when=asyncio.ALL_COMPLETED)
	
async def wheel(ws, ledId, start, v, length):
	h = start
	duration = 0
	while(duration < length):
		await setHSV(ws, ledId, h, 1, v)
		duration += 0.2
		await asyncio.sleep(0.2)
		h = (h + 6) % 360

async def reverseWheel(ws, ledId, start, v, length):
	h = start
	duration = 0
	while(duration < length):
		await setHSV(ws, ledId, h, 1, v)
		duration += 0.2
		await asyncio.sleep(0.2)
		h = (h - 6) % 360

async def globalWheel(ws, length):
	led1 = asyncio.ensure_future(wheel(ws, 1, 0, 1, length))
	led2 = asyncio.ensure_future(wheel(ws, 2, 72, 1, length))
	led3 = asyncio.ensure_future(wheel(ws, 3, 144, 1, length))
	led4 = asyncio.ensure_future(wheel(ws, 4, 216, 1, length))
	led5 = asyncio.ensure_future(wheel(ws, 5, 288, 1, length))
	await asyncio.wait([led1, led2, led3, led4, led5], return_when=asyncio.ALL_COMPLETED)
	
async def reverseGlobalWheel(ws, length):
	led1 = asyncio.ensure_future(reverseWheel(ws, 1, 0, 1, length))
	led2 = asyncio.ensure_future(reverseWheel(ws, 2, 72, 1, length))
	led3 = asyncio.ensure_future(reverseWheel(ws, 3, 144, 1, length))
	led4 = asyncio.ensure_future(reverseWheel(ws, 4, 216, 1, length))
	led5 = asyncio.ensure_future(reverseWheel(ws, 5, 288, 1, length))
	await asyncio.wait([led1, led2, led3, led4, led5], return_when=asyncio.ALL_COMPLETED)

async def blueWheel(ws):
	v = 1
	for i in range(0, 9):
		led1 = asyncio.ensure_future(wheel(ws, 1, 170, v, 2))
		led2 = asyncio.ensure_future(wheel(ws, 2, 170, v, 2))
		led3 = asyncio.ensure_future(wheel(ws, 3, 170, v, 2))
		led4 = asyncio.ensure_future(wheel(ws, 4, 170, v, 2))
		led5 = asyncio.ensure_future(wheel(ws, 5, 170, v, 2))
		await asyncio.wait([led1, led2, led3, led4, led5], return_when=asyncio.ALL_COMPLETED)
		v -= 0.05
		led1 = asyncio.ensure_future(reverseWheel(ws, 1, 230, v, 2))
		led2 = asyncio.ensure_future(reverseWheel(ws, 2, 230, v, 2))
		led3 = asyncio.ensure_future(reverseWheel(ws, 3, 230, v, 2))
		led4 = asyncio.ensure_future(reverseWheel(ws, 4, 230, v, 2))
		led5 = asyncio.ensure_future(reverseWheel(ws, 5, 230, v, 2))
		await asyncio.wait([led1, led2, led3, led4, led5], return_when=asyncio.ALL_COMPLETED)
		v -= 0.05
	await allOff(ws)
	
async def setServo(ws, servoId, pos):
	await ws.send("set " + str(constants.SERVOS[servoId]) + " " + str(pos))

async def shakeServo(ws, servoId):
	await setServo(ws, servoId, 2800)
	await asyncio.sleep(1)
	await setServo(ws, servoId, 800)

async def randomPart(ws1, ws2):
	for i in range(0, 23):
		for j in range(1, 6):
			h = random.randint(0, 359)
			await setHSV(ws1, j, h, 1, 1)
		for j in range(1, 6):
			h = random.randint(0, 359)
			await setHSV(ws2, j, h, 1, 1)
		n = random.randint(0, 9)
		if n < 5:
			await shakeServo(ws1, n)
		else:
			await shakeServo(ws2, n - 5)

async def finalPart(ws1, ws2):
	await allOff(ws1)
	await allOff(ws2)
	for i in range(0, 14):
		await flashAll(ws1, 0.2)
		await flashAll(ws2, 0.2)
	fade1 = asyncio.ensure_future(fadeAllOut(ws1, 0, 2.3))
	fade2 = asyncio.ensure_future(fadeAllOut(ws2, 0, 2.3))
	await asyncio.wait([fade1, fade2], return_when=asyncio.ALL_COMPLETED)
	for i in range(0, 14):
		flash1 = asyncio.ensure_future(flashAll(ws1, 0.2))
		flash2 = asyncio.ensure_future(flashAll(ws2, 0.2))
		await asyncio.wait([flash1, flash2], return_when=asyncio.ALL_COMPLETED)
		await asyncio.sleep(0.2)

async def tweets(ws0, ws1):
	while(True):
		msg = await ws1.recv()
		if msg == "touch 0 0":
			print("Uploading music from smartphone via Bluetooth. [Sylvain] #LaFleur")
			break
	while(True):
		msg = await ws1.recv()
		if msg == "touch 0 1":
			print("Crevé, dodo ! [Sylvain] #LaFleur")
			break
	while(True):
		msg = await ws0.recv()
		if msg == "touch 1 0":
			print("Chill #LaFleur")
			break
	while(True):
		msg = await ws0.recv()
		if msg == "touch 1 1":
			print("Volume down. #LaFleur")
			break
	while(True):
		msg = await ws0.recv()
		if msg == "touch 1 2":
			print("Volume up. #LaFleur")
			break
	while(True):
		msg = await ws0.recv()
		if msg == "touch 1 2":
			print("Volume up. #LaFleur")
			break
	while(True):
		msg = await ws1.recv()
		if msg == "touch 0 2":
			print("23h30, on se calme [Sylvain]. #LaFleur")
			break
	while(True):
		msg = await ws0.recv()
		if msg == "touch 1 3":
			print("Depressing coloc, time to party [Tanguy]. #LaFleur")
			break
	while(True):
		msg = await ws1.recv()
		if msg == "touch 0 3":
			print("Off. #LaFleur")
			break
	while(True):
		msg = await ws1.recv()
		if msg == "touch 0 0":
			print("Programmation réveil fleur Tanguy : 6h30. Musique “Rick Roll.mp3”. #LaFleur")
			break
	
			#		api.PostUpdate("Je suis touché(e) !")

async def leds(ws0, ws1):
	await allOff(ws0)
	await allOff(ws1)
	start = time.time()
	await blueWheel(ws1)
	await asyncio.sleep(2)
	print ("Tanguy : time to stop working! #LaFleur")
	await asyncio.sleep(1.4)
	await randomPart(ws0, ws1)
	wheel1 = asyncio.ensure_future(globalWheel(ws0, 11.3))
	wheel2 = asyncio.ensure_future(reverseGlobalWheel(ws1, 11.3))
	await asyncio.wait([wheel1, wheel2], return_when=asyncio.ALL_COMPLETED)
	await finalPart(ws0, ws1)
	await allOff(ws0)
	await allOff(ws1)

async def demo():
	api = twitter.Api(consumer_key='5AxCp1lqGjdQ123yDmAm6JBcw',
			  consumer_secret='1UTqusvXqMC0fJeQYCFLQbVVb1DiUyjE2p4Ru3FBd6Msa7N11D',
			  access_token_key='829427262965612553-Nvq67UVsuQad4fJVhJMYxwVM0kEdfil',
			  access_token_secret='mjvACBrmXt3I6H65YC7hV56Ja4pZaRnM8K7fYTfH3DVWK')
	async with websockets.connect('ws://makahiya.rfc1149.net/ws/clients/2883619') as ws0:
		async with websockets.connect('ws://makahiya.rfc1149.net/ws/clients/3342371') as ws1:
			task1 = asyncio.ensure_future(tweets(ws0, ws1))
			task2 = asyncio.ensure_future(leds(ws0, ws1))
			#TODO Wait for setup
			await ws1.send('/music/3342371/file.mp3')
			#TODO Wait for sync
			await asyncio.wait([task1, task2], return_when=asyncio.ALL_COMPLETED)


asyncio.get_event_loop().run_until_complete(demo())
