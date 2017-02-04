from pyramid.config import Configurator
from pyramid.session import SignedCookieSessionFactory
from pyramid.authentication import AuthTktAuthenticationPolicy
from pyramid.authorization import ACLAuthorizationPolicy
from .security import groupfinder

from sqlalchemy import engine_from_config

from .models import Session, Base

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

	# Route configuration
	config.add_route('home', '/')
	config.add_route('board_leds', '/{plant_id}/board/leds')
	config.add_route('board_timer', '/{plant_id}/board/timer')
	config.add_route('timer_deactivate', '/{plant_id}/board/timer/deactivate')
	config.add_route('quick_timer', '/{plant_id}/board/timer/{time}')
	config.add_route('touch_config', '/{plant_id}/touch')
	config.add_route('touch_config_delete', '/{plant_id}/touch/delete/{leaf_id}')
	config.add_route('music', '/{plant_id}/music')
	config.add_route('music_play', '/{plant_id}/music/play')
	config.add_route('wrong_id', '/wrong_id')
	config.add_route('led', '/led')
	config.add_route('set_led', '/api/v1/{plant_id}/actions/led/{led_id}/{color}/{value}')
	config.add_route('set_servo', '/api/v1/{plant_id}/actions/servo/{servo_id}/{value}')
	config.add_route('login', '/login')
	config.add_route('logout', '/logout')
	config.add_route('subscribe', '/subscribe')
	config.add_route('plant_ws', '/ws/plants/{plant_id}')
	config.add_route('client_ws', '/ws/clients/{client_id}')
	config.add_route('users', '/users')
	config.add_route('delete', '/delete/{email}')
	config.add_static_view(name='static', path='makahiya:static')
	config.add_static_view('deform_static', 'deform:static/')

	# Security configuration
	authn_policy = AuthTktAuthenticationPolicy('secret',
	 					callback = groupfinder, hashalg='sha512')
	authz_policy = ACLAuthorizationPolicy()
	config.set_authentication_policy(authn_policy)
	config.set_authorization_policy(authz_policy)


	config.scan('.views')
	config.scan('.websockets')
	return config.make_wsgi_app()
