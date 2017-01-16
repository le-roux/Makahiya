from pyramid.response import Response, FileResponse
from pyramid.httpexceptions import HTTPBadRequest, HTTPFound
from pyramid.view import view_config
from pyramid.renderers import get_renderer
from pyramid.interfaces import IBeforeRender
from pyramid.events import subscriber
from pyramid.security import remember, forget
from aiopyramid.config import CoroutineMapper
import pyramid
from .models import Session, Leds, Users, Timers, get_user_plant_id, get_user_level
from velruse import login_url
import logging
import colander
import deform.widget
from .websockets import plants, send_to_socket
import asyncio
import datetime
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
		SQLSession.add(Timers(plant_id=plant_id, activated=False, sound=0, light=0))
		for i in range(0,6):
			SQLsession.add(Leds(R=0, G=0, B=0, W=0, plant_id=plant_id, led_id=i))
		SQLsession.commit()
		request.session['status'] = 0
		headers = remember(request, email)
		return HTTPFound('/' + str(plant_id) + '/board', headers=headers)
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

		if request.method == 'POST':
			log.debug('POST: ' + str(request.POST))

			if (plants.registered(plant_id) or 1):
				# Modification on the powerful led
				if 'ledH_R' in request.POST:
					ledHP = SQLsession.query(Leds).filter_by(plant_id=plant_id, led_id=0).one()
					try:
						ledHP.R = int(request.POST.getone('ledH_R'))
					except ValueError as e:
						pass
					try:
						ledHP.G = int(request.POST.getone('ledH_G'))
					except ValueError as e:
						pass
					try:
						ledHP.B = int(request.POST.getone('ledH_B'))
					except ValueError as e:
						pass
					try:
						ledHP.W = int(request.POST.getone('ledH_W'))
					except ValueError as e:
						pass
					SQLsession.commit()
					msg = 'led 0 ' + str(ledHP.R) + ' ' + str(ledHP.G) + ' ' + str(ledHP.B) + ' ' + str(ledHP.W)
					await send_to_socket(plants, plant_id, msg)

				# Modification on a medium led
				for i in range(1,6):
					if 'ledM' + str(i) + 'R' in request.POST:
						led = SQLsession.query(Leds).filter_by(plant_id=plant_id, led_id=i).one()
						try:
							led.R = int(request.POST.getone('ledM'+str(i)+'R'))
						except ValueError as e:
							pass
						try:
							led.G = int(request.POST.getone('ledM'+str(i)+'G'))
						except ValueError as e:
							pass
						try:
							led.B = int(request.POST.getone('ledM'+str(i)+'B'))
							log.debug('led.b: ' + str(led.B))
						except ValueError as e:
							pass
						SQLsession.commit()
						msg = 'led ' + str(i) + ' ' + str(led.R) + ' ' + str(led.G) + ' ' + str(led.B)
						try:
							await send_to_socket(plants, plant_id, msg)
						except KeyError as e:
							pass

				# Timer creation

				res['title'] = 'Makahiya - board (plant #' + str(plant_id) + ')'
			else:
				res['title'] = 'Makahiya - board (plant #' + str(plant_id) + ') is disconnected'

		else:
			if plants.registered(plant_id):
				res['title'] = 'Makahiya - board (plant #' + str(plant_id) + ')'
			else:
				res['title'] = 'Makahiya - board (plant #' + str(plant_id) + ') is disconnected'

		# Get the leds colors
		led = SQLsession.query(Leds).filter_by(plant_id=int(plant_id), led_id=0).one()
		res['ledHP_R'] = led.R
		res['ledHP_G'] = led.G
		res['ledHP_B'] = led.B
		res['ledHP_W'] = led.W

		# Fill an array with the values of the normal leds.
		ledM = []
		led_range = range(1, 6)
		for i in led_range:
			# Query the database for led i from table 'leds'.
			led = SQLsession.query(Leds).filter_by(plant_id=plant_id, led_id=i).one()
			values = (led.R, led.G, led.B)
			ledM.append(values)

		res['ledM'] = ledM
		res['ran'] = led_range

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
		if request.method == 'POST':
			log.debug('POST: ' + str(request.POST))
			if 'type' in request.POST:
				try:
					absolute = request.POST.getone('type') == 'Absolute'
					hours = int(request.POST.getone('Hours'))
					minutes = int(request.POST.getone('Minutes'))
					seconds = int(request.POST.getone('Seconds'))
					timer.sound = int('sound' in request.POST)
					timer.light = int('light' in request.POST)
					date = await clock(plant_id, absolute,
						hour=hours, minute=minutes, second=seconds,
						sound=timer.sound, light=timer.light)
					timer.activated = True
					timer.date = date
					SQLsession.commit()
				except ValueError as e:
					pass


		res['activated'] = timer.activated
		if timer.activated:
			cur = datetime.datetime.now()
			delta = timer.date - cur
			res['hours'] = int(delta.seconds / 3600)
			res['minutes'] = int((delta.seconds % 3600) / 60)
			res['seconds'] = delta.seconds % 60
		else:
			res['hours'] = 0
			res['minutes'] = 0
			res['seconds'] = 0
		res['sound'] = timer.sound
		res['light'] = timer.light
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
