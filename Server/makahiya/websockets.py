from pyramid.view import view_config
from .custom_mapper import CustomWebsocketMapper
from .websocket_register import WebsocketRegister
from .models import Session, Leds, Servos, Touch, Music
from .constants import constants

import asyncio
import websockets
import re

# Registering connected plants and clients and managing synchronization between websockets
plants = WebsocketRegister('Plants')
clients = WebsocketRegister('Clients')

# Coroutine to send a message from a websocket
async def send_to_socket(register, identifier, msg):
	await register.get_var(identifier).acquire()
	register.set_message(identifier, register.get_message(identifier) + " " + msg)
	register.set_new(identifier, 1)
	register.get_var(identifier).notify()
	register.get_var(identifier).release()

# Coroutine that waits until send_to_socket is called
async def wait_producer(register, identifier):
	await register.get_var(identifier).acquire()
	await register.get_var(identifier).wait()
	register.get_var(identifier).release()

# Websocket between the server and a plant
@view_config(route_name='plant_ws', mapper=CustomWebsocketMapper)
async def plant(ws):
	plant_id = 0
	listener_task = None
	producer_task = None
	try:
		plant_id = ws.matchdict['plant_id']
		try:
			plant_id = int(plant_id)
		except ValueError:
			return HTTPBadRequest('id is a number')
		plants.register(plant_id)

		while True:

			# Creates a task that waits for an incoming message
			# and another one that waits for a call to send_to_socket
			# Then it waits until one of these task finishes

			listener_task = asyncio.ensure_future(ws.recv())
			producer_task = asyncio.ensure_future(wait_producer(plants, plant_id))
			done, pending = await asyncio.wait(
			    [listener_task, producer_task],
			    timeout=15,
			    return_when=asyncio.FIRST_COMPLETED)

			await plants.get_var(plant_id).acquire()

			if listener_task in done:
				msg = listener_task.result()
				command = re.split(' ', msg)
				SQLSession = Session()
				try:
					register = int(command[0])
					value = int(command[1])
					if register >= 50:
						if (command[1]):
							value = True
						else:
							value = False
					elif register == 33:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=1).one()
						led.R = value
					elif register == 10:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=1).one()
						led.G = value
					elif register == 11:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=1).one()
						led.B = value
					elif register == 50:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=1).one()
						led.on = value
					elif register == 34:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=2).one()
						led.R = value
					elif register == 6:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=2).one()
						led.G = value
					elif register == 7:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=2).one()
						led.B = value
					elif register == 51:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=2).one()
						led.on = value
					elif register == 35:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=3).one()
						led.R = value
					elif register == 29:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=3).one()
						led.G = value
					elif register == 30:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=3).one()
						led.B = value
					elif register == 52:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=3).one()
						led.on = value
					elif register == 31:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=4).one()
						led.R = value
					elif register == 0:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=4).one()
						led.G = value
					elif register == 36:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=4).one()
						led.B = value
					elif register == 53:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=4).one()
						led.on = value
					elif register == 37:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=5).one()
						led.R = value
					elif register == 38:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=5).one()
						led.G = value
					elif register == 5:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=5).one()
						led.B = value
					elif register == 54:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=5).one()
						led.on = value
					elif register == 8:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=0).one()
						led.R = value
					elif register == 9:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=0).one()
						led.G = value
					elif register == 12:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=0).one()
						led.B = value
					elif register == 13:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=0).one()
						led.W = value
					elif register == 55:
						led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=0).one()
						led.on = value
					SQLSession.commit()
				except ValueError:
					if (command[0] == 'sync'):
						ledHP = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=0).one()
						await ws.send(constants.SET + str(constants.LED_R[0]) + ' ' + str(ledHP.R))
						await ws.send(constants.SET + str(constants.LED_G[0]) + ' ' + str(ledHP.G))
						await ws.send(constants.SET + str(constants.LED_B[0]) + ' ' + str(ledHP.B))
						await ws.send(constants.SET + str(constants.LED_HP_W) + ' ' + str(ledHP.W))
						if (ledHP.on):
							await ws.send(constants.SET + str(constants.LED_ON[0]) + ' 1')
						else:
							await ws.send(constants.SET + str(constants.LED_ON[0]) + ' 0')
						for i in range(1, 6):
							led = SQLSession.query(Leds).filter_by(plant_id=plant_id, led_id=i).one()
							await ws.send(constants.SET + str(constants.LED_R[i]) + ' ' + str(led.R))
							await ws.send(constants.SET + str(constants.LED_G[i]) + ' ' + str(led.G))
							await ws.send(constants.SET + str(constants.LED_B[i]) + ' ' + str(led.B))
							if (ledHP.on):
								await ws.send(constants.SET + str(constants.LED_ON[i]) + ' 1')
							else:
								await ws.send(constants.SET + str(constants.LED_ON[i]) + ' 0')
						for i in range(0, 5):
							servo = SQLSession.query(Servos).filter_by(plant_id=plant_id, servo_id=i).one()
							await ws.send(constants.SET + str(constants.SERVOS[i]) + ' ' + str(servo.pos))
						for i in range(1, 9):
							sensor_id = 0
							channel_id = i
							if i > 4:
								sensor_id = 1
								channel_id -= 4
							touch_config = SQLSession.query(Touch).filter_by(plant_id=plant_id, leaf_id=i).first()
							await ws.send('add ' + str(sensor_id) + ' ' + str(channel_id) + ' ' + str(int(len(touch_config.commands.split()) / 2)) + ' ' + touch_config.commands)
					elif command[0] == 'start':
						music = SQLSession.query(Music).filter_by(plant_id=plant_id).first()
						music.playing = True
						SQLSession.commit()
					elif command[0] == 'stop':
						music = SQLSession.query(Music).filter_by(plant_id=plant_id).first()
						music.playing = False
						SQLSession.commit()
					elif command[0] == 'music':
						await ws.send('play /music/' + str(plant_id) + '/file.mp3')
				SQLSession.close()
				if (clients.registered(plant_id)):
					await send_to_socket(clients, plant_id, msg)

				else:
					await ws.send('No client listening')

				# Finishing the producer_task to avoid problems
				# with the condition when the task is recreated

				plants.get_var(plant_id).notify()
				plants.get_var(plant_id).release()
				await asyncio.wait([producer_task])
				await plants.get_var(plant_id).acquire()
			else:
				listener_task.cancel()
				if len(done) == 0:

				# The plant has timed out, we assume it lost the connection

					if (producer_task != None):
						plants.get_var(plant_id).notify()
						plants.get_var(plant_id).release()
						await asyncio.wait([producer_task])
					plants.unregister(plant_id)
					return

			if(plants.get_new(plant_id)):
				await ws.send(plants.get_message(plant_id))
				plants.set_new(plant_id, 0)
				plants.set_message(plant_id, "")

			plants.get_var(plant_id).release()

	except websockets.exceptions.ConnectionClosed:

		# Deleting tasks before returning

		if (not plants.get_var(plant_id).locked()):
			await plants.get_var(plant_id).acquire()
		if (listener_task != None):
			listener_task.cancel()
		if (producer_task != None):
			plants.get_var(plant_id).notify()
			plants.get_var(plant_id).release()
			await asyncio.wait([producer_task])
		plants.unregister(plant_id)

# Websocket between the server and a client
@view_config(route_name='client_ws', mapper=CustomWebsocketMapper)
async def client(ws):
	client_id = 0
	listener_task = None
	producer_task = None
	try:
		client_id = ws.matchdict['client_id']
		try:
			client_id = int(client_id)
		except ValueError:
			return HTTPBadRequest('id is a number')
		if not plants.registered(client_id):
			await ws.send('Plant with id ' + str(client_id) + ' not connected')
		else:
			clients.register(client_id)
			await send_to_socket(plants, client_id, 'Hello')
			await ws.send('Connection with id ' + str(client_id) + ' established')
			while True:

				# Creates a task that waits for an incoming message
				# and another one that waits for a call to send_to_socket
				# Then it waits until one of these task finishes

				listener_task = asyncio.ensure_future(ws.recv())
				producer_task = asyncio.ensure_future(wait_producer(clients, client_id))
				done, pending = await asyncio.wait(
				    [listener_task, producer_task],
				    return_when=asyncio.FIRST_COMPLETED)

				await clients.get_var(client_id).acquire()

				if listener_task in done:
					msg = listener_task.result()
					print (msg)
					if plants.registered(client_id):
						await send_to_socket(plants, client_id, msg)

						# Finishing the producer_task to avoid problems
						# with the condition when the task is recreated

						clients.get_var(client_id).notify()
						clients.get_var(client_id).release()
						await asyncio.wait([producer_task])
						await clients.get_var(client_id).acquire()
					else:
						await ws.send("Connection to the plant lost, exiting")
						clients.unregister(client_id)
						break
				else:
					listener_task.cancel()

				if(clients.get_new(client_id)):
					await ws.send(clients.get_message(client_id))
					clients.set_new(client_id, 0)
					clients.set_message(client_id, "")

				clients.get_var(client_id).release()

	except websockets.exceptions.ConnectionClosed:

		# Deleting tasks before returning

		if (not clients.get_var(client_id).locked()):
			await clients.get_var(client_id).acquire()
		if (listener_task != None):
			listener_task.cancel()
		if (producer_task != None):
			clients.get_var(client_id).notify()
			clients.get_var(client_id).release()
			await asyncio.wait([producer_task])
		await send_to_socket(plants, client_id, 'Goodbye')
		clients.unregister(client_id)
