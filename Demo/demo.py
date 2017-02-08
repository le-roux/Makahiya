import asyncio
import websockets
import time
import twitter

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

