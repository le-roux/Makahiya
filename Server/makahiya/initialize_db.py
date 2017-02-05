import os
import sys
import transaction
import psycopg2
import urllib.parse

from sqlalchemy import engine_from_config
from sqlalchemy.orm import (relationship,
		scoped_session)

from pyramid.paster import (
	get_appsettings,
	setup_logging,
	)

from .models import (
	Session,
	Leds,
	Servos,
	Users,
	Timers,
	Touch,
	Music,
	Base,
	)


urllib.parse.uses_netloc.append("postgres")
url = urllib.parse.urlparse(os.environ["DATABASE_URL"])

conn = psycopg2.connect(
    database=url.path[1:],
    user=url.username,
    password=url.password,
    host=url.hostname,
    port=url.port
)


def usage(argv):
	"""
		Print the proper usage of the module.
	"""
	cmd = os.path.basename(argv[0])
	print('usage: %s <config_uri>\n'
	    '(example: "%s production.ini")' % (cmd, cmd))
	sys.exit(1)


def main(argv=sys.argv):
	if len(argv) != 2:
		usage(argv)
	config_uri = argv[1]
	setup_logging(config_uri)
	settings = get_appsettings(config_uri)

	# Create a SQLAlchemy engine.
	# The second argument ('sqlalchemy.') is the prefix to search for in
	# settings in order to find the relevant information to configure the
	# engine.
	engine = engine_from_config(settings, 'sqlalchemy.')

	# Connect the engine to the session.
	Session.configure(bind=engine)

	# Create the tables (if they don't already exist).
	Base.metadata.create_all(engine)

	# Clear the current content of the tables.
	engine.execute("DELETE FROM leds;")
	engine.execute("DELETE FROM users;")

	# Open a transaction with the database.
	session = Session()

	# Fill the 'users' table with initial values.
	user = Users(email='sylvain.leroux3@gmail.com', level=1, plant_id=0)
	session.add(user)

	timer = Timers(plant_id=0, activated=False, sound=0, light=0)
	session.add(timer)

	# Fill the 'leds' table with initial values.
	for i in range(0, 6):
		model = Leds(R=0, G=0, B=0, W=0, plant_id=0, led_id=i, on=False)
		session.add(model)
	for i in range(0,5):
		model = Servos(servo_id=i, pos=0, plant_id=0)
		session.add(model)
	for i in range(1,9):
		model = Touch(plant_id=0, leaf_id=i, commands='')
		session.add(model)
	model = Music(plant_id=0, uploaded=False, playing=False)
	session.add(model)
	session.commit()
