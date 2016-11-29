import unittest
import transaction

from pyramid import testing

def initTestDb():
	from sqlalchemy import create_engine
	from .models import (DBSession,
			Leds,
			Base)
	engine = create_engine('sqlite://')
	DBSession.configure(bind=engine)
	Base.metadata.create_all(engine)
	with transaction.manager:
		for i in range(0, 6):
			model = Leds(uid=i, R=0, G=0, B=0, W=0)
			DBSession.add(model)
	return DBSession

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
		self.assertEqual(range(1,6), response['ran'])

class HomeFunctionalTests(unittest.TestCase):
    def setUp(self):
        from pyramid.paster import get_app
        app = get_app('development.ini')
        from webtest import TestApp
        self.testapp = TestApp(app)

    def tearDown(self):
        from .models import DBSession
        DBSession.remove()

    def test_it(self):
        res = self.testapp.get('/', status=200)
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

		response = set_led(request)
		self.assertIn(b'Good Request', response.body)

	def test_bad_plant_id(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '24'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'R'
		request.matchdict['value'] = '19'

		response = set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_bad_color(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'D'
		request.matchdict['value'] = '19'

		response = set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_white_middle_power(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'W'
		request.matchdict['value'] = '19'

		response = set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_bad_led_id(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '7'
		request.matchdict['color'] = 'R'
		request.matchdict['value'] = '19'

		response = set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_bad_value(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'R'
		request.matchdict['value'] = '300'

		response = set_led(request)
		self.assertEqual(response.status_code, 400)

	def test_not_a_number(self):
		from .views import set_led

		request = testing.DummyRequest()
		request.matchdict['plant_id'] = '42'
		request.matchdict['led_id'] = '1'
		request.matchdict['color'] = 'R'
		request.matchdict['value'] = 'foo'

		response = set_led(request)
		self.assertEqual(response.status_code, 400)
