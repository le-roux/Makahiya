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
	engine = engine_from_config(settings, 'sqlalchemy.')
	Base.metadata.create_all(engine)
	Session.configure(bind=engine)
	engine.execute("DELETE FROM leds;")
	engine.execute("DELETE FROM users;")
	session = Session()
	Users.leds = relationship("Leds", back_populates="user")
	Base.metadata.create_all(engine)
	user = Users(uid=0, email='sylvain.leroux3@gmail.com', level=1)
	session.add(user)
	for i in range(0, 6):
		model = Leds(uid=i, R=0, G=0, B=0, W=0, userid=0)
		session.add(model)
	session.commit()
