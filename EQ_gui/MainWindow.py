from PyQt5 import QtCore, QtGui, QtWidgets
from mw import Ui_MainWindow

from PyQt5.QtCore import *
from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import *
from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QSystemTrayIcon


from serStm import serStm

class MW(QtWidgets.QMainWindow, Ui_MainWindow):


    def __init__(self):
        super(MW, self).__init__()
        self.setupUi(self)
        self.setCentralWidget(self.cw)


        self.ser = serStm()

    def showErr(self, text):
        QtWidgets.QMessageBox.critical(self, 'Ошибка!', text)