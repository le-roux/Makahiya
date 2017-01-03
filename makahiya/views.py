from pyramid.response import Response
from pyramid.response import FileResponse
from pyramid.httpexceptions import HTTPBadRequest
from pyramid.view import view_config
from .models import Session, Leds, Users
from .custom_mapper import CustomWebsocketMapper
from aiopyramid.config import CoroutineMapper
from .models import Session, Leds, Users
from velruse import login_url
from .websocket_register import WebsocketRegister

import asyncio
import concurrent
import pyramid
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

# home page
@view_config(route_name='home', renderer='makahiya:templates/home.pt')
def home(request):
	session = request.session
	if 'logged' not in session:
		session['logged'] = 0
	return {'title':'Makahiya',
			'logged':session['logged']}

# upload mp3 file
@view_config(route_name='upload', renderer='makahiya:templates/upload.pt')
def upload(request):
	if request.method == 'POST':
		sound = request.params['sound']
		open ('file.mp3', 'wb').write(sound.file.read())
		return Response('File uploaded')
	return {}

#download the mp3 file
@view_config(route_name = 'mp3')
def download(request):
	return FileResponse('/home/tanguy/makahiya/file.mp3', request=request, content_type='audio/mp3')

# led view
@view_config(route_name='led', renderer='makahiya:templates/led_view.pt')
def led_view(request):
	# Query the first row (representing the powerful led) of the table 'leds'.
	session = request.session
	led = session.query(Leds).filter_by(uid=0).one()

	# Set the values in a dictionary
	res = {}
	res['ledHP_R'] = led.R
	res['ledHP_G'] = led.G
	res['ledHP_B'] = led.B
	res['ledHP_W'] = led.W
	res['user_id'] = led.userid

	# Fill an array with the values of the normal leds.
	ledM = []
	led_range = range(1, 6)
	for i in led_range:
		# Query the database for led i from table 'leds'.
		led = session.query(Leds).filter_by(uid=i).one()
		values = (led.R, led.G, led.B)
		ledM.append(values)

	res['ledM'] = ledM
	res['ran'] = led_range
	return res

# Set LED color
@view_config(route_name='set_led', request_method='POST', mapper=CoroutineMapper)
async def set_led(request):
	session = request.session
	plant_id = request.matchdict['plant_id']
	led_id = request.matchdict['led_id']
	color = request.matchdict['color']
	value = request.matchdict['value']
	try:
		plant_id = int(plant_id)
		led_id = int(led_id)
		value = int(value)
	except ValueError:
		return HTTPBadRequest('Some number could not be casted')
	if(plant_id < 0):
		return HTTPBadRequest('Id is positive')
	if(led_id < 0 or led_id > 5):
		return HTTPBadRequest('Invalid LED ID')
	if((color != 'R' and color != 'G' and color != 'B' and color != 'W') or (color == 'W' and led_id != 0)):
		return HTTPBadRequest('Invalid color')
	if(value < 0 or value > 255):
		return HTTPBadRequest('Invalid value')
	led = session.query(Leds).filter_by(uid=led_id).one()
	if (color == 'R'):
		led.R = value
	if (color == 'G'):
		led.G = value
	if (color == 'B'):
		led.B = value
	if (color == 'W'):
		led.W = value
	session.add(led)
	session.commit()

	await send_to_socket(plants, plant_id, 'led ' + str(led_id) + ' ' + color + ' ' + str(value))

	return Response('<body>Good Request</body>')

# Send a command to a servomotor
@view_config(route_name='set_servo', request_method='POST', mapper=CoroutineMapper)
async def set_servo(request):
	plant_id = request.matchdict['plant_id']
	servo_id = request.matchdict['servo_id']
	value = request.matchdict['value']
	try:
		plant_id = int(plant_id)
		servo_id = int(servo_id)
		value = int(value)
	except ValueError:
		return HTTPBadRequest('Some number could not be casted')
	if(plant_id < 0):
		return HTTPBadRequest('Id is positive')
	if(servo_id < 0 or servo_id > 4):
		return HTTPBadRequest('Invalid servo ID')
	if(value < 0 or value > 200):
		return HTTPBadRequest('Invalid value')

	await send_to_socket(plants, plant_id, 'servo ' + str(servo_id) + ' ' + str(value))

	return Response('<body>Good Request</body>')

@view_config(route_name='login', renderer='makahiya:templates/login.pt')
def login(request):
	return {"google_login_url": login_url(request, 'google')}

@view_config(context='velruse.AuthenticationComplete',
			renderer='makahiya:templates/logged.pt')
def login_callback(request):
	session = request.session
	context = request.context
	user = session.query(Users).filter_by(email=context.profile['verifiedEmail']).first()
	session['logged'] = user.level
	viewer = not user.level
	return {'editor': user.level,
			'viewer': viewer}

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

					# Finishing the producer_task to avoid problems
					# with the condition when the task is recreated

					plants.get_var(plant_id).notify()
					plants.get_var(plant_id).release()
					await asyncio.wait([producer_task])
					await plants.get_var(plant_id).acquire()
				else:
					print ('No client listening')
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
