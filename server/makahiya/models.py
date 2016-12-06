from pyramid.security import Allow, Everyone

from sqlalchemy import (
	Column,
	Integer,
	Text,
	)

from sqlalchemy.ext.declarative import declarative_base

from sqlalchemy.orm import (
	scoped_session,
	sessionmaker,
	)

from zope.sqlalchemy import ZopeTransactionExtension

DBSession = scoped_session(
	sessionmaker(extension=ZopeTransactionExtension()))
Base = declarative_base()

# Table containing the leds status.
class Leds(Base):
	__tablename__ = 'leds'
	uid = Column(Integer, primary_key=True)
	R = Column(Integer)
	G = Column(Integer)
	B = Column(Integer)
	W = Column(Integer)

# Authorization stuff (access control list).
class Root(object):
	__acl__ = [(Allow, Everyone, 'view'),
		 (Allow, 'group:editors', 'edit')]

	def __init__(self, request):
		pass
