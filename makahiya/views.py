from pyramid.response import Response
from pyramid.httpexceptions import HTTPBadRequest
from pyramid.view import view_config
from .models import Session, Leds, Users
from .custom_mapper import CustomWebsocketMapper
from aiopyramid.config import CoroutineMapper
from .models import Session, Leds, Users
from velruse import login_url
from .plant_register import PlantRegister

import asyncio
import pyramid

id = 42
pr = PlantRegister()

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
	session = Session()
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
def set_led(request):
	session = Session()
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

	yield from pr.get_var(plant_id).acquire()
	pr.set_message(plant_id, 'led ' + str(led_id) + ' ' + color + ' ' + str(value))
	pr.set_new(plant_id, 1)
	pr.get_var(plant_id).notify()
	pr.get_var(plant_id).release()

	return Response('<body>Good Request</body>')

@view_config(route_name='set_servo', request_method='POST', mapper=CoroutineMapper)
def set_servo(request):
	plant_id = request.matchdict['plant_id']
	servo_id = request.matchdict['servo_id']
	value = request.matchdict['value']
	try:
		plant_id = int(plant_id)
		servo_id = int(servo_id)
		value = int(value)
	except ValueError:
		return HTTPBadRequest('Some number could not be casted')
	if(plant_id != id):
		return HTTPBadRequest('Id is 42')
	if(servo_id < 0 or servo_id > 4):
		return HTTPBadRequest('Invalid servo ID')
	if(value < 0 or value > 200):
		return HTTPBadRequest('Invalid value')

	yield from pr.get_var(plant_id).acquire()
	pr.set_message(plant_id, 'servo ' + str(servo_id) + ' ' + str(value))
	pr.set_new(plant_id, 1)
	pr.get_var(plant_id).notify()
	pr.get_var(plant_id).release()

	return Response('<body>Good Request</body>')

@view_config(route_name='login', renderer='makahiya:templates/login.pt')
def login(request):
	return {"google_login_url": login_url(request, 'google')}

@view_config(context='velruse.AuthenticationComplete',
			renderer='makahiya:templates/logged.pt')
def login_callback(request):
	session = Session()
	context = request.context
	user = session.query(Users).filter_by(email=context.profile['verifiedEmail']).first()
	request.session['logged'] = user.level
	viewer = not user.level
	return {'editor': user.level,
			'viewer': viewer}

async def socket_send(plant_id):
	await pr.get_var(plant_id).acquire()
	await pr.get_var(plant_id).wait()
	pr.get_var(plant_id).release()

@view_config(route_name='plant_ws', mapper=CustomWebsocketMapper)
async def plant(ws):
	plant_id = ws.matchdict['plant_id']
	try:
		plant_id = int(plant_id)
	except ValueError:
		return HTTPBadRequest('id is a number')
	pr.register(plant_id)
	while True:
		listener_task = asyncio.ensure_future(ws.recv())
		producer_task = asyncio.ensure_future(socket_send(plant_id))
		done, pending = await asyncio.wait(
		    [listener_task, producer_task],
		    return_when=asyncio.FIRST_COMPLETED)

		await pr.get_var(plant_id).acquire()

		if listener_task in done:
			pr.set_message(plant_id, listener_task.result())
			await ws.send(msg)
			pr.get_var(plant_id).notify()
			pr.get_var(plant_id).release()
			await asyncio.wait([producer_task])
			await pr.get_var(plant_id).acquire()
		else:
			listener_task.cancel()

		if(pr.get_new(plant_id)):
			await ws.send(pr.get_message(plant_id))
			pr.set_new(plant_id, 0)

		pr.get_var(plant_id).release()

