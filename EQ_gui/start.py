from PyQt5 import QtWidgets
from MainWindow import MW
import sys

# --- Нормальный вывод ошибок
def log_uncaught_exceptions(ex_cls, ex, tb):
    text = '{}: {}:\n'.format(ex_cls.__name__, ex)
    import traceback
    text += ''.join(traceback.format_tb(tb))


    print('\n\n-------------------------------------------- \n')
    print(text)
    print('\n----------------------------------------------\n')


import sys
sys.excepthook = log_uncaught_exceptions


def start():
    app = QtWidgets.QApplication(sys.argv)
    mv = MW()
    mv.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    start()