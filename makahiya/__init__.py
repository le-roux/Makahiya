from pyramid.config import Configurator
from pyramid.session import SignedCookieSessionFactory

from sqlalchemy import engine_from_config

from .models import Session, Base

import os
import asyncio
import websockets
from threading import Thread

def main(global_config, **settings):
	engine = engine_from_config(settings, 'sqlalchemy.')
	Session.configure(bind=engine)
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
	config.add_route('set_led', '/api/v1/{plant_id}/actions/led/{led_id}/{color}/{value}')
	config.add_route('set_servo', '/api/v1/{plant_id}/actions/servo/{servo_id}/{value}')
	config.add_route('login', '/login')
	config.add_route('ws', '/ws')
	config.add_static_view(name='static', path='makahiya:static')

	config.scan('.views')
	return config.make_wsgi_app()

