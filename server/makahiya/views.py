from pyramid.response import Response
from pyramid.httpexceptions import HTTPBadRequest
from pyramid.view import view_config
from aiopyramid.websocket.config import WebsocketMapper
from .models import DBSession, Leds, Users
from velruse import login_url
from .websocket import led_producer
from . import loop

import asyncio

id = 42

# home page
@view_config(route_name='home', renderer='makahiya:templates/home.pt')
def home(request):
	session = request.session
	if 'logged' not in session:
		session['logged'] = 0
	return {'title':'Makahiya',
			'logged':session['logged']}

# led view
@view_config(route_name='led', renderer='makahiya:templates/led_view.pt')
def led_view(request):
	# Query the first row (representing the powerful led) of the table 'leds'.
	led = DBSession.query(Leds).filter_by(uid=0).one()

	# Set the values in a dictionary
	res = {}
	res['ledHP_R'] = led.R
	res['ledHP_G'] = led.G
	res['ledHP_B'] = led.B
	res['ledHP_W'] = led.W

	# Fill an array with the values of the normal leds.
	ledM = []
	led_range = range(1, 6)
	for i in led_range:
		# Query the database for led i from table 'leds'.
		led = DBSession.query(Leds).filter_by(uid=i).one()
		values = (led.R, led.G, led.B)
		ledM.append(values)

	res['ledM'] = ledM
	res['ran'] = led_range
	return res

# Set LED color
@view_config(route_name='set_led', request_method='POST')
def set_led(request):
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
	if(plant_id != id):
		return HTTPBadRequest('Id is 42')
	if(led_id < 0 or led_id > 5):
		return HTTPBadRequest('Invalid LED ID')
	if((color != 'R' and color != 'G' and color != 'B' and color != 'W') or (color == 'W' and led_id != 0)):
		return HTTPBadRequest('Invalid color')
	if(value < 0 or value > 255):
		return HTTPBadRequest('Invalid value')
	led = DBSession.query(Leds).filter_by(uid=led_id).one()
	if (color == 'R'):
		led.R = value
	if (color == 'G'):
		led.G = value
	if (color == 'B'):
		led.B = value
	if (color == 'W'):
		led.W = value
	DBSession.add(led)

	if (loop != None):
		asyncio.run_coroutine_threadsafe(led_producer(led_id, color, value), loop)

	return Response('<body>Good Request</body>')

@view_config(route_name='login', renderer='makahiya:templates/login.pt')
def login(request):
	return {"google_login_url": login_url(request, 'google')}

@view_config(context='velruse.AuthenticationComplete',
			renderer='makahiya:templates/logged.pt')
def login_callback(request):
	session = request.session
	context = request.context
	user = DBSession.query(Users).filter_by(email=context.profile['verifiedEmail']).first()
	session['logged'] = user.level
	viewer = not user.level
	return {'editor': user.level,
			'viewer': viewer}

@view_config(route_name='ws', mapper=WebsocketMapper)
def echo(ws):
	while True:
		message = yield from ws.recv()
		if message is None:
			break
		yield from ws.send(message)

