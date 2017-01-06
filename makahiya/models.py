from pyramid.security import Allow, Everyone, Authenticated

from sqlalchemy import (
	Column,
	Integer,
	Text,
	String,
	ForeignKey,
	)

from sqlalchemy.ext.declarative import declarative_base

from sqlalchemy.orm import (
	scoped_session,
	sessionmaker,
	relationship,
	)

# Handle to the database.
Session = sessionmaker()

# Base element representing the database. It'll hold all the tables we'll
# create in a declarative way (using Python classes).
Base = declarative_base()

class Users(Base):
	"""
		Database table storing the users and their rights.
		Level:
			0 : simple user
			1 : editor
	"""
	__tablename__ = 'users'
	uid = Column(Integer, primary_key=True)
	email = Column(String)
	level = Column(Integer)
	plant_id = Column(Integer)

# Table containing the leds status.
class Leds(Base):
	__tablename__ = 'leds'
	uid = Column(Integer, primary_key=True)
	userid = Column(Integer)
	R = Column(Integer)
	G = Column(Integer)
	B = Column(Integer)
	W = Column(Integer)

# Authorization stuff (access control list).
class Root(object):
	__acl__ = [(Allow, Everyone, 'view'),
		(Allow, Authenticated, 'logged'),
		(Allow, 'group:sudo', 'sudo'),
		(Allow, 'group:editors', 'edit'),
		(Allow, 'group:viewers', 'view')]

	def __init__(self, request):
		pass

def get_user_level(userid):
    return Session().query(Users).filter_by(email=userid).first().level
