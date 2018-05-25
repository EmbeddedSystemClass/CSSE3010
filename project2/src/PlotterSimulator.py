import serial, time

class SerialConnection(object):

        def __init__(self):

                #Configure serial connection
                self._ser = serial.Serial()
                self._ser.port = "COM11" 
                self._ser.baudrate = 115200
                self._ser.bytesize = serial.EIGHTBITS #number of bits per bytes
                self._ser.parity = serial.PARITY_NONE #set parity check: no parity
                self._ser.stopbits = serial.STOPBITS_ONE #number of stop bits
                self._ser.timeout = 1            #non-block read
                self._ser.xonxoff = False     #disable software flow control
                self._ser.rtscts = False     #disable hardware (RTS/CTS) flow control
                self._ser.dsrdtr = False       #disable hardware (DSR/DTR) flow control
                self._ser.writeTimeout = 2     #timeout for write

                #Open serial 
                self._ser.open()
                if self._ser.isOpen():
                        self._ser.flushInput()
                        self._ser.flushOutput()

        def set_mode(self, mode):
                response = ''
                while not response.startswith('Mode ' + str(mode)):
                        self._ser.write(bytes(str(mode), 'utf-8'))
                        time.sleep(0.5)
                        self._ser.write(b'\r')
                        response = str(self._ser.readline(), 'utf-8').strip()

        def get_message(self):
                receivedPacket = 0
                receivedMessage = 0
                source = 0;
                destination = 0;
                message = ''

                
                while True:
                        if (receivedPacket and receivedMessage):
                                break
                        
                        response = str(self._ser.readline(), 'utf-8').strip()
                        
                        if response != '':
                                
                                if response.startswith("RECEIVED"):
                                        parts = response.split(' ')
                                        destination = int((parts[1])[5:])
                                        source = int((parts[2])[7:])

                                        receivedPacket = 1;
                                        
                                elif response.startswith("Received from Radio:"):
                                        message = response.split(': ')[1]
                                        message.strip(chr(0))
                                        
                                        receivedMessage = 1;

                return source, destination, message
                                

def main():

        serial = SerialConnection()
        serial.set_mode(5)

        myAddress = 11223352
        joinedAddress = 0

        while True:
                source, destination, message = serial.get_message()
                
                if message.startswith('JOIN'):
                        joinedAddress = source
                        print("Joined Address changed to " + str(joinedAddress))
        
                        
                elif source == joinedAddress:
                        print(str(destination) + ' received message from ' + str(source) + ': ' + message)

                        if message.startswith("XYZ"):
                                x = int(message[3:6])
                                y = int(message[6:9])
                                z = int(message[9:11])
                                print("({}, {}, {})".format(x, y, z))
                else:
                        print("Received message from unjoined address: " + str(source))
                
        
        ser.close()

class PlotterView(object):

        def __init__(self, master):
                return
        
        def move_plotter(self, x, y, z):
                return
        
        def reset(self):
                return

if __name__ == "__main__":
        main()
        #root = tk.Tk()
        #view = PlotterView(root)
        #root.mainloop()
