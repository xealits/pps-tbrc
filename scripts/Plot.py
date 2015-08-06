#!/usr/bin/python

import gtk
from gtk import gdk
import cairo
from pycha.line import LineChart
from pycha.bar import VerticalBarChart

class Plot (gtk.DrawingArea):
  def __init__(self):
    gtk.DrawingArea.__init__(self)
    self.connect("expose_event", self.expose)
    self.connect("size-allocate", self.size_allocate)
    self._surface = None
    self._options = None
 
  def set_options(self, options):
    """Set plot's options"""
    self._options = options
 
  def set_data(self, data):
    pass

  def get_data(self):
    pass
 
  def plot(self):
    pass
 
  def expose(self, widget, event):
    context = widget.window.cairo_create()
    context.rectangle(event.area.x, event.area.y, event.area.width, event.area.height)
    context.clip()
    self.draw(context)
    return False
 
  def draw(self, context):
    rect = self.get_allocation()
    self._surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, rect.width, rect.height)
    self.plot()
    context.set_source_surface(self._surface, 0, 0)
    context.paint()
 
  def size_allocate(self, widget, requisition):
    self.queue_draw()

class LinePlot(Plot):
  def __init__(self):
    Plot.__init__(self)
 
  def set_data(self, data, title='Data'):
    """
    Update plot's data and refreshes DrawingArea contents. Data must be a
    list containing a set of x and y values.
    """
    self._data = ((title, data),)
    self.queue_draw()
 
  def plot(self):
    """Initializes chart (if needed), set data and plots."""
    chart = LineChart(self._surface, self._options)
    chart.addDataset(self._data)
    chart.render()
