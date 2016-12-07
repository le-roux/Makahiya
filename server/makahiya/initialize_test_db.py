import os
import sys
import transaction

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
	engine = engine_from_config(settings, 'sqlalchemy.')
	DBSession.configure(bind=engine)
	Base.metadata.create_all(engine)
	cmd = "DELETE FROM leds;"
	engine.execute(cmd)
	with transaction.manager:
		for i in range(0, 6):
			model = Leds(uid=i, R=0, G=0, B=0, W=0)
			DBSession.add(model)
		user = Users(uid=0, email='sylvain.leroux3@gmail.com', level=1)
		DBSession.add(user)
