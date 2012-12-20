#!/usr/bin/env python
#
# gHPSDR3 manager
#
# Andrea Montefusco IW0HDV
#

import os
import sys 
import time 
from PyQt4.QtCore import * 
from PyQt4.QtGui import * 
import subprocess
import json

default_cfg = {

   #
   # Hardware servers
   #
   'Hardware servers': {
      'perseus': {
          'startCommand': "perseus_server -d3 -s96000",
          'status':  { "No Perseus receiver(s) detected": 'abnormally ended',        
                       "Listening for TCP connections on port 11000": 'started',     
                       "start async input: STARTED": 'running',                      
                       "detach_receiver: Quitting...": 'stopped'         ,           
                       "perseus: read eeprom successful": 'loading.....'    ,        
                       "perseus: perseus_open": 'enumerating.....',                  
                       "perseus: re-enumerating usb devices": 're-enumerating.....', 
                       " user_data_callback": 'running'   ,                          
                       "perseus_stop_async_input": 'stopping async input',           
                       "perseus: closing device handle": 'closed'                    
                      }                                                              

       },
      'sdriq': {
          'startCommand': "sdriq-server -s 111111",
          'status':  { "No  HiqSDR hardware detected": 'abnormally ended',        
                       "Listening for TCP connections on port 11000": 'started',     
                       "asynch_input_thread STARTED": 'running',                      
                       "detach_receiver: ... done.": 'stopped'         ,           
                       "Capture from SDR-IQ seria": 'loading.....' 
                      }                                                              

       },

      'widget-server': {
          'startCommand': "widget-server",
          'status':  { "Cannot locate Ozy": 'abnormally ended',        
                       "Listening for TCP connections on port 11000": 'started',     
                      }                                                              
       },

      'hpsdr-server': {
          'startCommand': "hpsdr-server --dither off",
          'status':  { "Cannot locate Ozy": 'abnormally ended',        
                       "Listening for TCP connections on port 11000": 'started',     
                      }                                                              

      },

      'hiqsdr-server-96kS': {
          'startCommand': "hiqsdr-server -d3 -s96000 --ip 192.168.2.196",
          'status':  { "Cannot locate HiQSDR": 'abnormally ended',
                       "Listening for TCP connections on port 11000": 'started',
                      }

      },

      'hiqsdr-server-240kS': {
          'startCommand': "hiqsdr-server -d3 -s240000 --ip 192.168.2.196",
          'status':  { "Cannot locate HiQSDR": 'abnormally ended',
                       "Listening for TCP connections on port 11000": 'started',
                      }

      },

      "softrock pmsdr 48kS": {
          "startCommand": "softrock --si570 --qi --samplerate 48000",
          "status": {
              "softrock_set_sample_rate": "starting....",
              "Cannot open USB device": "USB device not found",
              "Segmentation fault": "abnormally ended",
              "Listening for TCP connections on port 11000": "started"
          }
      },
      "softrock-pmsdr-96kS": {
          "startCommand": "softrock --si570 --qi --samplerate 96000 ", 
          "status": {
              "softrock_set_sample_rate": "starting....",
              "Cannot open USB device": "USB device not found" ,
              "Segmentation fault": "abnormally ended", 
              "Listening for TCP connections on port 11000": "started"
          }
      }
   },

   #
   # dspserver
   #
   'DSP server': {
       'dttsp': {
          'startCommand': "dspserver",
          'status':  { "connect failed": 'abnormally ended',     
                       "gHPSDR": 'starting....',
                       "command bound to port": 'running'
                     }
       },
       'dttsp-ddc': {
          'startCommand': "dspserver --lo 0 --nocorrectiq",
          'status':  { "connect failed": 'abnormally ended',     
                       "gHPSDR": 'starting....',
                       "command bound to port": 'running'
                     }
       },
       'dttsp-ddc-delayed': {
          'startCommand': '/bin/bash -c "sleep 2 ; dspserver --lo 0 --nocorrectiq"',
          'status':  { "connect failed": 'abnormally ended',     
                       "gHPSDR": 'starting....',
                       "command bound to port": 'running'
                     }
       },
   },

   #
   # QtRadio
   #
   'GUI': {
        'QtRadioPerseus': {
           'startCommand': "QtRadio 127.0.0.1",
           'environment': "QT_RADIO_NO_LOCAL_AUDIO=1",
           'status':  { "connect failed": 'abnormally ended',     
                        "rigctl: Listening": 'starting....',
                        "Spectrum::setBandLimits": 'running',
                        "Remote disconnected": 'disconnected'
                      }
        },
        'QtRadio': {
           'startCommand': "QtRadio 127.0.0.1",
           'status':  { "connect failed": 'abnormally ended',     
                        "rigctl: Listening": 'starting....',
                        "Spectrum::setBandLimits": 'running',
                        "Remote disconnected": 'disconnected'
                      }
        },
   },

}

def loadConfig (fn):
    file = 0
    cfg = 0

    if os.path.isfile(fn):
       file = open(fn, 'r')
       if file:
           print "Reading config...."
           try:
               cfg = json.load(file)
               
           except Exception as inst:
               print type(inst)     # the exception instance
               print inst.args      # arguments stored in .args
               #print inst           # __str__ allows args to printed directly
               #x, y = inst          # __getitem__ allows args to be unpacked directly
               #print 'x =', x
               #print 'y =', y
               print "FATAL: syntax error reading configuration"
               print

               msgBox = QMessageBox ()
               msgBox.setIcon(QMessageBox.Warning)
               msgBox.setText("Error loading configuration");
               msgBox.setInformativeText("%s\n%s" %(type(inst), inst.args));
               msgBox.setStandardButtons(QMessageBox.Ok | QMessageBox.Discard);
               msgBox.setDefaultButton(QMessageBox.Ok);
               ret = msgBox.exec_();
               if ret == QMessageBox.Discard: sys.exit()

               cfg = default_cfg
       else:
           print "FATAL: unable to open configuration file: %s" % fn
           cfg = default_cfg
    else:
       file = open(fn, 'w')
       cfg = default_cfg
       json.dump(cfg,file, sort_keys=True, indent=4)

    file.close()
    return cfg

################## os.getcwd()##############
def get_path():
    return os.path.dirname(__file__)+'/'

class MainWin(QMainWindow):
    
    mtbar = [
              {    'name':      'sdriq',
                   'tooltip':  'SDR-IQ',
                   'shortcut': 'Ctrl+Q',
                   'icon':     'p24.png' 
              },
              {    'name':      'perseus',
                   'tooltip':  'Perseus',
                   'shortcut': 'Ctrl+P',
                   'icon':     'p24.png' 
              },
              {    'name':     'softrock',
                   'tooltip':  'Softrock',
                   'shortcut': 'Ctrl+S',
                   'icon':     'sr24.png' 
              },
              {    'name':     'hpsdr',
                   'tooltip':  'HPSDR',
                   'shortcut': 'Ctrl+H',
                   'icon':     'hpsdr24.png' 
              },
              {    'name':     'hiqsdr',
                   'tooltip':  'HiQSDR',
                   'shortcut': 'Ctrl+N',
                   'icon':     'hiqsdr.png' 
              }
    ]

    def __init__(self, qApp, cfg):
        super(MainWin, self).__init__()
        
        self.initUI(qApp, cfg)
        
    def initUI(self, qApp, cfg):               

        # find path
        print get_path()
        self.icon_path = get_path()
        
        self.toolbar = self.addToolBar('MainToolbar')
        #
        # create the icon for closing
        #
        exitAction = QAction(QIcon(self.icon_path + 'exit24.png'), 'Exit', self)
        exitAction.setShortcut('Ctrl+Q')
        exitAction.triggered.connect(self.close)
        
        self.toolbar.addAction(exitAction)


        stopAction = QAction(QIcon(self.icon_path + 'stop24.png'), 'Stop', self)
        stopAction.setShortcut('Ctrl+K')
        self.connect(stopAction, SIGNAL("triggered()"), self.groupStop )

        self.toolbar.addAction(stopAction)
        
        
        #
        # iterate on main task bar and create the macro button (typically one for each setup)
        #
        for x in MainWin.mtbar:
            xAction = QAction(QIcon(self.icon_path + x['icon']), x['tooltip'], self)
            xAction.setShortcut(x['shortcut'])
            self.connect(xAction, SIGNAL("triggered()"), lambda who=x['name']: self.groupStart(who) )
            self.toolbar.addAction(xAction)
        

        #
        # create and show the central widget, a multi tab window
        #       
        self.w = QTabWidget(self)

        keys = cfg.keys()
        keys.sort()
        self.tss = []
        self.tss_d = {}
        for k in keys:
            print k
            ts = TaskStarter(k, cfg[k])
            i = self.w.addTab (ts,  k)
            self.w.tabBar().setTabTextColor (i, QColor(Qt.red))
            #if cfg[k].has_key('tooltip') : w.tabBar().setTabToolTip (0, "hardware servers")
            ts.setTabIndex(i)
            ts.setParentTabBar(self.w.tabBar())
            self.w.tabBar().setTabToolTip (i, k)

            self.tss.append(ts)
            self.tss_d[k] = ts

        self.setCentralWidget(self.w) 

    def closeEvent (self, event):
        print "CLOSE in MainWin"
        r = QMessageBox.question(self, 'Message',
                                 "Are you sure to quit?", 
                                 QMessageBox.Yes | QMessageBox.No, 
                                 QMessageBox.No)
        if r == QMessageBox.Yes:
            for x in self.tss:
               x.close() 
            event.accept()
        else:
            event.ignore()        

    def groupStop (self):
        print "TTT:"
        #self.running_group = 'softrock'
        self.tss_d['GUI'].run_command()
        self.tss_d['DSP server'].run_command()
        self.tss_d['Hardware servers'].run_command()


    def groupStart (self, x):
        print "SSS: %s" % x

        if x == 'softrock':
            self.tss_d['Hardware servers'].setCurrent('softrock pmsdr 48kS')
            self.tss_d['Hardware servers'].run_command()

            self.tss_d['GUI'].setCurrent('QtRadio')
            self.tss_d['GUI'].run_command()

            self.tss_d['DSP server'].setCurrent('dttsp')
            self.tss_d['DSP server'].run_command()

        if x == 'perseus':
            self.tss_d['Hardware servers'].setCurrent('perseus')
            self.tss_d['Hardware servers'].run_command()

            self.tss_d['GUI'].setCurrent('QtRadio')
            self.tss_d['GUI'].run_command()

            self.tss_d['DSP server'].setCurrent('dttsp-ddc')
            self.tss_d['DSP server'].run_command()

        if x == 'hpsdr':
            self.tss_d['Hardware servers'].setCurrent('hpsdr-server')
            self.tss_d['Hardware servers'].run_command()

            self.tss_d['GUI'].setCurrent('QtRadio')
            self.tss_d['GUI'].run_command()

            self.tss_d['DSP server'].setCurrent('dttsp')
            self.tss_d['DSP server'].run_command()

        if x == 'sdriq':
            self.tss_d['Hardware servers'].setCurrent('sdriq')
            self.tss_d['Hardware servers'].run_command()

            self.tss_d['GUI'].setCurrent('QtRadio')
            self.tss_d['GUI'].run_command()

            self.tss_d['DSP server'].setCurrent('dttsp-ddc-delayed')
            self.tss_d['DSP server'].run_command()

        if x == 'hiqsdr':
            self.tss_d['Hardware servers'].setCurrent('hiqsdr-server-96kS')
            self.tss_d['Hardware servers'].run_command()

            self.tss_d['GUI'].setCurrent('QtRadio')
            self.tss_d['GUI'].run_command()

            self.tss_d['DSP server'].setCurrent('dttsp-ddc')
            self.tss_d['DSP server'].run_command()



def main(argv): 

    app = QApplication(argv) 

    cfg = loadConfig ( "ghpsdr3_mgr.conf" )

    mw = MainWin(qApp, cfg)



    mw.setWindowTitle('gHPSDR3')
    mw.resize (1024, 640)
    mw.show() 

    rc = app.exec_()
    sys.exit(rc)






class TaskStarter(QWidget):
    def __init__ (self, name, hw_srv):
        QWidget.__init__(self) 

        # creates the basic gui elements
        self.combo = QComboBox(self)
        self.te = QTextEdit(self)
        self.te.setReadOnly(True)
        self.te.setStyleSheet ( "background-color: DarkBlue; color: Yellow; " 
                                'font-family: "Fixed" ;'
                                "font-size: 10px" 
        )
        self.msg = QLabel("never started")
        self.msg.setAlignment(Qt.AlignHCenter)
        self.startPB = QPushButton("START")

        # put the gui elements into a suitable layout
        layout = QVBoxLayout(self)
        layout.addWidget(self.msg)
        layout.addWidget(self.startPB)
        layout.addWidget(self.combo)
        layout.addWidget(self.te)
        self.setLayout(layout) 

        # process object, useful to run child
        self.process = QProcess()

        # create event connection
        self.connect(self.startPB, SIGNAL("clicked()"), self.run_command)
        self.connect(self.process, SIGNAL("readyReadStandardOutput()"), self.readOutput)
        self.connect(self.process, SIGNAL("readyReadStandardError()"), self.readErrors)
        self.connect(self.process, SIGNAL("finished(int, QProcess::ExitStatus)"), self.processFinished)
        self.connect(self.combo, SIGNAL("currentIndexChanged(QString)"), self.changeHw)

        # initialize internal state
        self.status = 'STOP'
        self.hw = hw_srv


        self.combo_d = {} 
        i = 0
        keys = self.hw.keys()
        keys.sort()
        for k in keys:
            self.combo.addItem(k)
            print k
            self.combo_d[k] = i
            i = i +1


    def close (self):
        print "closing %s...." % str(self.toolTip())
        if self.isRunning(): 
           print "Terminating....."
           self.process.terminate()
           ###print "Killing....."
           ###self.process.kill()        
        return True

    def setTabIndex(self, i):
        self.tab_index = i

    def setParentTabBar (self, w):
        self.tab_widget = w


    def changeHw (self, txt):
        s=str(txt)
        print "[%s]" % s
        self.hw_sel = self.hw[s]  # current hardware selected


    def setCurrent (self, name):
        print "................ %s" % name
        self.combo.setCurrentIndex (self.combo_d[name])


    def setStatusStop (self, msg=''):
        self.status = 'STOP'
        self.tab_widget.setTabTextColor (self.tab_index, QColor(Qt.red))
        self.startPB.setText("START");
        if len(msg)==0: 
           self.msg.setText("stopped")
        else:
            self.msg.setText(msg)
        self.combo.setEnabled(True)


    def setStatusRun (self):
        self.status = 'RUN'
        self.tab_widget.setTabTextColor (self.tab_index, QColor(Qt.blue))
        self.startPB.setText("STOP");
        self.combo.setEnabled(False)

    def isRunning (self):
        return self.status == 'RUN'



    def readOutput(self):
        line = QString(self.process.readLine())
        #print line

        sl = line.split("\n")
        for x in sl: self.process_line(x)


    def readErrors(self):
        line = QString(self.process.readAllStandardError())
        #print line

        sl = line.split("\n")
        for x in sl: self.process_line(x)


    def processFinished (self, exitCode, exitStatus):
        print "EXIT.... %d %s" % (exitCode, exitStatus)
        self.setStatusStop("process unexpectedly ended")

    def closeEvent(self, event):

        
        reply = QtGui.QMessageBox.question(self, 'Message',
            "Are you sure to quit?", QtGui.QMessageBox.Yes | 
            QtGui.QMessageBox.No, QtGui.QMessageBox.No)

        if reply == QtGui.QMessageBox.Yes:
            print "CLOSE...."
            self.disconnect(self.startPB, SIGNAL("clicked()"), self.run_command)
            self.disconnect(self.process, SIGNAL("readyReadStandardOutput()"), self.readOutput)
            self.disconnect(self.process, SIGNAL("readyReadStandardError()"), self.readErrors)
            self.disconnect(self.process, SIGNAL("finished(int, QProcess::ExitStatus)"), self.processFinished)
            self.disconnect(self.combo, SIGNAL("currentIndexChanged(QString)"), self.changeHw)

            if self.isRunning(): 
               print "Killing....."
               self.process.kill()        

            event.accept()
        else:
            event.ignore()        



    def run_command(self):

        if self.isRunning():
            print "STOPPING......"
            if self.isRunning(): 
               print "Killing....."
               self.process.kill()      
            self.setStatusStop()
            self.te.moveCursor(QTextCursor.End)
            self.te.insertPlainText( "===================================================\n" )

        else:
            self.te.clear()
            command = self.hw_sel['startCommand']
            print ">>>>>>>>>> %s" % command

            li = command.split(" ")
            #self.process.closeStdin()
            #print li[0]
            #par = " ".join(map(str,li[1:] ))
            #print par

            # setup the environment if so specified
            self.process.setEnvironment([])
            env = QProcess.systemEnvironment();
            if self.hw_sel.has_key('environment'): 
                 env.append(QString(self.hw_sel['environment']))
   
            # start child
            self.process.setEnvironment(env)
            self.process.start(command)

            self.setStatusRun()


    def process_line(self, linex):
        if len(linex):

            #print "[%s] %d\n" % (linex, len(linex) )

            self.te.moveCursor(QTextCursor.End)
            self.te.insertPlainText( linex+"\n" )

            current_hw = self.combo.currentText()

            ##print self.st
            for k in self.hw_sel['status'].keys():
                ##print "****[%s]\n" % k
                if linex.contains(k):
                    self.msg.setText(self.hw_sel['status'][k])


#    def terminate(self):
#        self.emit(SIGNAL("kill"), self.process)





if __name__ == "__main__": 
    main(sys.argv)

