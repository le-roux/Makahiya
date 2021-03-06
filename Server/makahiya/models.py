from pyramid.security import Allow, Everyone, Authenticated
import colander

from sqlalchemy import (
	Column,
	Integer,
	Text,
	String,
	Boolean,
	ForeignKey,
	Boolean,
	DateTime,
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
			2 : simple user
			1 : editor/master
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
	plant_id = Column(Integer)
	led_id = Column(Integer)
	R = Column(Integer)
	G = Column(Integer)
	B = Column(Integer)
	W = Column(Integer)
	on = Column(Boolean)

# Table containing the servos status
class Servos(Base):
	__tablename__ = 'Servos'
	uid = Column(Integer, primary_key=True)
	plant_id = Column(Integer)
	servo_id = Column(Integer)
	pos = Column(Integer)

# Timers
class Timers(Base):
	__tablename__ = 'timers'
	uid = Column(Integer, primary_key=True)
	plant_id = Column(Integer)
	activated = Column(Boolean)
	date = Column(DateTime)
	sound = Column(Integer)
	light = Column(Integer)

# Touch reaction
class Touch(Base):
	__tablename__ = 'touch'
	uid = Column(Integer, primary_key=True)
	plant_id = Column(Integer)
	leaf_id = Column(Integer)
	commands = Column(String)

# Music status
class Music(Base):
	__tablename__ = 'Music'
	uid = Column(Integer, primary_key=True)
	plant_id = Column(Integer)
	uploaded = Column(Boolean)
	playing = Column(Boolean)

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

def get_user_plant_id(userid):
	return Session().query(Users).filter_by(email=userid).first().plant_id

class LedsForm(colander.MappingSchema):
	ledH_R = colander.SchemaNode(colander.Int())
	ledH_G = colander.SchemaNode(colander.Int())
	ledH_B = colander.SchemaNode(colander.Int())
	ledH_W = colander.SchemaNode(colander.Int())

	led1_R = colander.SchemaNode(colander.Int())
	led1_G = colander.SchemaNode(colander.Int())
	led1_B = colander.SchemaNode(colander.Int())

	led2_R = colander.SchemaNode(colander.Int())
	led2_G = colander.SchemaNode(colander.Int())
	led2_B = colander.SchemaNode(colander.Int())

	led3_R = colander.SchemaNode(colander.Int())
	led3_G = colander.SchemaNode(colander.Int())
	led3_B = colander.SchemaNode(colander.Int())

	led4_R = colander.SchemaNode(colander.Int())
	led4_G = colander.SchemaNode(colander.Int())
	led4_B = colander.SchemaNode(colander.Int())

	led5_R = colander.SchemaNode(colander.Int())
	led5_G = colander.SchemaNode(colander.Int())
	led5_B = colander.SchemaNode(colander.Int())

	led6_R = colander.SchemaNode(colander.Int())
	led6_G = colander.SchemaNode(colander.Int())
	led6_B = colander.SchemaNode(colander.Int())
