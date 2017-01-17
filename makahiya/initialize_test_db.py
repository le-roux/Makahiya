import os
import sys
import transaction

from sqlalchemy import engine_from_config
from sqlalchemy.orm import relationship

from pyramid.paster import (
	get_appsettings,
	setup_logging,
	)

from .models import (
	Session,
	Leds,
	Users,
	Base,
	)

def usage(argv):
	cmd = os.path.basename(argv[0])
	print('usage: %s <config_uri>\n'
	    '(example: "%s tests.ini")' % (cmd, cmd))
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

	# Fill the 'leds' table with initial values.
	for i in range(0, 6):
		model = Leds(R=0, G=0, B=0, W=0, plant_id=0, led_id=i, on=False)
		session.add(model)
	session.commit()
