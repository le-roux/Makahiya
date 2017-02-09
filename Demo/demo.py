import asyncio
import websockets
import time
import twitter

def setLed(i, h, s, v):
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
	print (str(r) + ' ' + str (g) + ' ' + str(b))

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

# asyncio.get_event_loop().run_until_complete(demo())

while True:
	h = int(input())
	s = float(input())
	v = float(input())
	setLed(1, h, s, v)

