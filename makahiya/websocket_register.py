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
	
	def register(self, id):
		if not (id in self.dict):
			self.dict[id] = WS()
			print (str(id) + ' registered in ' + self.name)

	def unregister(self, id):
		if (id in self.dict):
			del self.dict[id]
			print (str(id) + ' unregistered in ' + self.name)

	def get_message(self, id):
		return self.dict[id].message

	def set_message(self, id, message):
		self.dict[id].message = message

	def get_var(self, id):
		return self.dict[id].var

	def get_new(self, id):
		return self.dict[id].new

	def set_new(self, id, val):
		self.dict[id].new = val
		
	def registered(self, id):
		if (id in self.dict):
			return True
		else:
			return False

