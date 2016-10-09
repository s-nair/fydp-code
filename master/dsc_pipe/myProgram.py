from twisted.internet.protocol import Protocol, Factory
from twisted.internet import reactor
import smbus
import time

class ListeningService(Protocol):
	def connectionMade(self):
		self.factory.clients.append(self)
		print "clients are ", self.factory.clients

	def connectionLost(self, reason):
		print "connection lost ", self.factory.clients
		self.factory.clients.remove(self)


	def dataReceived(self, dataR):
		address = 0x04 #address of arduino
		bus = smbus.SMBus(1)
		if (dataR == 'P7H'):
			print "Data received: ", dataR
			dataS = 1				
			bus.write_byte(address, dataS)
			print "Data sent: ", dataS

		elif (dataR == 'P7L'):
			print "Data received: ", dataR
			dataS = 0
			bus.write_byte(address, dataS)
			print "Data sent: ", dataS

		elif (dataR == '2'):
			print "Data received: ", dataR
			dataS = 2
			bus.write_byte(address, dataS)
			print "Data sent: ", dataS
			dataR2 = 9999
			counter = 0
			while dataR2 != 0 and dataR2 != 1 and counter < 16:
				dataR2 = bus.read_byte(address)
				counter = counter + 1
				print "Attempt ", counter
				print "Data: ", dataR2
			print "Data received: ", dataR2
			dataS2 = dataR2
			for c in self.factory.clients:
				c.message(dataS2)
				print "Data sent: ", dataS2


	def message(self, data):
		self.transport.write(str(data) + '\n')

factory = Factory()
factory.protocol = ListeningService
factory.clients = []

reactor.listenTCP(7777, factory)
print "ListeningService has started."
reactor.run()