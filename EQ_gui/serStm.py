import serial
import PyQt5.QtCore
from PyQt5.QtCore import *
import time


class serStm(PyQt5.QtCore.QObject):


    log_signal =  pyqtSignal(str)

    def __init__(self):
        super(serStm, self).__init__()
        self.ser = serial.Serial()

    def log(self, msg):
        self.log_signal.emit(msg)

    def connect(self, port):
        self.ser.port = port
        self.ser.baudrate = 115200

        try:
            self.ser.open()
        except:
            self.log('Cannot open port ' +  port  + '!')
            return

        self.log('Port ' + port + ' succ. open')

    def disconnect(self):
        try:
            self.ser.close()
        except:
            pass

    def _ask(self, txt):
        self._write(txt)
        time.sleep(0.015)
        resp = self._read()
        return resp

    def _read(self):

        vals = self.ser.read_all()
        vals = vals.decode("utf-8")
        vals = vals.replace('\r', '')
        vals = vals.replace('\n', '')
        return vals

    def _write(self, txt):
        txt = txt + '\n\r'
        self.ser.write(txt.encode())
        time.sleep(0.01)


    def stmConn(self):

        if not self.ser.is_open:
            self.log('The port is not open!')
            return

        resp = self._ask('conn')
        self.log('STM : ' + resp)

    def stmReset(self):

        if not self.ser.is_open:
            self.log('The port is not open!')
            return

        resp = self._ask('reset')
        self.log('STM : ' + resp)

    def find(self):
        self._write('find')


        while True:
            text = self._read()


            if text != '':
                self.log(text)

            if text.find('End') != -1:
                break


    def stmSetAddr(self, addr):
        addr = hex(addr).replace('0x', '')
        self._write('addr ' +  addr)

    def stmReadReg(self, reg):

        if not self.ser.is_open:
            self.log('The port is not open!')
            return

        reg = hex(reg).replace('0x', '')
        ret = self._ask('read ' + reg)


        if ret.find('ERR') != -1 or ret == '':
            ret = 'ERROR'
            self.log('Read error!')
        else:

            ret = ret.split(' ')
            ret = ret[2]
            self.log('Read ' + ret + ' from 0x' + reg)
            #self.log('Read succ.')

        return ret



    def stmWriteReg(self, reg, val):

        if not self.ser.is_open:
            self.log('The port is not open!')
            return

        reg = hex(reg).replace('0x', '')
        val = hex(val).replace('0x', '')

        msg = self._ask('write ' + reg + ' ' + val)

        if msg.find('ERR') != -1:
            self.log('STM : Error during writing!')
        else:
            self.log('Write 0x' + val + ' to 0x' + reg)
            #self.log('STM : Writing successful')
