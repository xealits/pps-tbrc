#!/usr/bin/python

from sys import argv
import pygtk
pygtk.require('2.0')
import gtk, glib, gobject
import re
import time, signal
from SocketHandler import SocketHandler
from Plot import LinePlot

class DAQgui:
  exc_rgx = re.compile(r'\[(.*)\] === (.*)\ === (.*)')
  client_rgx = re.compile(r'(.*)\ \(type (.*)\)')

  def __init__(self):
    self.socket_handler = None
    self.client_id = -1
    self.acquisition_started = False
    self.daq_loop_launched = False
    self.current_run_id = -1
    self.previous_burst_time = -1.
    self.trigger_rate_data = [[0, 0.]]
    self.time_start = time.time()
    self.tot_trigger_data = [[self.time_start, 0]]
    self.dqm_enabled = True
    self.dqm_updated_plots = []
    self.dqm_updated_plots_images = []

    self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    #self.window.maximize()
    self.window.set_title("PPS Run control")
    self.window.set_border_width(20)

    main = gtk.VBox(False, 10)
    top = gtk.HBox(False, 10)
    bottom = gtk.HBox(False, 10)
    buttons = gtk.VBox(False, 5)
    run = gtk.VBox(False, 5)
    top.pack_start(buttons, False)
    buttons.show()
    top.pack_start(run, False)
    run.show()
    main.pack_start(top, False)
    self.dqm_frame = gtk.HBox(False)
    main.pack_start(bottom, False)
    main.pack_start(self.dqm_frame, False)
    top.show()
    bottom.show()
    self.dqm_frame.show()

    self.log_frame = gtk.ScrolledWindow()
    self.log_view = gtk.TextView()
    self.log_view.set_editable(False)
    self.log_frame.set_size_request(200, 400)
    #self.log = self.log_view.get_buffer()
    self.log = gtk.TextBuffer()
    self.log_view.set_buffer(self.log)
    bottom.pack_start(self.log_frame)
    self.log_frame.add(self.log_view)
    self.log_frame.show()
    self.log_view.show()

    #self.stat_frame = gtk.ScrolledWindow()
    self.stat_frame = gtk.VBox(False)
    self.stat_frame.set_size_request(50, 400)
    bottom.pack_start(self.stat_frame)

    self.tot_trigger = gtk.Label()
    self.stat_frame.pack_start(self.tot_trigger)
    self.tot_trigger.set_markup('Triggers number: ###')
    self.tot_trigger.set_alignment(0, 0)
    self.tot_trigger.show()

    self.trigger_rate = gtk.Label()
    self.stat_frame.pack_start(self.trigger_rate)
    self.trigger_rate.set_markup('Trigger rate: ###')
    self.trigger_rate.set_alignment(0, 0)
    self.trigger_rate.show()

    self.hv_imon0 = gtk.Label()
    self.stat_frame.pack_start(self.hv_imon0)
    self.hv_imon0.set_markup('I(GasToF):###')
    self.hv_imon0.set_alignment(0, 0)
    self.hv_imon0.show()

    self.hv_vmon0 = gtk.Label()
    self.stat_frame.pack_start(self.hv_vmon0)
    self.hv_vmon0.set_markup('V(GasToF):###')
    self.hv_vmon0.set_alignment(0, 0)
    self.hv_vmon0.show()

    self.hv_imon1 = gtk.Label()
    self.stat_frame.pack_start(self.hv_imon1)
    self.hv_imon1.set_markup('I(reference timing):###')
    self.hv_imon1.set_alignment(0, 0)
    self.hv_imon1.show()

    self.hv_vmon1 = gtk.Label()
    self.stat_frame.pack_start(self.hv_vmon1)
    self.hv_vmon1.set_markup('V(reference timing):###')
    self.hv_vmon1.set_alignment(0, 0)
    self.hv_vmon1.show()
    
    self.plots_frame = gtk.HBox(False)
    self.stat_frame.pack_start(self.plots_frame)
    self.trigger_rate_plot = LinePlot()
    self.trigger_rate_plot.set_size_request(-1, 300)
    self.plots_frame.pack_start(self.trigger_rate_plot)
    self.trigger_rate_plot.set_data(self.trigger_rate_data, 'Trigger rate')
    self.trigger_rate_plot.show()

    self.tot_trigger_plot = LinePlot()
    self.plots_frame.pack_start(self.tot_trigger_plot)
    self.tot_trigger_plot.set_data(self.trigger_rate_data, 'Triggers number')
    self.plots_frame.set_size_request(-1, 300)
    self.tot_trigger_plot.show()

    #self.plots_frame.show()
    self.stat_frame.show()

    self.window.connect('destroy', self.Close)

    self.bind_button = gtk.Button("Bind")
    self.bind_button.connect('clicked', self.Bind)
    self.bind_button.set_size_request(100, -1)
    self.bind_button.set_tooltip_text("Bind this GUI instance to the main controller")
    self.bind_button.set_sensitive(True)
    buttons.pack_start(self.bind_button, False, False)
    self.bind_button.show()

    self.unbind_button = gtk.Button("Unbind")
    self.unbind_button.connect('clicked', self.Unbind)
    self.unbind_button.set_tooltip_text("Disconnect this instance from the main controller.\nIf any run is started it will be left as it is!")
    self.unbind_button.set_sensitive(False)
    buttons.pack_start(self.unbind_button, False)
    self.unbind_button.show()

    self.exit_button = gtk.Button("Quit")
    self.exit_button.connect('clicked', self.Close)
    self.exit_button.set_tooltip_text("Quit and disconnect this instance from the main controller")
    buttons.pack_start(self.exit_button, False)
    self.exit_button.show()

    self.socket_id = gtk.Label()
    self.socket_id.set_tooltip_text("Identifier of this instance to the socket")
    buttons.pack_start(self.socket_id, False)
    self.socket_id.set_markup('########')
    self.socket_id.show()

    self.start_button = gtk.Button("Start")
    self.start_button.connect('clicked', self.StartAcquisition)
    self.start_button.set_tooltip_text("Start a new run with the configuration provided in $PPS_PATH/config/config.xml")
    #self.start_button.modify_bg(gtk.STATE_NORMAL, gtk.gdk.Color(0, 1, 0))
    self.start_button.set_size_request(120, 60)
    self.start_button.set_sensitive(False)
    run.pack_start(self.start_button, False, False)
    self.start_button.show()

    self.stop_button = gtk.Button("Stop")
    self.stop_button.connect('clicked', self.StopAcquisition)
    self.stop_button.set_size_request(120, 30)
    self.stop_button.set_sensitive(False)
    run.pack_start(self.stop_button, False, False)
    self.stop_button.show()

    self.run_id = gtk.Label()
    run.pack_start(self.run_id, False)
    self.run_id.set_markup('Run id: ###')
    self.run_id.show()

    #self.list_clients = gtk.ListStore(int, int)
    self.list_clients = gtk.Label()
    self.list_clients.set_size_request(-1, 120)
    top.pack_start(self.list_clients, True)
    self.list_clients.show()

    self.status_bar = gtk.Statusbar()
    main.pack_start(self.status_bar)
    self.status_bar.show()

    #self.window.maximize()
    self.window.set_default_size(1024, 668)
    self.window.add(main)
    main.show()
    self.window.show()
 
  def Bind(self, widget, data=None):
    if self.socket_handler:
      print "Warning: Socket already bound to master"
      return
    try:
      self.socket_handler = SocketHandler('localhost', 1987) 
      self.socket_handler.Handshake(5) # 5=DAQ
      self.bind_button.set_sensitive(False)
      self.unbind_button.set_sensitive(True)
      self.start_button.set_sensitive(not self.acquisition_started)
      self.stop_button.set_sensitive(self.acquisition_started)
      self.client_id = int(self.socket_handler.GetClientId())

      self.socket_id.set_markup('Client id: <b>%d</b>' % self.client_id)
      self.socket_handler.Send('GET_RUN_NUMBER', self.client_id)
      res = self.socket_handler.Receive('RUN_NUMBER')
      if res:
        self.current_run_id = int(res[1])
        self.run_id.set_markup('Run id: <b>%d</b>' % self.current_run_id)

      glib.timeout_add(1000, self.Update)
      self.Log('Client connected with id: %d' % self.client_id)
      self.Update()
      if self.acquisition_started and not self.daq_loop_launched:
        #print "Launching the acquisition monitor loop."
        self.DAQLoop()
    except SocketHandler.SocketError:
      print "Failed to bind!"
      return
 
  def Unbind(self, widget, data=None):
    if self.socket_handler:
      print "Requested to unbind from master"
      if not self.socket_handler.Disconnect():
        print 'forcing the stop'
        self.socket_handler = None
      print 'socket handler deleted'
      self.socket_id.set_markup('########')
      self.bind_button.set_sensitive(True)
      self.unbind_button.set_sensitive(False)
      self.start_button.set_sensitive(False)
      self.stop_button.set_sensitive(False)
      self.socket_handler = None
      self.client_id = -1

  def Close(self, widget, data=None):
    self.Unbind(self)
    gtk.main_quit()

  def Update(self, source=None, condition=None):
    if not self.socket_handler: return False
    #print "Getting clients..."
    self.socket_handler.Send('GET_CLIENTS', self.client_id)
    rcv = self.socket_handler.Receive()
    if len(rcv)<2: return True
    if rcv[0]=='CLIENTS_LIST':
      if ';' not in rcv[1]: return True
      clients_list = []
      for client in rcv[1].split(';'):
        try: client_id, client_type = [int(v) for v in re.split(self.client_rgx, client)[1:3]]
        except ValueError:
          print "Wrong value for client list:", client
          return True
        clients_list.append({'id': client_id, 'type': client_type, 'me': (client_id==self.client_id), 'web': (client_type==2)})
        if client_type==3: self.acquisition_started = True
      self.UpdateClientsList(clients_list)
    self.start_button.set_sensitive(not self.acquisition_started)
    self.stop_button.set_sensitive(self.acquisition_started)
    if self.acquisition_started and not self.daq_loop_launched:
      #print "Launching the acquisition monitor loop."
      self.DAQLoop()
    return True

  def DAQLoop(self):
    gobject.io_add_watch(self.socket_handler, gobject.IO_IN, self.ReceiveDAQMasterMessages)

  def ReceiveDAQMasterMessages(self, source, condition):
    if not self.socket_handler: return False
    if not self.acquisition_started: return False
    if not self.daq_loop_launched: self.daq_loop_launched = True
    rcv = source.Receive()
    print 'received', rcv
    if rcv[0]=='EXCEPTION':
      type, func, log = re.split(self.exc_rgx, rcv[1])[1:4]
      self.LogException(type, func, log)
    elif rcv[0]=='RUN_NUMBER':
      self.current_run_id = int(rcv[1])
      self.run_id.set_markup('Run id: <b>%d</b>' % self.current_run_id)
      self.dqm_updated_plots = []
      self.dqm_updated_plots_images = []
    elif rcv[0]=='NUM_TRIGGERS':
      #return False
      burst_id, num_trig = [int(a) for a in rcv[1].split(':')]
      self.tot_trigger.set_markup('Triggers number: <b>%d</b>' % num_trig)
      now = time.time()
      self.tot_trigger_data.append([(now-self.time_start)*1e6, num_trig])
      #self.tot_trigger_plot.set_data(self.tot_trigger_data)
      if self.previous_burst_time>0:
        rate = num_trig/(now-self.previous_burst_time)/1000.
        self.trigger_rate.set_markup('Trigger rate: <b>%.2f kHz</b>' % round(rate, 2))
        has_data = False
        i = 0
        #print 'aaa',self.trigger_rate_data
        for data in self.trigger_rate_data:
          if data[0]==burst_id:
            self.trigger_rate_data[i][1] = rate
            has_data = True
            break
          i += 1
        if not has_data:
          self.trigger_rate_data.append([burst_id, rate])
        #print now, "trigger rate: ", rate
        #self.trigger_rate_plot.set_data(self.trigger_rate_data, 'Trigger rate (kHz)')
      self.previous_burst_time = now
      return True
    elif rcv[0]=='HV_STATUS':
      channel, stat = rcv[1].split(':')
      status, imon, vmon = [int(a) for a in stat.split(',')]
      if channel=='0':
        self.hv_imon0.set_markup('I(GasToF): <b>%d uA</b>' % imon)
        self.hv_vmon0.set_markup('V(GasToF): <b>%d V</b>' % vmon)
      elif channel=='3':
        self.hv_imon1.set_markup('I(reference timing): <b>%d uA</b>' % imon)
        self.hv_vmon1.set_markup('V(reference timing): <b>%d V</b>' % vmon)
      return True
    elif rcv[0]=='UPDATED_DQM_PLOT':
      return True
      if not self.dqm_enabled: return True
      i = 0
      for p in self.dqm_updated_plots:
        if p==rcv[1] or rcv[1][0:15] in p:
          if p!=rcv[1]: self.dqm_updated_plots[i] = rcv[1]
          try:
            pixbuf = gtk.gdk.pixbuf_new_from_file_at_size('/tmp/%s.png' % (rcv[1]), 200, 200)
            self.dqm_updated_plots_images[i].set_from_pixbuf(pixbuf)
          except glib.GError:
            return True
          return True
        i += 1
      self.dqm_updated_plots.append(rcv[1])
      img = gtk.Image()
      try:
        pixbuf = gtk.gdk.pixbuf_new_from_file_at_size('/tmp/%s.png' % (rcv[1]), 200, 200)
        img.set_from_pixbuf(pixbuf)
        self.dqm_frame.pack_start(img)
        self.dqm_updated_plots_images.append(img)
        img.show()
      except glib.GError:
        return True
    return True

  def UpdateClientsList(self, clients_list):
    #for client in clients_list:
    #  self.list_clients.append([client['id'], client['type']])
    #columns = ["Id", "Type"]
    #view = gtk.TreeView(model=self.list_clients)
    #idx = 0
    #for c in columns:
    #  col = gtk.TreeViewColumn(c, gtk.CellRendererText(), text=idx)
    #  view.append_column(col)
    #  idx += 1
    out = ""
    for c in clients_list:
      #out += "Client %d, type %d (%s)\n" % (c['id'], c['type'], self.GetClientTypeName(c['type']))
      out += "<b>%s</b> (%d)  " % (self.GetClientTypeName(c['type']), c['id'])
    self.list_clients.set_markup(out)

  def GetClientTypeName(self, type):
    if type==-1: return "INVALID"
    elif type==0: return "MASTER"
    elif type==1: return "WEB_CLIENT"
    elif type==2: return "TERM_CLIENT"
    elif type==3: return "DAQ"
    elif type==4: return "DQM"
    elif type==5: return "GUI"

  def StartAcquisition(self, widget, data=None):
    print "Requested to start acquisition"
    self.socket_handler.Send('START_ACQUISITION', self.client_id)
    rcv = self.socket_handler.Receive('ACQUISITION_STARTED')
    self.acquisition_started = True
    self.start_button.set_sensitive(not self.acquisition_started)
    self.stop_button.set_sensitive(self.acquisition_started)

  def StopAcquisition(self, widget, data=None):
    print "Requested to stop acquisition"
    self.socket_handler.Send('STOP_ACQUISITION', self.client_id)
    rcv = self.socket_handler.Receive('ACQUISITION_STOPPED')
    self.acquisition_started = False
    self.start_button.set_sensitive(not self.acquisition_started)
    self.stop_button.set_sensitive(self.acquisition_started)

  def Log(self, data=None):
    self.log.insert_at_cursor(data+'\n')
    while gtk.events_pending():
      gtk.main_iteration()

  def LogException(self, code, func, message):
    str = "%s" % (message)
    self.Log(str)

  def main(self):
    gtk.main()

if __name__=='__main__':
  signal.signal(signal.SIGINT, signal.SIG_DFL)
  gui = DAQgui()
  gui.main()
