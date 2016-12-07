import unittest
import transaction

from pyramid import testing

def initTestDb():
	from sqlalchemy import create_engine
	from sqlalchemy.orm import (relationship,
			scoped_session)
	from .models import (Session,
			Leds,
			Base,
			Users)
	engine = create_engine('sqlite:///test')
	session = scoped_session(Session)
	Base.metadata.create_all(engine)
	session.configure(bind=engine)
	engine.execute("DELETE FROM leds;")
	engine.execute("DELETE FROM users;")
	Users.leds = relationship("Leds", back_populates="user")
	Base.metadata.create_all(engine)
	user = Users(uid=0, email='sylvain.leroux3@gmail.com', level=1)
	with transaction.manager:
		for i in range(0, 6):
			model = Leds(uid=i, R=0, G=0, B=0, W=0, userid=0)
			session.add(model)
	return session

class HomeViewTest(unittest.TestCase):
	def setUp(self):
		self.session = initTestDb()
		self.config = testing.setUp()

	def tearDown(self):
		self.session.remove()
		testing.tearDown()

	def test_home_view(self):
		from .views import home

		request = testing.DummyRequest()
		response = home(request)
		self.assertEqual('Makahiya', response['title'])

	def test_led_view(self):
		from .views import led_view

		request = testing.DummyRequest()
		response = led_view(request)
		self.assertEqual(range(1,6), response['ran'])

class HomeFunctionalTests(unittest.TestCase):
	def setUp(self):
		from pyramid.paster import get_app
		app = get_app('tests.ini')
		from webtest import TestApp
		self.testapp = TestApp(app)

	def tearDown(self):
		from .models import Session
		from sqlalchemy.orm import (scoped_session)
		scoped_session(Session).remove()

	def test_it(self):
		res = self.testapp.get('/led', status=200)
		self.assertIn(b'Makahiya', res.body)
		self.assertIn(b'LED HP', res.body)
		self.assertIn(b'LED M1', res.body)
		self.assertIn(b'LED M2', res.body)
		self.assertIn(b'LED M3', res.body)
		self.assertIn(b'LED M4', res.body)
		self.assertIn(b'LED M5', res.body)

class SetLedTest(unittest.TestCase):

	def setUp(self):
		self.session = initTestDb()
		self.config = testing.setUp()

	def tearDown(self):
		self.session.remove()
		testing.tearDown()

	def test_success(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '0'
		request.matchdict['color'] = 'W'
		request.matchdict['value'] = '19'

		response = yield from set_led(request)
		self.assertIn(b'Good Request', response.body)

	def test_bad_plant_id(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '24'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'R'
		request.matchdict['value'] = '19'

		response = yield from set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_bad_color(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'D'
		request.matchdict['value'] = '19'

		response = yield from set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_white_middle_power(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'W'
		request.matchdict['value'] = '19'

		response = yield from set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_bad_led_id(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '7'
		request.matchdict['color'] = 'R'
		request.matchdict['value'] = '19'

		response = yield from set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_bad_value(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'R'
		request.matchdict['value'] = '300'

		response = yield from set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_not_a_number(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'R'
		request.matchdict['value'] = 'foo'

		response = yield from set_led(request)
		self.assertEqual(response.status_code, 400)
