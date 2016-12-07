import os
import sys
import transaction
import psycopg2
import urllib.parse

from sqlalchemy import engine_from_config

from pyramid.paster import (
	get_appsettings,
	setup_logging,
	)

from .models import (
	DBSession,
	Leds,
	Users,
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
	DBSession.configure(bind=engine)

	# Create the tables (if they don't already exist).
	Base.metadata.create_all(engine)

	# Clear the current content of the leds table.
	engine.execute("DELETE FROM leds;")

	# Fill the database with initial values.
	with transaction.manager:
		# Fill the 'leds' table.
		for i in range(0, 6):
			model = Leds(uid=i, R=0, G=0, B=0, W=0)
			DBSession.add(model)
		# Fill the 'users' table
		user = Users(uid=0, email='sylvain.leroux3@gmail.com', level=1)
		DBSession.add(user)
