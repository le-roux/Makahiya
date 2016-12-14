import asyncio

class WS():
	def __init__(self):
		self.message = ""
		self.var = asyncio.Condition()
		self.new = 0

class WebsocketRegister():
	def __init__(self, name):
		self.name = name
		self.dict = {}
	
	def register(self, identifier):
		if not (identifier in self.dict):
			self.dict[identifier] = WS()
			print (str(identifier) + ' registered in ' + self.name)

	def unregister(self, identifier):
		if (identifier in self.dict):
			del self.dict[identifier]
			print (str(identifier) + ' unregistered in ' + self.name)

	def get_message(self, identifier):
		return self.dict[identifier].message

	def set_message(self, identifier, message):
		self.dict[identifier].message = message

	def get_var(self, identifier):
		return self.dict[identifier].var

	def get_new(self, identifier):
		return self.dict[identifier].new

	def set_new(self, identifier, val):
		self.dict[identifier].new = val
		
	def registered(self, identifier):
		if (identifier in self.dict):
			return True
		else:
			return False

