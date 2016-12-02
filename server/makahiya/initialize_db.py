import os
import sys
import transaction
import psycopg2
import urlparse

from sqlalchemy import engine_from_config

from pyramid.paster import (
	get_appsettings,
	setup_logging,
	)

from .models import (
	DBSession,
	Leds,
	Base,
	)


urlparse.uses_netloc.append("postgres")
url = urlparse.urlparse(os.environ["DATABASE_URL"])

conn = psycopg2.connect(
    database=url.path[1:],
    user=url.username,
    password=url.password,
    host=url.hostname,
    port=url.port
)


def usage(argv):
	cmd = os.path.basename(argv[0])
	print('usage: %s <config_uri>\n'
	    '(example: "%s development.ini")' % (cmd, cmd))
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
	with transaction.manager:
		for i in range(0, 6):
			model = Leds(uid=i, R=0, G=0, B=0, W=0)
			DBSession.add(model)
