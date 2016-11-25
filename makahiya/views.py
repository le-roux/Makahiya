from pyramid.response import Response
from pyramid.view import view_config
from .models import DBSession, Leds

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

