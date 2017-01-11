from pyramid.view import view_config
from .custom_mapper import CustomWebsocketMapper
from .websocket_register import WebsocketRegister

import asyncio
import websockets

# Registering connected plants and clients and managing synchronization between websockets
plants = WebsocketRegister('Plants')
clients = WebsocketRegister('Clients')

# Coroutine to send a message from a websocket
async def send_to_socket(register, id, msg):
	await register.get_var(id).acquire()
	register.set_message(id, msg)
	register.set_new(id, 1)
	register.get_var(id).notify()
	register.get_var(id).release()

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
			    return_when=asyncio.FIRST_COMPLETED)

			await plants.get_var(plant_id).acquire()

			if listener_task in done:
				msg = listener_task.result()
				print (msg)
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

			if(plants.get_new(plant_id)):
				await ws.send(plants.get_message(plant_id))
				plants.set_new(plant_id, 0)

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
