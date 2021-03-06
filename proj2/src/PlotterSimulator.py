import serial, time
import tkinter as tk

UPDATE_TIME = 100

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
                source = -1;
                destination = -1;
                message = ''
                
                response = str(self._ser.readline(), 'utf-8').strip()
                
                if response.startswith("RECEIVED"):
                        parts = response.split(' ')
                        destination = int((parts[1])[5:])
                        source = int((parts[2])[7:])

                        response = str(self._ser.readline(), 'utf-8').strip()
                        if response.startswith("Received from Radio: "):
                                message = response.split(': ')[1]
                        else:
                                message = response

                return source, destination, message
                                

class PlotterView(object):

        def __init__(self, master):
                self._serial = SerialConnection()
                self._serial.set_mode(5)
                
                self._address = 11223352
                self._joined_address = 0

                self._x = 0
                self._y = 0
                self._z = 0

                #Gcode variables
                self._units_scaling = 1 #default unit is mm
                self._mode = 90 #default G90 - absolute movement

                self._master = master
                master.title("Plotter Simulation")

                #Information canvas
                self._info_canvas = tk.Canvas(master)
                self._info_canvas.pack(side = tk.TOP)
                self._plotter_address_label = tk.Label(self._info_canvas, text = "Plotter Address: {}".format(self._address))
                self._plotter_address_label.pack(side = tk.LEFT)
                self._joined_address_label = tk.Label(self._info_canvas, text = "Joined Address: {}".format("None"))
                self._joined_address_label.pack(side = tk.LEFT)
                self._location_label = tk.Label(self._info_canvas, text = "Position: ({}, {}, {})".format(self._x, self._y, self._z))
                self._location_label.pack(side = tk.RIGHT)

                #Gcode enter pane
                self._gcode_canvas = tk.Canvas(master)
                self._gcode_canvas.pack(side = tk.TOP)
                self._gcode_label = tk.Label(self._gcode_canvas, text = "Gcode:")
                self._gcode_label.pack(side = tk.LEFT)
                self._gcode_entry = tk.Entry(self._gcode_label)
                self._gcode_entry.pack(side = tk.LEFT)
                self._gcode_button = tk.Button(self._gcode_canvas, text = "Enter", command = self.execute_gcode)
                self._gcode_button.pack(side = tk.LEFT)

                #Plotter plane
                self._plotter_canvas = tk.Canvas(master, bg = 'white', height = 800, width = 800)
                self._plotter_canvas.pack(side = tk.TOP)

                       
                
                

        def update(self):
                source, destination, message = self._serial.get_message()
                self._address = destination

                if source == -1:
                        self._master.after(UPDATE_TIME, self.update)
                        return
                if message.startswith('JOIN'):
                        self._joined_address = source
                        self._x = 0
                        self._y = 0
                        self._z = 0
                        self.reset()
                        print("Joined Address changed to " + str(self._joined_address))
                elif source == self._joined_address:
                        print(str(destination) + ' received message from ' + str(source) + ': ' + message)
                        if message.startswith("XYZ"):
                                x = int(message[3:6])
                                y = int(message[6:9])
                                z = int(message[9:11])

                                self.move_plotter(x, y, z)
                                
                                print("({}, {}, {})".format(x, y, z))
                else:
                        print("Received message from unjoined address: " + str(source))

                #Update GUI info
                self._plotter_address_label.config(text = "Plotter Address: {}".format(self._address))
                self._joined_address_label.config(text = "Joined Address: {}".format(self._joined_address if self._joined_address else "None"))
                self._location_label.config(text = "({}, {}, {})".format(self._x, self._y, self._z))

                #Register another update
                self._master.after(UPDATE_TIME, self.update)
        
        def move_plotter(self, x, y, z):
                #print("Started move at ({}, {}, {})".format(self._x, self._y, self._z))
                if(self._z):
                        #Move in x direction
                        self._plotter_canvas.create_line(4 * self._x, 800 - 4 * self._y, 4 * x, 800 - 4 * self._y, tags = tk.ALL)
                        self._x = x

                        #Move in y direction
                        self._plotter_canvas.create_line(4 * self._x, 800 - 4 * self._y, 4 * self._x, 800 - 4 * y, tags = tk.ALL)
                        self._y = y
                else:
                        self._x = x
                        self._y = y

                #Move in z direction
                self._z = z
                #print("Ended move at ({}, {}, {})".format(self._x, self._y, self._z))
                return

        def execute_gcode(self):
                entry = self._gcode_entry.get()
                #Change units to inches
                if entry.startswith("G20"):
                        self._units_scaling = 25.4
                        print("Changed Gcode units to inches")
                        
                #Change units to mm
                elif entry.startswith("G21"):
                        self._units_scaling = 1
                        print("Changed Gcode units to mm")
                        
                #Change mode to absolute distance
                elif entry.startswith("G90"):
                        self._mode = 90
                        print("Changed Gcode mode to absolute distance")
                        
                #Change mode to incremntal distance
                elif entry.startswith("G91"):
                        self._mode = 91
                        print("Changed Gcode mode to incremental distance")

                #Move
                elif entry.startswith("G0"):
                        parts = entry.split(' ')
                        parts.pop(0)
                        x = self._x
                        y = self._y
                        z = self._z
                        for part in parts:
                                if part[0] == 'X':
                                        value = int(int(part[1:]) * self._units_scaling)
                                        if self._mode == 90:
                                                x = value
                                        else:
                                                x += value

                                elif part[0] == 'Y':
                                        value = int(int(part[1:]) * self._units_scaling)
                                        if self._mode == 90:
                                                y = value
                                        else:
                                                y += value
                                        
                                elif part[0] == 'Z':
                                        value = int(int(part[1:]) * self._units_scaling)
                                        if self._mode == 90:
                                                z = value
                                        else:
                                                z += value
                        self.move_plotter(x, y, z)
                        print("Moved plotter to ({}, {}, {}) from Gcode".format(x, y, z))
                        
                        #Update GUI info
                        self._location_label.config(text = "({}, {}, {})".format(self._x, self._y, self._z))
                        
                self._gcode_entry.delete(0, tk.END)
                return
        
        def reset(self):
                self._plotter_canvas.delete(tk.ALL)
                return

if __name__ == "__main__":
        root = tk.Tk()
        view = PlotterView(root)
        root.after(UPDATE_TIME, view.update)
        root.mainloop()
