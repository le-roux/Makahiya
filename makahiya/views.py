from pyramid.response import Response, FileResponse
from pyramid.httpexceptions import HTTPBadRequest, HTTPFound
from pyramid.view import view_config
from .models import Session, Leds, Users
from velruse import login_url
from pyramid.security import remember, forget
import pyramid
import logging
import colander
import deform.widget
log = logging.getLogger(__name__)

# home page
@view_config(route_name='home', renderer='makahiya:templates/home.pt')
def home(request):
	return {'title':'Makahiya',
		'id':request.authenticated_userid if not request.authenticated_userid == None else ''}

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
	return {"google_login_url": login_url(request, 'google')}

@view_config(context='velruse.AuthenticationComplete')
def login_callback(request):
	email = request.context.profile['verifiedEmail']
	session = Session()
	if 'status' in request.session and request.session['status'] == 1:
		# Check that this user doesn't already exist
		if session.query(Users).filter_by(email=email).first() is None:
			# Create this user in the database
			session.add(Users(email=email, level=2, plant_id=request.session['plant_id']))
			headers = remember(request, email)
			return HTTPFound('/board')
		else: # User already existing
			request.session['status'] = 2
			return HTTPFound('/subscribe')
	else:
		# Check that this user in in the database
		if session.query(Users).filter_by(email=email).first() is None:
			request.session['status'] = 3
			return HTTPFound('/subscribe')
		else: # User exists
			headers = remember(request, email)
			return HTTPFound(location = "/", headers = headers)

@view_config(route_name='logout', permission='logged')
def logout(request):
	headers = forget(request)
	return HTTPFound(location='https://google.com/accounts/logout', headers = headers)

class SubscribePage(colander.MappingSchema):
	plant_id = colander.SchemaNode(colander.Int())

class SecurityViews(object):
	def __init__(self, request):
		self.request = request

	@property
	def subscribe_form(self):
		schema = SubscribePage()
		return deform.Form(schema, buttons=('Associate with Google account',))

	@property
	def reqts(self):
		return self.subscribe_form.get_widget_resources()

	@view_config(route_name='subscribe', renderer='makahiya:templates/subscribe.pt')
	def subscribe(self):
		form = self.subscribe_form.render()

		if 'Associate_with_Google_account' in self.request.params:
			values = self.request.POST.items()
			try:
				pages = self.subscribe_form.validate(values)
			except deform.ValidationFailure as e:
				return dict(form=e.render())
			self.request.session['status'] = 1
			self.request.session['plant_id'] = pages['plant_id']
			return HTTPFound(login_url(self.request, 'google'))

		# First access to this page
		if 'status' not in self.request.session:
			self.request.session['status'] = 0
		return dict({'status':self.request.session['status']}, form=form)
