from pyramid.response import Response, FileResponse
from pyramid.httpexceptions import HTTPBadRequest, HTTPFound
from pyramid.view import view_config
from pyramid.renderers import get_renderer
from pyramid.interfaces import IBeforeRender
from pyramid.events import subscriber
from pyramid.security import remember, forget
from aiopyramid.config import CoroutineMapper
import pyramid
from .models import Session, Leds, Servos, Users, Timers, Touch, get_user_plant_id, get_user_level
from velruse import login_url
import logging
import colander
import deform.widget
from colour import Color
from .websockets import plants, send_to_socket
import asyncio
import datetime
from .constants import constants
from .timer import clock
log = logging.getLogger(__name__)

@subscriber(IBeforeRender)
def globals_factory(event):
	master = get_renderer('templates/master.pt').implementation()
	event['master'] = master

# home page
@view_config(route_name='home', renderer='makahiya:templates/home.pt')
def home(request):
	request.session['status'] = 0
	res = {'title':'Makahiya',
		'email':request.authenticated_userid if not request.authenticated_userid == None else ''}
	if len(res['email']) > 0:
		res['plant_id'] = get_user_plant_id(res['email'])
		res['level'] = get_user_level(res['email'])
	else:
		res['level'] = -1
	return res

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

@view_config(route_name='wrong_id', renderer='makahiya:templates/wrong_id.pt')
def wrong_id(request):
	return{}

# Set LED color
@view_config(route_name='set_led', request_method='POST')
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
	SQLsession = Session()
	led = SQLsession.query(Leds).filter_by(uid=led_id).one()
	if (color == 'R'):
	       led.R = value
	if (color == 'G'):
	       led.G = value
	if (color == 'B'):
	       led.B = value
	if (color == 'W'):
	       led.W = value
	SQLsession.add(led)
	SQLsession.commit()

	return Response('<body>Good Request</body>')


# Send a command to a servomotor
@view_config(route_name='set_servo', request_method='POST')
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

	return Response('<body>Good Request</body>')

# Security related views (login & logout)
@view_config(route_name='login', renderer='makahiya:templates/login.pt')
def login(request):
	return HTTPFound(login_url(request, 'google'))

@view_config(context='velruse.AuthenticationComplete')
def login_callback(request):
	email = request.context.profile['verifiedEmail']
	SQLsession = Session()
	if 'status' in request.session and request.session['status'] == 1:
		plant_id = request.session['plant_id']

		# Check that this plant id doesn't already exist
		if SQLsession.query(Leds).filter_by(plant_id=plant_id).first() is not None:
			request.session['status'] = 4
			return HTTPFound('/subscribe')
		# Check that this user doesn't already exist
		if SQLsession.query(Users).filter_by(email=email).first() is not None:
			request.session['status'] = 2
			return HTTPFound('/subscribe')

		# Create this user in the database
		SQLsession.add(Users(email=email, level=2, plant_id=plant_id))
		SQLsession.add(Timers(plant_id=plant_id, activated=False, sound=0, light=0))
		for i in range(0,6):
			SQLsession.add(Leds(R=0, G=0, B=0, W=0, on=False, plant_id=plant_id, led_id=i))
		for i in range(0,5):
			SQLsession.add(Servos(servo_id=i, pos=0, plant_id=plant_id))
		for i in range(1,9):
			SQLsession.add(Touch(plant_id=plant_id, leaf_id=i, commands=''))
		SQLsession.commit()
		request.session['status'] = 0
		headers = remember(request, email)
		return HTTPFound('/' + str(plant_id) + '/board/leds', headers=headers)
	else:
		# Check that this user in in the database
		if SQLsession.query(Users).filter_by(email=email).first() is None:
			request.session['status'] = 3
			return HTTPFound('/subscribe')
		else: # User exists
			headers = remember(request, email)
			return HTTPFound(location = "/", headers = headers)

@view_config(route_name='logout', permission='logged')
def logout(request):
	headers = forget(request)
	request.session['state'] = 0
	request.session['plant_id'] = -1
	return HTTPFound(location='/', headers = headers)

class SubscribePage(colander.MappingSchema):
	plant_id = colander.SchemaNode(colander.Int())

class BoardPage(object):
	def __init__(self, request):
		self.request = request

	@property
	def led_form(self):
		schema = SubscribePage()
		return deform.Form(schema, buttons=('Send',))

	@property
	def reqts(self):
		return self.led_form.get_widget_resources()

@view_config(route_name='board_leds', renderer='makahiya:templates/board.pt', permission='view', mapper=CoroutineMapper)
async def board_leds(request):
	plant_id = None
	email = request.authenticated_userid
	SQLsession = Session()
	user = SQLsession.query(Users).filter_by(email=email).first()
	if user is not None:
		plant_id = user.plant_id
	if plant_id is not None and plant_id == int(request.matchdict['plant_id']):
		res = {'email': email,
				'plant_id': plant_id,
				'level': get_user_level(email)}

		if plants.registered(plant_id):
			# Modification on the powerful led
			if 'ledH_R' in request.POST:
				ledHP = SQLsession.query(Leds).filter_by(plant_id=plant_id, led_id=0).one()
				try:
					R = int(request.POST.getone('ledH_R'))
					if ledHP.R != R and R >= 0 and R < 256:
						msg = constants.SET + str(constants.LED_R[0]) \
								+ ' ' + str(R)
						await send_to_socket(plants, plant_id, msg)
						ledHP.R = R
				except KeyError:
					res['KeyError'] = 1
				except ValueError:
					pass
				try:
					G = int(request.POST.getone('ledH_G'))
					if ledHP.G != G and G >= 0 and G < 256:
						msg = constants.SET + str(constants.LED_G[0]) \
								+ ' ' + str(G)
						await send_to_socket(plants, plant_id, msg)
						ledHP.G = G
				except KeyError:
					res['KeyError'] = 1
				except ValueError:
					pass
				try:
					B = int(request.POST.getone('ledH_B'))
					if ledHP.B != B and B >= 0 and B < 256:
						msg = constants.SET + str(constants.LED_B[0]) \
								+ ' ' + str(B)
						await send_to_socket(plants, plant_id, msg)
						ledHP.B = B
				except KeyError:
					res['KeyError'] = 1
				except ValueError:
					pass
				try:
					W = int(request.POST.getone('ledH_W'))
					if ledHP.W != W and W >= 0 and W < 256:
						msg = constants.SET + str(constants.LED_HP_W) + ' ' + str(W)
						await send_to_socket(plants, plant_id, msg)
						ledHP.W = W
				except KeyError:
					res['KeyError'] = 1
				except ValueError:
					pass
				try:
					on = 'ledH_state' in request.POST
					if ledHP.on != on:
						if (on):
							msg = constants.SET + str(constants.LED_ON[0]) + ' 1'
						else:
							msg = constants.SET + str(constants.LED_ON[0]) + ' 0'
						await send_to_socket(plants, plant_id, msg)
						ledHP.on = on
				except KeyError:
					res['KeyError'] = 1
				SQLsession.commit()

			# Modification on a medium led
			for i in range(1,6):
				if 'color_ledM' + str(i) in request.POST:
					led = SQLsession.query(Leds).filter_by(plant_id=plant_id, led_id=i).one()
					c = Color(request.POST.getone('color_ledM'+str(i)))
					R = round(c.red*255)
					G = round(c.green*255)
					B = round(c.blue*255)
					on = 'state_ledM' + str(i) in request.POST
					try:
						if led.R != R and R >= 0 and R < 256:
							msg = constants.SET + str(constants.LED_R[i]) \
									+ ' ' + str(R)
							await send_to_socket(plants, plant_id, msg)
							led.R = R
					except KeyError:
						res['KeyError'] = 1
					try:
						if led.G != G and G >=0 and G < 256:
							msg = constants.SET + str(constants.LED_G[i]) \
									+ ' ' + str(G)
							await send_to_socket(plants, plant_id, msg)
							led.G = G
					except KeyError:
						res['KeyError'] = 1
					try:
						if led.B != B and B >= 0 and B < 256:
							msg = constants.SET + str(constants.LED_B[i]) \
									+ ' ' + str(B)
							await send_to_socket(plants, plant_id, msg)
							led.B = B
					except KeyError:
						res['KeyError'] = 1
					try:
						if led.on != on:
							if (on):
								msg = constants.SET + str(constants.LED_ON[i]) + ' 1'
							else:
								msg = constants.SET + str(constants.LED_ON[i]) + ' 0'
							await send_to_socket(plants, plant_id, msg)
							led.on = on
					except KeyError:
						res['KeyError'] = 1
					SQLsession.commit()

			# Modification on a servo position
			for i in range(0, 5):
				if 'pos_servo' + str(i) in request.POST:
					new_pos = request.POST.getone('pos_servo' + str(i))
					servo = SQLsession.query(Servos).filter_by(plant_id=plant_id, servo_id=i).one()
					try:
						msg = constants.SET + str(constants.SERVOS[i]) + \
						' ' + str(new_pos)
						await send_to_socket(plants, plant_id, msg)
						servo.pos = new_pos
					except KeyError:
						res['KeyError'] = 1
					SQLsession.commit()

		res['connected'] = plants.registered(plant_id)

		# Get the leds colors
		led = SQLsession.query(Leds).filter_by(plant_id=int(plant_id), led_id=0).one()
		res['ledHP_R'] = led.R
		res['ledHP_G'] = led.G
		res['ledHP_B'] = led.B
		res['ledHP_W'] = led.W
		res['ledH_state'] = led.on

		# Fill an array with the values of the normal leds.
		ledM = []
		ledM_state = []
		leds_range = range(1, 6)
		for i in leds_range:
			# Query the database for led i from table 'leds'.
			led = SQLsession.query(Leds).filter_by(plant_id=plant_id, led_id=i).one()
			try:
				values = Color(rgb=(led.R/255, led.G/255, led.B/255))
			except ValueError:
				values = Color('#000')
			ledM.append(values.hex)
			ledM_state.append(led.on)

		pos = []
		servos_range = range(0, 5)
		for i in servos_range:
			servo = SQLsession.query(Servos).filter_by(plant_id=plant_id, servo_id=i).one()
			pos.append(servo.pos)

		res['ledM_state'] = ledM_state
		res['ledM'] = ledM
		res['pos'] = pos
		res['leds_range'] = leds_range
		res['servos_range'] = servos_range
		return res
	else:
		return HTTPFound('/wrong_id')

@view_config(route_name='board_timer', renderer='makahiya:templates/timer.pt', permission='view', mapper=CoroutineMapper)
async def board_timer(request):
	plant_id = None
	email = request.authenticated_userid
	SQLsession = Session()
	user = SQLsession.query(Users).filter_by(email=email).first()
	if user is not None:
		plant_id = user.plant_id
	if plant_id is not None and plant_id == int(request.matchdict['plant_id']):
		res = {'email': email,
				'plant_id': plant_id,
				'level': get_user_level(email)}
		timer = SQLsession.query(Timers).filter_by(plant_id=plant_id).first()
		if request.method == 'POST' and plants.registered(plant_id):
			log.debug('POST: ' + str(request.POST))
			if 'type' in request.POST:
				try:
					absolute = request.POST.getone('type') == 'Absolute'
					hours = int(request.POST.getone('Hours'))
					minutes = int(request.POST.getone('Minutes'))
					seconds = int(request.POST.getone('Seconds'))
					if 'sound' in request.POST:
						timer.sound = int(request.POST.getone('alarm_id'))
					else:
						timer.sound = 0

					if 'light' in request.POST:
						timer.light = int(request.POST.getone('light_id'))
					else:
						timer.light = 0

					date = await clock(plant_id, absolute,
						hour=hours, minute=minutes, second=seconds,
						sound=timer.sound, light=timer.light)
					timer.activated = True
					timer.date = date
					SQLsession.commit()
				except ValueError as e:
					pass


		if timer.activated:
			cur = datetime.datetime.now()
			delta = timer.date - cur
			res['hours'] = int(delta.seconds / 3600)
			res['minutes'] = int((delta.seconds % 3600) / 60)
			res['seconds'] = delta.seconds % 60
			if res['hours'] == 0 and res['minutes'] == 0 and res['seconds'] == 0:
				timer.activated = False
				SQLsession.commit()
		else:
			res['hours'] = 0
			res['minutes'] = 0
			res['seconds'] = 0

		res['activated'] = timer.activated
		res['sound'] = timer.sound
		res['light'] = timer.light
		res['connected'] = plants.registered(plant_id)
		res['light_config'] = constants.LIGHT_CONFIG
		return res
	else:
		return HTTPFound('/wrong_id')


@view_config(route_name='timer_deactivate', permission='view')
def timer_deactivate(request):
	plant_id = None
	email = request.authenticated_userid
	SQLsession = Session()
	user = SQLsession.query(Users).filter_by(email=email).first()
	if user is not None:
		plant_id = user.plant_id
	if plant_id is not None and plant_id == int(request.matchdict['plant_id']):
		timer = SQLsession.query(Timers).filter_by(plant_id=plant_id).first()
		timer.activated = False
		SQLsession.commit()
		return HTTPFound('/' + str(plant_id) + '/board/timer')
	else:
		return HTTPFound('/wrong_id')

@view_config(route_name='quick_timer', permission='view', mapper=CoroutineMapper)
async def quick_timer(request):
	plant_id = None
	email = request.authenticated_userid
	SQLsession = Session()
	user = SQLsession.query(Users).filter_by(email=email).first()
	if user is not None:
		plant_id = user.plant_id
	if plant_id is not None and plant_id == int(request.matchdict['plant_id']):
		if plants.registered(plant_id):
			timer = SQLsession.query(Timers).filter_by(plant_id=plant_id).first()
			timer.activated = True
			timer.sound = 2
			timer.light = 1
			date = await clock(plant_id, minute=int(request.matchdict['time']), sound=timer.sound, light=timer.light)
			timer.date = date
			SQLsession.commit()
		return HTTPFound('/' + str(plant_id) + '/board/timer')
	else:
		return HTTPFound('/wrong_id')


@view_config(route_name='subscribe', renderer='makahiya:templates/subscribe.pt')
def subscribe(request):
	if request.method == 'POST': # Access via the form
		request.session['status'] = 1
		request.session['plant_id'] = request.POST.getone('plant_id')
		try:
			plant_id = int(request.session['plant_id'])
		except ValueError:
			return HTTPBadRequest('The plant id must be a number')
		return HTTPFound(login_url(request, 'google'))

	# Access to this page via a GET request
	# session['status']:
	#		+ 0 -> nothing
	#		+ 1 -> subscribe
	#		+ 2 -> already existing user
	#		+ 3 -> non existing user
	#		+ 4 -> already existing plant_id
	if 'status' not in request.session:
		request.session['status'] = 0

	if request.authenticated_userid is None:
		email =''
	else:
		email = request.authenticated_userid

	return {'status':request.session['status'],
			'title':'Makahiya - subscribe',
			'email':email}

@view_config(route_name='users', renderer='makahiya:templates/users.pt', permission='sudo')
def users(request):
	SQLsession = Session()
	res = {}
	users = []
	for user in SQLsession.query(Users).order_by(Users.email):
		us = {'email':user.email, 'level':user.level, 'plant_id':user.plant_id}
		users.append(us)
		log.debug('user: ' + user.email)
	res['number'] = range(0, len(users))
	res['users'] = users
	res['email'] = request.authenticated_userid
	res['plant_id'] = get_user_plant_id(res['email'])
	res['level'] = get_user_level(res['email'])
	return res

@view_config(route_name='delete', permission='sudo')
def delete(request):
	email = request.matchdict['email']
	SQLsession = Session()
	user = SQLsession.query(Users).filter_by(email=email).first()
	if user is not None and user.level > 1:
		SQLsession.delete(user)
		SQLsession.commit()
	return HTTPFound('/users')

@view_config(route_name='touch_config', renderer='makahiya:templates/touch.pt', permission='view', mapper=CoroutineMapper)
async def touch_config(request):
	SQLsession = Session()
	plant_id = None
	email = request.authenticated_userid
	SQLsession = Session()
	user = SQLsession.query(Users).filter_by(email=email).first()
	if user is not None:
		plant_id = user.plant_id
	if plant_id is not None and plant_id == int(request.matchdict['plant_id']):
		res = {'email': email,
				'plant_id': plant_id,
				'level': get_user_level(email)}
		res['leaf_range'] = range(1,9)
		if request.method == 'POST':
			for i in res['leaf_range']:
				if 'Leaf#' + str(i) in request.POST:
					commands = request.POST.getone('Leaf#' + str(i))
					sensor_id = 0
					channel_id = int(i)
					if i > 4:
						sensor_id = 1
						channel_id -= 4
					commands_nb = len(commands.split())
					if commands_nb % 2 == 1:
						res['error'] = 1
					else:
						try:
							await send_to_socket(plants, plant_id, 'add ' +\
							 str(sensor_id) + ' ' + str(channel_id) + ' ' + \
							 str(commands_nb) + ' ' + commands)
							touch_config = SQLsession.query(Touch).filter_by(plant_id=plant_id, leaf_id=i).first()
							touch_config.commands = commands
							SQLsession.commit()
						except KeyError:
							log.debug("key error")
		res['connected'] = plants.registered(plant_id)
		values = []
		for i in res['leaf_range']:
			touch_config = SQLsession.query(Touch).filter_by(plant_id=plant_id, leaf_id=i).first()
			values.append(touch_config.commands)
		res['values'] = values
		return res

	else:
		return HTTPFound('/wrong_id')

@view_config(route_name='touch_config_delete', permission='view', mapper=CoroutineMapper)
async def touch_config_delete(request):
	SQLsession = Session()
	plant_id = None
	email = request.authenticated_userid
	SQLsession = Session()
	user = SQLsession.query(Users).filter_by(email=email).first()
	if user is not None:
		plant_id = user.plant_id
	if plant_id is not None and plant_id == int(request.matchdict['plant_id']):
		leaf_id = int(request.matchdict['leaf_id'])
		sensor_id = 0
		channel_id = int(leaf_id)
		if (leaf_id > 4):
			sensor_id = 1
			channel_id -= 4
		try:
			await send_to_socket(plants, plant_id, 'clear ' + str(sensor_id) + ' ' + str(channel_id))
			touch_config = SQLsession.query(Touch).filter_by(plant_id=plant_id, leaf_id=leaf_id).first()
			touch_config.commands = ''
			SQLsession.commit()
		except KeyError:
			log.debug('key error')
		return HTTPFound('/' + str(plant_id) + '/touch')
	else:
		return HTTPFound('/wrong_id')
