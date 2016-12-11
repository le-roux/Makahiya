import asyncio

class PlantWS():
	def __init__(self):
		self.message = ""
		self.var = asyncio.Condition()
		self.new = 0

class PlantRegister():
	def __init__(self):
		self.dict = {}
	
	def register(self, id):
		if not (id in self.dict):
			self.dict[id] = PlantWS()

	def unregister(self, id):
		if (id in self.dict):
			del self.dict[id]

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

