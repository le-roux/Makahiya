from pyramid.config import Configurator
from pyramid.session import SignedCookieSessionFactory

from sqlalchemy import engine_from_config

from .models import DBSession, Base

import asyncio
import websockets
from threading import Thread

from .websocket import led

loop = None

def launch_websocket():
	asyncio.set_event_loop(loop)
	start_server = websockets.serve(led, "0.0.0.0", 8765)
	loop.run_until_complete(start_server)
	loop.run_forever()

def main(global_config, **settings):
	engine = engine_from_config(settings, 'sqlalchemy.')
	DBSession.configure(bind=engine)
	Base.metadata.bind = engine
	session_factory = SignedCookieSessionFactory('makahiya')

	config = Configurator(settings=settings,
				root_factory='makahiya.models.Root',
				session_factory=session_factory)
	config.include('pyramid_chameleon')
	config.include('velruse.providers.google_oauth2')
	config.add_google_oauth2_login_from_settings()
	config.add_route('home', '/')
	config.add_route('led', '/led')
	config.add_route('set_led', '/api/v1/{plant_id}/actions/{led_id}/{color}/{value}')
	config.add_route('login', '/login')
	config.add_static_view(name='static', path='makahiya:static')

	global loop
	loop = asyncio.get_event_loop()
	thread = Thread(target = launch_websocket)
	thread.start()

	config.scan('.views')
	return config.make_wsgi_app()
