from pyramid.response import Response
from pyramid.httpexceptions import HTTPBadRequest
from pyramid.view import view_config
from .models import DBSession, Leds

id = 42

# Home view
@view_config(route_name='home')
def home(request):
	res = '<h1> Makahiya </h1>'
	led = DBSession.query(Leds).filter_by(uid=0).one()
	res += 'LED HP --> R = '
	res += str(led.R)
	res += ' ; G = '
	res += str(led.G)
	res += ' ; B = '
	res += str(led.B)
	res += ' ; W = '
	res += str(led.W)
	res += '</br>'
	for i in range(1, 6):
		led = DBSession.query(Leds).filter_by(uid=i).one()
		res += 'LED M' + str(i) + ' --> R = '
		res += str(led.R)
		res += ' ; G = '
		res += str(led.G)
		res += ' ; B = '
		res += str(led.B)
		res += '</br>'

	return Response(res)

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
		return HTTPBadRequest('Id or value is number')
	if(plant_id != id):
		return HTTPBadRequest('Id is 42')
	if(led_id < 0 or led_id > 5):
		return HTTPBadRequest('Invalid led ID')
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
	return Response('<body>Good Request</body>')

