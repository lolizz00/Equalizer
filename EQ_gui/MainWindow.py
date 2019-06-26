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

    def showMsg(self, text):
        QtWidgets.QMessageBox.information(self, 'Сообщение', text)

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
        self.writeDataPushButton.clicked.connect(self.writeDataPushButtonClicked)
        self.readDataPushButton.clicked.connect(self.readDataPushButtonClicked)
        self.clearAllPushButton.clicked.connect(self.clearAllPushButtonClicked)
        self.writeGenPushButton.clicked.connect(self.writeGenPushButtonClicked)
        self.readGenPushButton.clicked.connect(self.readGenPushButtonClicked)
        self.selectFileReadPushButton.clicked.connect(self.selectFileReadPushButtonClicked)
        self.selectFileSavePushButton.clicked.connect(self.selectFileSavePushButtonClicked)
        self.readFilePushButton.clicked.connect(self.readFilePushButtonClicked)
        self.saveFilePushButton.clicked.connect(self.saveFilePushButtonClicked)

    def writeLog(self, msg):
        old_text = self.logTextEdit.toPlainText()

        if old_text == '':
            self.logTextEdit.setText(msg)
        else:
            self.logTextEdit.setText(old_text + '\n' + msg)

    # ---


    def readFilePushButtonClicked(self):
        file = self.readFileLineEdit.text()

        if file == '':
            file = 'equalizerConfig.conf'
        else:
            file = file + '\equalizerConfig.conf'


        try:
            f = open(file, 'r')


            for line in f:
                line = line.split(' ')
                reg, val = int(line[0], 16), int(line[1], 16)
                self.ser.stmWriteReg(reg, val)

            f.close()

        except:
            self.showErr('Ошибка во время чтения или записи из файла!')
            return

        self.readDataPushButtonClicked()
        self.readGenPushButtonClicked()

        self.showMsg('Готово!')


    def saveFilePushButtonClicked(self):
        file = self.saveFileLineEdit.text()

        if file == '':
            file = 'equalizerConfig.conf'


        f = open(file, 'w')

        # -- сохраняем для регистров
        reg_n = 5

        table = \
        [
            int('0x0E', 16), int('0x15', 16), int('0x1C', 16), int('0x23', 16), \
            int('0x2B', 16), int('0x32', 16), int('0x39', 16), int('0x40', 16), \
        ]


        for i in range(len(table)):
            for j in range(reg_n):
                reg = table[i] + j
                val = self.ser.stmReadReg(reg)
                f.write(hex(reg) + ' ' + val + '\n')

        table = [0x01, 0x02, 0x08]
        for i in range(len(table)):
            val = self.ser.stmReadReg(table[i])
            f.write(hex(table[i]) + ' ' + val + '\n')

        f.close()

        self.writeLog('Saved!')
        self.showMsg('Готово!')

    def selectFileReadPushButtonClicked(self):
        self.readFileLineEdit.setText(QFileDialog.getExistingDirectory())

    def selectFileSavePushButtonClicked(self):
        self.saveFileLineEdit.setText(QFileDialog.getOpenFileName()[0])

    # ---

    def readGenPushButtonClicked(self):
        reg = []

        # ...

        reg_addr = [0x00, 0x01, 0x02, 0x08, 0x0A]


        for t in reg_addr:
            tmp = self.ser.stmReadReg(t)
            tmp = int(tmp, 16)
            reg.append(tmp)

        # Device Address Observation
        _reg = reg[0]
        if _reg & (1 << 2):
            self.READDONE_comboBox.setCurrentIndex(0)
        else:
            self.READDONE_comboBox.setCurrentIndex(1)
        _reg = _reg >> 3
        _reg = _reg & 0x7
        self.ADDR_lineEdit.setText(str(_reg))


        # PWDN CHx
        _reg = reg[1]
        arr = [self.PWDN0checkBox,  self.PWDN1checkBox, self.PWDN2checkBox, self.PWDN3checkBox, self.PWDN4checkBox, self.PWDN5checkBox, self.PWDN6checkBox, self.PWDN7checkBox]
        if reg == 0x00:
            for t in arr:
                t.setChecked(False)
        elif reg == 0xFF:
            for t in arr:
                t.setChecked(True)
        else:
            sch = 0
            for t in arr:
                arr[sch].setChecked(bool(_reg & (1 << sch)))
                sch = sch + 1


        # Override PRSNT, LPBK Control
        _reg = reg[2]

        if _reg & (1 << 7):
            self.OVERRXDET_comboBox.setCurrentIndex(0)
        else:
            self.OVERRXDET_comboBox.setCurrentIndex(1)

        if _reg & (1 << 6):
            self.RXDET_comboBox.setCurrentIndex(0)
        else:
            self.RXDET_comboBox.setCurrentIndex(1)
        tmp = _reg & 0x30
        tmp = tmp >> 4
        self.LPBK_comboBox.setCurrentIndex(tmp)
        if _reg & 1:
            self.OPRSNT_comboBox.setCurrentIndex(0)
        else:
            self.OPRSNT_comboBox.setCurrentIndex(1)

        # Override Pin Control
        _reg = reg[3]
        if _reg & (1 << 6):
            self.OSDTHcomboBox.setCurrentIndex(0)
        else:
            self.OSDTHcomboBox.setCurrentIndex(1)
        if _reg & (1 << 4):
            self.OIDLEcomboBox.setCurrentIndex(0)
        else:
            self.OIDLEcomboBox.setCurrentIndex(1)
        if _reg & (1 << 3):
            self.ORXDETcomboBox.setCurrentIndex(0)
        else:
            self.ORXDETcomboBox.setCurrentIndex(1)
        if _reg & (1 << 2):
            self.ORATEcomboBox.setCurrentIndex(0)
        else:
            self.ORATEcomboBox.setCurrentIndex(1)

        # Signal Detect Monitor
        _reg = reg[4]
        arr = [self.SDTH0checkBox, self.SDTH1checkBox, self.SDTH2checkBox, self.SDTH3checkBox, \
               self.SDTH4checkBox, self.SDTH5checkBox, self.SDTH6checkBox, self.SDTH7checkBox ]

        for i in range(len(arr)):
            arr[i].setChecked(_reg & (1 << i))

    def writeGenPushButtonClicked(self):

        addr = [0x01, 0x02, 0x08]
        vals = []

        # PWDN CHx
        arr = [self.PWDN0checkBox, self.PWDN1checkBox, self.PWDN2checkBox, self.PWDN3checkBox, self.PWDN4checkBox,
               self.PWDN5checkBox, self.PWDN6checkBox, self.PWDN7checkBox]
        tmp = 0x0
        for i in range(len(arr)):
            if arr[i].isChecked():
                tmp |= (1 << i)
        vals.append(tmp)

        # Override PRSNT, LPBK Control
        tmp = 0x0
        if self.OVERRXDET_comboBox.currentIndex() == 0:
            tmp |= (1 << 7)
        if  self.RXDET_comboBox.currentIndex() == 0:
            tmp |= (1 << 6)
        tmp |= self.LPBK_comboBox.currentIndex() << 4
        if self.OPRSNT_comboBox.currentIndex() == 0:
            tmp |= 1
        vals.append(tmp)

        # Override Pin Control
        tmp = 0x0
        if self.OSDTHcomboBox.currentIndex() == 0:
            tmp |= 1 << 6
        if self.OIDLEcomboBox.currentIndex() == 0:
            tmp |= 1 << 4
        if self.ORXDETcomboBox.currentIndex() == 0:
            tmp |= 1 << 3
        if self.ORATEcomboBox.currentIndex() == 0:
            tmp |= 1 << 2
        vals.append(tmp)
        # ---

        for i in range(len(addr)):
            self.ser.stmWriteReg(addr[i], vals[i])

    def clearAllPushButtonClicked(self):
        val = 1 << 6
        self.ser.stmWriteReg(0x07, val)

    def readDataPushButtonClicked(self):

        reg = []

        n = int(self.readDataComboBox.currentText())

        table = {
            0: int('0x0E', 16), 1: int('0x15', 16), 2: int('0x1C', 16), 3: int('0x23', 16), \
            4: int('0x2B', 16), 5: int('0x32', 16), 6: int('0x39', 16), 7: int('0x40', 16), \
            }

        for i in range(5):
            tmp = self.ser.stmReadReg(table[n] + i)
            tmp = int(tmp, 16)
            reg.append(tmp)

        #  --- IDLE, RXDET ---


        _reg = reg[0]

        if (1 << 5) & _reg:
            self.IDLE_AUTOcomboBox.setCurrentIndex(0)
        else:
            self.IDLE_AUTOcomboBox.setCurrentIndex(1)

        if (1 << 4) & _reg:
            self.IDLE_SELcomboBox.setCurrentIndex(0)
        else:
            self.IDLE_SELcomboBox.setCurrentIndex(1)

        tmp = (_reg & (0xE)) >> 1
        if tmp == int('0b00', 2):
            self.RXDETcomboBox.setCurrentIndex(0)
        elif tmp == int('0b01', 2):
            self.RXDETcomboBox.setCurrentIndex(1)
        elif tmp == int('0b10', 2):
            self.RXDETcomboBox.setCurrentIndex(2)
        elif tmp == int('0b11', 2):
            self.RXDETcomboBox.setCurrentIndex(3)


        # --- EQ ---

        _reg = reg[1]

        if _reg == 0x0:
            self.EQControlcomboBox.setCurrentIndex(0)
        elif _reg == 0x01:
            self.EQControlcomboBox.setCurrentIndex(1)
        elif _reg == 0x02:
            self.EQControlcomboBox.setCurrentIndex(2)
        elif _reg == 0x03:
            self.EQControlcomboBox.setCurrentIndex(3)
        elif _reg == 0x07:
            self.EQControlcomboBox.setCurrentIndex(4)
        elif _reg == 0x15:
            self.EQControlcomboBox.setCurrentIndex(5)
        elif _reg == 0x0B:
            self.EQControlcomboBox.setCurrentIndex(6)
        elif _reg == 0x0F:
            self.EQControlcomboBox.setCurrentIndex(7)
        elif _reg == 0x55:
            self.EQControlcomboBox.setCurrentIndex(8)
        elif _reg == 0x1F:
            self.EQControlcomboBox.setCurrentIndex(9)
        elif _reg == 0x2F:
            self.EQControlcomboBox.setCurrentIndex(10)
        elif _reg == 0x3F:
            self.EQControlcomboBox.setCurrentIndex(11)
        elif _reg == 0xAA:
            self.EQControlcomboBox.setCurrentIndex(12)
        elif _reg == 0x7F:
            self.EQControlcomboBox.setCurrentIndex(13)
        elif _reg == 0xBF:
            self.EQControlcomboBox.setCurrentIndex(14)
        elif _reg == 0xFF:
            self.EQControlcomboBox.setCurrentIndex(15)

        # --- VOD ---

        _reg = reg[2]

        if _reg & (1 << 7):
            self.SCPcomboBox.setCurrentIndex(0)
        else:
            self.SCPcomboBox.setCurrentIndex(1)

        if _reg & (1 << 6):
            self.RATE_SELcomboBox.setCurrentIndex(0)
        else:
            self.RATE_SELcomboBox.setCurrentIndex(1)

        tmp = _reg & 0x7

        if tmp == int('0b000', 2):
            self.VODCcomboBox.setCurrentIndex(0)
        elif tmp == int('0b001', 2):
            self.VODCcomboBox.setCurrentIndex(1)
        elif tmp == int('0b010', 2):
            self.VODCcomboBox.setCurrentIndex(2)
        elif tmp == int('0b011', 2):
            self.VODCcomboBox.setCurrentIndex(3)
        elif tmp == int('0b100', 2):
            self.VODCcomboBox.setCurrentIndex(4)
        elif tmp == int('0b101', 2):
            self.VODCcomboBox.setCurrentIndex(5)
        elif tmp == int('0b110', 2):
            self.VODCcomboBox.setCurrentIndex(6)
        elif tmp == int('0b111', 2):
            self.VODCcomboBox.setCurrentIndex(7)

        # --- DEM ---

        _reg = reg[3]
        tmp = _reg & 0x7
        if tmp == int('0b000', 2):
            self.DEMcomboBox.setCurrentIndex(0)
        elif tmp == int('0b001', 2):
            self.DEMcomboBox.setCurrentIndex(1)
        elif tmp == int('0b010', 2):
            self.DEMcomboBox.setCurrentIndex(2)
        elif tmp == int('0b011', 2):
            self.DEMcomboBox.setCurrentIndex(3)
        elif tmp == int('0b100', 2):
            self.DEMcomboBox.setCurrentIndex(4)
        elif tmp == int('0b101', 2):
            self.DEMcomboBox.setCurrentIndex(5)
        elif tmp == int('0b110', 2):
            self.DEMcomboBox.setCurrentIndex(6)
        elif tmp == int('0b111', 2):
            self.DEMcomboBox.setCurrentIndex(7)

        # --- IDLE Threshold ---

        _reg = reg[4]

        tmp = (_reg & 0xC) >> 2
        self.IDLEthacomboBox.setCurrentIndex(tmp)

        tmp = (_reg & 0x3)
        self.IDLEthdcomboBox.setCurrentIndex(tmp)

        self.writeLog('Finish!')

    def manWritePushButtonClicked(self):
        try:
            reg = int(self.manRegLineEdit.text(), 16)
            val = int(self.manValLineEdit.text(), 16)
        except:
            self.showErr('Неккоректный номер!')
            return

        self.ser.stmWriteReg(reg, val)

    def IDLERXDETtoBytes(self):

        res = 0

        if self.IDLE_AUTOcomboBox.currentIndex() == 0:
            res |= (1 << 5)

        if self.IDLE_SELcomboBox.currentIndex() == 0:
            res |= (1 << 4)

        if self.RXDETcomboBox.currentIndex() == 0:
            res |= ( int('0b00', 2) << 1)
        elif self.RXDETcomboBox.currentIndex() == 1:
            res |= (int('0b01', 2) << 1)
        elif self.RXDETcomboBox.currentIndex() == 2:
            res |= (int('0b10', 2) << 1)
        elif self.RXDETcomboBox.currentIndex() == 3:
            res |= (int('0b11', 2) << 1)

        return res

    def EQtoBytes(self):
        table = { \
                '2.1 | 3.7 | 4.9'   : int('0x00', 16), '3.4 | 5.8 | 7.9'    : int('0x01', 16), '4.8 | 7.7 | 9.9'     : int('0x02', 16), \
                '5.9 | 8.9 | 11.0'  : int('0x03', 16), '7.2 | 11.2 | 14.3'  : int('0x07', 16), '6.1 | 11.4 | 14.6'   : int('0x15', 16), \
                '8.8 |  13.5 | 17.0': int('0x0B', 16), '10.2 | 15.0 | 18.5' : int('0x0F', 16), '7.5 |  12.8 | 18.0 ' : int('0x55', 16), \
                '11.4 | 17.4 | 22.0': int('0x1F', 16), '13.0 | 19.7 | 22.4' : int('0x2F', 16), '14.2 | 21.1 | 25.8'  : int('0x3F', 16), \
                '13.8 | 21.7 | 27.4': int('0xAA', 16), '15.6 | 23.5 | 29.0' : int('0x7F', 16), '17.2 | 25.8 | 31.4'  : int('0xBF', 16), \
                '18.4 | 27.3 | 32.7': int('0xFF', 16)
            }

        res = 0

        res = table[self.EQControlcomboBox.currentText()]

        return res

    def VODtoBytes(self):

        res = 0

        if self.SCPcomboBox.currentIndex() == 0:
            res |= (1 << 7)

        if self.RATE_SELcomboBox.currentIndex() == 0:
            res |= (1 << 6)

        table = { \
                '0.7 V' : int('0b000', 2), '0.8 V' : int('0b001', 2), '0.9 V' : int('0b010', 2), \
                '1.0 V' : int('0b011', 2), '1.1 V' : int('0b100', 2), '1.2 V' : int('0b101', 2), \
                '1.3 V' : int('0b110', 2), '1.4 V' : int('0b111', 2) \
                 }

        res |= table[self.VODCcomboBox.currentText()]

        return res

    def DEMtoBytes(self):
        res = 0

        table = { \
                '0 dB' : int('0b000', 2),  '–1.5 dB' : int('0b001', 2),  '–3.5 dB' : int('0b010', 2), \
                '–5 dB': int('0b011', 2),  '–6 dB'   : int('0b100', 2),   '–8 dB'  : int('0b101', 2),
                '–9 dB': int('0b110', 2),  '–12 dB'  : int('0b111', 2), \
                }


        res = table[self.DEMcomboBox.currentText()]

        return  res

    def IDLEThetoBytes(self):
        res = 0


        tableTha = { '180 mVp-p' : int('0b00', 2), '160 mVp-p' : int('0b01', 2),'210 mVp-p' : int('0b10', 2),'190 mVp-p' : int('0b11', 2)  }

        tableThd = { '110 mVp-p' : int('0b00', 2), '100 mVp-p' : int('0b01', 2),'150 mVp-p' : int('0b10', 2),'130 mVp-p' : int('0b11', 2)  }

        res |= tableTha[self.IDLEthacomboBox.currentText()] << 2
        res |= tableThd[self.IDLEthdcomboBox.currentText()]

        return res

    def writeDataPushButtonClicked(self):

        data = []
        data.append(self.IDLERXDETtoBytes())
        data.append(self.EQtoBytes())
        data.append(self.VODtoBytes())
        data.append(self.DEMtoBytes())
        data.append(self.IDLEThetoBytes())

        # DBG
        #print(bin(regIDLERXDET))
        #print(bin(regEQ))
        #print(bin(regVOD))
        #print(bin(regDEM))
        #print(bin(regIDLETHE))


        table  = {
                    0 : int('0x0E', 16), 1 : int('0x15', 16), 2 : int('0x1C', 16), 3 : int('0x23', 16), \
                    4 : int('0x2B', 16), 5 : int('0x32', 16), 6 : int('0x39', 16), 7 : int('0x40', 16), \
                }


        targ = []
        if self.CH0checkBox.isChecked():
            targ.append(0)
        if self.CH1checkBox.isChecked():
            targ.append(1)
        if self.CH2checkBox.isChecked():
            targ.append(2)
        if self.CH3checkBox.isChecked():
            targ.append(3)
        if self.CH4checkBox.isChecked():
            targ.append(4)
        if self.CH5checkBox.isChecked():
            targ.append(5)
        if self.CH6checkBox.isChecked():
            targ.append(6)
        if self.CH7checkBox.isChecked():
            targ.append(7)


        for t in targ:
            addr = table[t]

            for i in range((len(data))):
                self.ser.stmWriteReg(addr + i, data[i])

        self.writeLog('Finish!')

    def manReadPushButtonClicked(self):
        reg = self.manRegLineEdit.text()

        try:
            reg = int(reg, 16)
        except:
            self.showErr('Неккоректный номер!')
            return

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


