from PyQt5 import QtCore, QtGui, QtWidgets
from mw import Ui_MainWindow

from PyQt5.QtCore import *
from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import *
from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QSystemTrayIcon

import re
import serial.tools.list_ports

from serStm import serStm

class MW(QtWidgets.QMainWindow, Ui_MainWindow):


    def __init__(self):
        super(MW, self).__init__()
        self.setupUi(self)
        self.setCentralWidget(self.cw)
        self.initSlots()

        self.ser = serStm()
        self.ser.log_signal.connect(self.writeLog)

        # DBG
        self.ser.connect('COM10')

    def showErr(self, text):
        QtWidgets.QMessageBox.critical(self, 'Ошибка!', text)

    def initSlots(self):
        self.refrPortPushButton.clicked.connect(self.refrPortPushButtonClicked)
        self.clearLogPushButton.clicked.connect(self.clearLogPushButtonClicked)
        self.openPortPushButton.clicked.connect(self.openPortPushButtonClicked)
        self.closePortPushButton.clicked.connect(self.closePortPushButtonClicked)
        self.manResetPushButton.clicked.connect(self.manResetPushButtonClicked)
        self.manConnPushButton.clicked.connect(self.manConnPushButtonClicked)
        self.manReadPushButton.clicked.connect(self.manReadPushButtonClicked)
        self.manWritePushButton.clicked.connect(self.manWritePushButtonClicked)

    def writeLog(self, msg):
        old_text = self.logTextEdit.toPlainText()

        if old_text == '':
            self.logTextEdit.setText(msg)
        else:
            self.logTextEdit.setText(old_text + '\n' + msg)



    # ---

    def manWritePushButtonClicked(self):
        reg = int(self.manRegLineEdit.text(), 16)
        val = int(self.manValLineEdit.text(), 16)

        self.ser.stmWriteReg(reg, val)

    def manReadPushButtonClicked(self):
        reg = self.manRegLineEdit.text()
        reg = int(reg, 16)
        reg = self.ser.stmReadReg(reg)
        self.manValLineEdit.setText(reg)

    def manConnPushButtonClicked(self):
        self.ser.stmConn()

    def manResetPushButtonClicked(self):
        self.ser.stmReset()

    def closePortPushButtonClicked(self):
        self.ser.disconnect()

    def openPortPushButtonClicked(self):
        port = self.portComboBox.currentText()
        self.ser.connect(port)

    def clearLogPushButtonClicked(self):
        self.logTextEdit.setText('')

    def refrPortPushButtonClicked(self):
        prt = list(serial.tools.list_ports.comports())

        sch = 1
        self.writeLog('Описание доступных портов:')
        for t in prt:
            self.writeLog(str(sch) + ' : '  + t.description)
            self.portComboBox.addItem(t.device)
            sch = sch + 1


