from serial.tools import list_ports
from serial import Serial
from serial import SerialException
import PySimpleGUI as sg
from PyCRC.CRC32 import CRC32
from enum import IntEnum


# define the same enums as ledCmdExecutor.h
class CMD_ID(IntEnum):
    cmd_all_off = 0
    cmd_all_on = 1
    cmd_steady = 2
    cmd_blink = 3
    cmd_none = 255  # not actually a command


# build a command with some simple framing to send to the MCU
# <0x02> <cmdNum> <red> <green> <blue> <crc_lsb> <crc_lsb+1> <crc_lsb+2> <crc_msb>
def buildCmd(cmdNum: int, red: int, green: int, blue: int):
    cmd = bytearray([0x02, int(cmdNum & 0xff), int(red) & 0xff, int(green) & 0xff, int(blue) & 0xff])
    crc = CRC32().calculate(bytes(cmd[0:4]))
    cmd.append(crc & 0x000000FF)
    cmd.append((crc & 0x0000FF00) >> 8)
    cmd.append((crc & 0x00FF0000) >> 16)
    cmd.append((crc & 0xFF000000) >> 24)
    return cmd


#   Create the main UI window and allow the user to select from a list of ports
#
#   @param expects list of COM Ports output from getPorts
#   @returns sg.layout and sg.window
def createLayout(ports: list):
    sg.theme('SystemDefault')

    layout = [
        [sg.DropDown(ports, size=(12, 1), key='COMPORT', enable_events=True),
         sg.Radio('Steady', group_id='CMD', default=True, key='CMD_STEADY', enable_events=True),
         sg.Radio('Blink', group_id='CMD', key='CMD_BLINK', enable_events=True),
         sg.Radio('UseButton', group_id='CMD', default=False, visible=False, key='CMD_NO_RAD')],
        [sg.Text('red    '), sg.Slider(range=(0, 255), key='RED',
                                       orientation='hor', enable_events=True)],
        [sg.Text('green'), sg.Slider(range=(0, 255), key='GREEN',
                                     orientation='hor', enable_events=True)],
        [sg.Text('blue  '), sg.Slider(range=(0, 255), key='BLUE',
                                      orientation='hor', enable_events=True)],
        [sg.Text('status: '), sg.Text('', key='status', size=(50, 1))],
        [sg.Button('ALL ON', key='CMD_ALL_ON', enable_events=True), sg.Button('ALL OFF', key='CMD_ALL_OFF', enable_events=True)]
    ]

    window = sg.Window('Nucleo Color Selector', layout)
    window.finalize()
    return layout, window


# @param window main window, which will be evaluated for events
# @param ser serial port used
# returns 0 when there is nothing to do
# returns 1 when update to LED's is required, along with the CMD_ID
# returns -1 on window close
def evaluateUI(window: sg.Window, ser: Serial):
    event, values = window.read(1000)
    print(event, values)
    if event in (None, 'Exit'):
        return -1, CMD_ID.cmd_none
    elif event == 'COMPORT':
        return 2, CMD_ID.cmd_none
    elif event == 'RED' or event == 'GREEN' or event == 'BLUE':
        # if there was a slider event, make sure to update and
        # return the necessary command (blinking or steady)
        if window['CMD_STEADY'].Get() is True:
            return 1, CMD_ID.cmd_steady
        else:
            return 1, CMD_ID.cmd_blink
    elif event == 'CMD_STEADY':
        return 1, CMD_ID.cmd_steady
    elif event == 'CMD_BLINK':
        return 1, CMD_ID.cmd_blink
    elif event == 'CMD_ALL_ON':
        # update state of the radio buttons and sliders
        window['CMD_STEADY'].Update(True)
        window['RED'].Update(255)
        window['GREEN'].Update(255)
        window['BLUE'].Update(255)
        window.Refresh()
        return 1, CMD_ID.cmd_all_on
    elif event == 'CMD_ALL_OFF':
        # update state of the radio buttons and sliders
        window['CMD_STEADY'].Update(True)
        window['RED'].Update(0)
        window['GREEN'].Update(0)
        window['BLUE'].Update(0)
        window.Refresh()
        return 1, CMD_ID.cmd_all_off
    else:
        return 0, CMD_ID.cmd_none


# Produce a list of serial ports
def getPorts():
    allPorts = list(list_ports.comports())
    comPortList = [x[0] for x in allPorts]
    return comPortList


# return current slider values
def getSliderValues(window: sg.Window):
    event, values = window.read(0)
    return values['RED'], values['GREEN'], values['BLUE']


def openComPort(portName: str):
    if(portName != ''):
        try:
            ser = Serial(portName)
            return ser
        except SerialException:
            return None
    return None


# return the current COM port selection in the dropdown
def selectComPort(window: sg.Window):
    setStatus(window, "Select COM Port")
    while(True):
        event, values = window.read()
        if event == 'COMPORT':
            portName = values['COMPORT']
            ser = openComPort(portName)
            if ser is not None:
                setStatus(window, "%s open" % portName)
                return ser
            else:
                setStatus(window, "UNABLE to open %s" % portName)
        elif event in (None, 'Exit'):
            return None


# update the status string at the botton of the window
def setStatus(window: sg.Window, statusText: str):
    window['status'].update(statusText)
    window.Refresh()


def main():
    availablePorts = getPorts()
    layout, window = createLayout(availablePorts)

    setStatus(window, "select COM port")

    # wait until a valid port is selected from the dropdown
    ser = selectComPort(window)
    if ser is None:
        return

    # run the primary UI polling loop which watches for UI events
    # and takes the appropriate action
    while True:
        action, cmd = evaluateUI(window, ser)
        if action == 1:
            # update required
            red, green, blue = getSliderValues(window)
            setStatus(window, "%i %i %i %i" % (cmd, red, green, blue))
            cmdBytes = buildCmd(cmd, red, green, blue)
            ser.write(cmdBytes)
            setStatus(window, "sent cmd  (0x)" + cmdBytes.hex())
        elif action == 2:
            # selected COM PORT changed
            # close the previously opened com port
            # disableSliders(window)
            ser.close()
            ser = selectComPort(window)
            if ser is None:
                return
        elif action == -1:
            # window was closed, time to exit
            ser.close()
            window.close()
            break


main()
