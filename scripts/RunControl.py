#!/usr/bin/python

from sys import argv
import pygtk
pygtk.require('2.0')
import gtk, glib
import re
from SocketHandler import SocketHandler

class DAQgui:
  def __init__(self):
    self.socket_handler = None
    self.client_id = -1
    self.acquisition_started = False
    self.current_run_id = -1

    self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    #self.window.maximize()
    self.window.set_title("PPS Run control")
    self.window.set_border_width(20)

    main = gtk.VBox(False, 10)
    top = gtk.HBox(False, 10)
    bottom = gtk.HBox(False)
    buttons = gtk.VBox(False, 5)
    run = gtk.VBox(False, 5)
    top.pack_start(buttons, False)
    buttons.show()
    top.pack_start(run, False)
    run.show()
    main.pack_start(top, False)
    main.pack_start(bottom, False)
    top.show()
    bottom.show()

    self.log_frame = gtk.ScrolledWindow()
    self.log_view = gtk.TextView()
    self.log_frame.set_size_request(50, 500)
    #self.log = self.log_view.get_buffer()
    self.log = gtk.TextBuffer()
    self.log_view.set_buffer(self.log)
    bottom.pack_start(self.log_frame)
    self.log_frame.add(self.log_view)
    self.log_frame.show()
    self.log_view.show()

    self.dqm_frame = gtk.ScrolledWindow()
    bottom.pack_start(self.dqm_frame)
    self.dqm_frame.show()

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
    self.unbind_button.set_sensitive(False)
    buttons.pack_start(self.unbind_button, False)
    self.unbind_button.show()

    self.exit_button = gtk.Button("Quit")
    self.exit_button.connect('clicked', self.Close)
    buttons.pack_start(self.exit_button, False)
    self.exit_button.show()

    self.socket_id = gtk.Label()
    buttons.pack_start(self.socket_id, False)
    self.socket_id.set_markup('########')
    self.socket_id.show()

    self.start_button = gtk.Button("Start")
    self.start_button.connect('clicked', self.StartAcquisition)
    self.start_button.modify_bg(gtk.STATE_NORMAL, gtk.gdk.Color(0, 1, 0))
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

    self.list_clients_frame = gtk.ScrolledWindow()
    self.list_clients_frame.set_size_request(-1, 150)
    self.list_clients = gtk.TextView()
    self.clients = self.list_clients.get_buffer()
    top.pack_start(self.list_clients_frame, True)
    self.list_clients_frame.add(self.list_clients)
    self.list_clients_frame.show()
    self.list_clients.show()

    self.status_bar = gtk.Statusbar()
    main.pack_start(self.status_bar)
    self.status_bar.show()

    self.window.maximize()
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

      glib.timeout_add(1000, self.GetClients)
      glib.timeout_add(1000, self.GetExceptions)
      self.Log('Client connected with id: %d' % self.client_id)
    except SocketHandler.SocketError:
      print "Failed to bind!"
      return
 
  def Unbind(self, widget, data=None):
    if self.socket_handler:
      self.socket_handler.Disconnect()
      self.socket_id.set_markup('########')
      self.bind_button.set_sensitive(True)
      self.unbind_button.set_sensitive(False)
      self.start_button.set_sensitive(False)
      self.stop_button.set_sensitive(False)
      self.socket_handler = None
      self.client_id = -1
      print "disconnect sent"

  def Close(self, widget, data=None):
    self.Unbind(self)
    gtk.main_quit()

  def GetClients(self):
    if not self.socket_handler: return False
    rgx = re.compile(r'(.*)\ \(type (.*)\)')
    print "Getting clients..."
    self.socket_handler.Send('GET_CLIENTS', self.socket_handler.GetClientId())
    rcv = self.socket_handler.Receive()
    if len(rcv)<2 or rcv[0]!='CLIENTS_LIST' or ';' not in rcv[1]:
      return True
    for client in rcv[1].split(';'):
      client_id, client_type = re.split(rgx, client)[1:3]
      self.Log('Client: '+str(client_id))
      print client_id, client_type
    return True

  def GetExceptions(self):
    return False

  def StartAcquisition(self, widget, data=None):
    print "Requested to start acquisition"
    self.acquisition_started = True
    self.start_button.set_sensitive(not self.acquisition_started)
    self.stop_button.set_sensitive(self.acquisition_started)

  def StopAcquisition(self, widget, data=None):
    print "Requested to stop acquisition"
    self.acquisition_started = False
    self.start_button.set_sensitive(not self.acquisition_started)
    self.stop_button.set_sensitive(self.acquisition_started)

  def Log(self, data=None):
    #self.log.insert_at_cursor(data)
    self.log.insert(self.log.get_end_iter(), data+'\n')
    while gtk.events_pending():
      gtk.main_iteration()

  def main(self):
    gtk.main()

if __name__=='__main__':
  gui = DAQgui()
  gui.main()
