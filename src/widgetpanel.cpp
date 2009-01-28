/***************************************************************************
 *   Copyright (C) 2008 by Andres Cabrera   *
 *   mantaraya36@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/
#include "widgetpanel.h"
#include "qutewidget.h"
#include "quteslider.h"
#include "qutetext.h"
#include "qutespinbox.h"
#include "qutebutton.h"
#include "quteknob.h"
#include "qutecheckbox.h"
#include "qutecombobox.h"
#include "qutemeter.h"
#include "quteconsole.h"
#include "qutegraph.h"
#include "qutescope.h"
#include "qutedummy.h"
#include "framewidget.h"
#include "curve.h"

#include "qutecsound.h"

WidgetPanel::WidgetPanel(QWidget *parent)
  : QDockWidget(parent)
{
  setWindowTitle("Widgets");
  setMinimumSize(200, 140);
  layoutWidget = new LayoutWidget(this);
  layoutWidget->setGeometry(QRect(0, 0, 800, 600));
  layoutWidget->setAutoFillBackground(true);
//   layoutWidget->setFocusPolicy(Qt::NoFocus);
  connect(layoutWidget, SIGNAL(deselectAll()), this, SLOT(deselectAll()));
  connect(layoutWidget, SIGNAL(selection(QRect)), this, SLOT(selectionChanged(QRect)));
//   connect(this,SIGNAL(topLevelChanged(bool)), this, SLOT(dockStateChanged(bool)));

  createSliderAct = new QAction(tr("Create Slider"),this);
  connect(createSliderAct, SIGNAL(triggered()), this, SLOT(createSlider()));
  createLabelAct = new QAction(tr("Create Label"),this);
  connect(createLabelAct, SIGNAL(triggered()), this, SLOT(createLabel()));
  createDisplayAct = new QAction(tr("Create Display"),this);
  connect(createDisplayAct, SIGNAL(triggered()), this, SLOT(createDisplay()));
  createScrollNumberAct = new QAction(tr("Create ScrollNumber"),this);
  connect(createScrollNumberAct, SIGNAL(triggered()), this, SLOT(createScrollNumber()));
  createLineEditAct = new QAction(tr("Create LineEdit"),this);
  connect(createLineEditAct, SIGNAL(triggered()), this, SLOT(createLineEdit()));
  createSpinBoxAct = new QAction(tr("Create SpinBox"),this);
  connect(createSpinBoxAct, SIGNAL(triggered()), this, SLOT(createSpinBox()));
  createButtonAct = new QAction(tr("Create Button"),this);
  connect(createButtonAct, SIGNAL(triggered()), this, SLOT(createButton()));
  createKnobAct = new QAction(tr("Create Knob"),this);
  connect(createKnobAct, SIGNAL(triggered()), this, SLOT(createKnob()));
  createCheckBoxAct = new QAction(tr("Create Checkbox"),this);
  connect(createCheckBoxAct, SIGNAL(triggered()), this, SLOT(createCheckBox()));
  createMenuAct = new QAction(tr("Create Menu"),this);
  connect(createMenuAct, SIGNAL(triggered()), this, SLOT(createMenu()));
  createMeterAct = new QAction(tr("Create Controller"),this);
  connect(createMeterAct, SIGNAL(triggered()), this, SLOT(createMeter()));
  createConsoleAct = new QAction(tr("Create Console"),this);
  connect(createConsoleAct, SIGNAL(triggered()), this, SLOT(createConsole()));
  createGraphAct = new QAction(tr("Create Graph"),this);
  connect(createGraphAct, SIGNAL(triggered()), this, SLOT(createGraph()));
  createScopeAct = new QAction(tr("Create Scope"),this);
  connect(createScopeAct, SIGNAL(triggered()), this, SLOT(createScope()));
  propertiesAct = new QAction(tr("Properties"),this);
  connect(propertiesAct, SIGNAL(triggered()), this, SLOT(propertiesDialog()));
  editAct = new QAction(tr("Widget Edit Mode"), this);
  editAct->setCheckable(true);
  editAct->setShortcut(tr("Ctrl+E"));
  connect(editAct, SIGNAL(triggered(bool)), this, SLOT(activateEditMode(bool)));
  copyAct = new QAction(tr("Copy Selected"), this);
  copyAct->setShortcut(tr("Ctrl+C"));
  connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));
  cutAct = new QAction(tr("Cut Selected"), this);
  cutAct->setShortcut(tr("Ctrl+X"));
  connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));
  pasteAct = new QAction(tr("Paste Selected"), this);
  pasteAct->setShortcut(tr("Ctrl+V"));
  connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));
  duplicateAct = new QAction(tr("Duplicate Selected"), this);
  duplicateAct->setShortcut(tr("Ctrl+D"));
  connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));
  deleteAct = new QAction(tr("Delete Selected"), this);
  deleteAct->setShortcut(tr("Del"));
  connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteSelected()));
  clearAct = new QAction(tr("Clear all widgets"), this);
  connect(clearAct, SIGNAL(triggered()), this, SLOT(clearWidgets()));
  selectAllAct = new QAction(tr("Select all widgets"), this);
  connect(selectAllAct, SIGNAL(triggered()), this, SLOT(selectAll()));

  setWidget(layoutWidget);
  resize(200, 100);

  eventQueue.resize(QUTECSOUND_MAX_EVENTS);
  eventQueueSize = 0;
  QTimer::singleShot(30, this, SLOT(updateData()));
}

WidgetPanel::~WidgetPanel()
{
}

unsigned int WidgetPanel::widgetCount()
{
  return widgets.size();
}

void WidgetPanel::getValues(QVector<QString> *channelNames, QVector<double> *values, QVector<QString> *stringValues)
{
  if (channelNames->size() < widgets.size()*2)
    return;
  for (int i = 0; i < widgets.size(); i++) {
    (*channelNames)[i*2] = widgets[i]->getChannelName();
    (*values)[i*2] = widgets[i]->getValue();
    (*stringValues)[i*2] = widgets[i]->getStringValue();
    (*channelNames)[i*2 + 1] = widgets[i]->getChannel2Name();
    (*values)[i*2 + 1] = widgets[i]->getValue2();
  }
}

void WidgetPanel::setValue(QString channelName, double value)
{
  for (int i = 0; i < widgets.size(); i++) {
    if (widgets[i]->getChannelName() == channelName) {
      widgets[i]->setValue(value);
    }
    if (widgets[i]->getChannel2Name() == channelName) {
      widgets[i]->setValue2(value);
    }
  }
}

void WidgetPanel::setValue(QString channelName, QString value)
{
  for (int i = 0; i < widgets.size(); i++) {
    if (widgets[i]->getChannelName() == channelName) {
      widgets[i]->setValue(value);
    }
//     if (widgets[i]->getChannel2Name() == channelName) {
//       widgets[i]->setValue2(value);
//     }
  }
}

void WidgetPanel::setValue(int index, double value)
{
  // there are two values for each widget
  if (index >= widgets.size() * 2)
    return;
  widgets[index/2]->setValue(value);
}

void WidgetPanel::setValue(int index, QString value)
{
  // there are two values for each widget
  if (index >= widgets.size() * 2)
    return;
  widgets[index/2]->setValue(value);
}

void WidgetPanel::loadWidgets(QString macWidgets)
{
  clearWidgetPanel();
  QStringList widgetLines = macWidgets.split(QRegExp("[\n\r]"), QString::SkipEmptyParts);
  foreach (QString line, widgetLines) {
//     qDebug("WidgetLine: %s", line.toStdString().c_str());
    if (line.startsWith("i")) {
      if (newWidget(line) < 0)
        qDebug() << "WidgetPanel::loadWidgets error processing line: " << line;
    }
    else {
      if (!line.contains("<MacGUI>") && !line.contains("</MacGUI>"))
      qDebug() << "WidgetPanel::loadWidgets error processing line: " << line;
    }
  }
  if (editAct->isChecked()) {
    activateEditMode(true);
  }
}

int WidgetPanel::newWidget(QString widgetLine, bool offset)
{
  // This function returns -1 on error, 0 when no widget was created and 1 if widget was created
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QStringList quoteParts = widgetLine.split('"'); //Remove this line whe not needed
  if (parts.size()<5)
    return -1;
  if (parts[0]=="ioView") {
  // Colors in MacCsound have a range of 0-65535
    setBackground(parts[1]=="background",
                  QColor(parts[2].toInt()/256,
                         parts[3].toInt()/256,
                         parts[4].toInt()/256
                        )
                 );
    return 0;
  }
  else {
    int x,y,width,height;
    x = parts[1].toInt();
    y = parts[2].toInt();
    if (offset) {
      x += 20;
      y += 20;
    }
    width = parts[3].toInt();
    height = parts[4].toInt();
    if (parts[0]=="ioSlider") {
        return createSlider(x,y,width,height, widgetLine);
    }
    else if (parts[0]=="ioText") {
      if (parts[5]=="label" or parts[5]=="display") {
        return createText(x,y,width,height, widgetLine);
      }
      else if (parts[5]=="edit") {
        return createLineEdit(x,y,width, height, widgetLine);
      }
      else if (parts[5]=="scroll") {
        return createScrollNumber(x,y,width, height, widgetLine);
      }
      else if (parts[5]=="editnum") {
        return createSpinBox(x,y,width, height, widgetLine);
      }
    }
    else if (parts[0]=="ioButton") {
      return createButton(x,y,width,height, widgetLine);
    }
    else if (parts[0]=="ioKnob") {
      return createKnob(x,y,width, height, widgetLine);
    }
    else if (parts[0]=="ioCheckbox") {
      return createCheckBox(x,y,width, height, widgetLine);
    }
    else if (parts[0]=="ioMenu") {
      return createMenu(x,y,width, height, widgetLine);
    }
    else if (parts[0]=="ioListing") {
      return createConsole(x,y,width, height, widgetLine);
    }
    else if (parts[0]=="ioMeter") {
      return createMeter(x,y,width, height, widgetLine);
    }
    else if (parts[0]=="ioGraph") {
      if (parts.size() < 6 or parts[5]=="table")
        return createGraph(x,y,width, height, widgetLine);
      else if (parts[5]=="fft" or parts[5]=="scope")
        return createScope(x,y,width, height, widgetLine);
    }
    else {
      // Unknown widget...
      qDebug("WidgetPanel::newWidget Warning: unknown widget!");
      return createDummy(x,y,width, height, widgetLine);
    }
  }
  return -1;
}

void WidgetPanel::clearWidgets()
{
  clearWidgetPanel();
  widgetChanged();
}

void WidgetPanel::clearWidgetPanel()
{
//   qDebug("WidgetPanel::clearWidgetPanel()");
  foreach (QuteWidget *widget, widgets) {
    delete widget;
  }
  widgets.clear();
  foreach (FrameWidget *widget, editWidgets) {
//     qDebug("WidgetPanel::clearWidgetpanel() removed editWidget");
    delete widget;
  }
  editWidgets.clear();
  consoleWidgets.clear();
  graphWidgets.clear();
  scopeWidgets.clear();
}

void WidgetPanel::closeEvent(QCloseEvent * /*event*/)
{
  emit Close(false);
}

QString WidgetPanel::widgetsText()
{
  // This function must be used with care as it accesses the widgets, which
  // may cause crashing since widgets are not reentrant
  QString text = "<MacGUI>\n";
  text += "ioView " + (autoFillBackground()? QString("background "):QString("nobackground "));
  text += "{" + QString::number((int) (palette().button().color().redF()*65535.)) + ", ";
  text +=  QString::number((int) (palette().button().color().greenF()*65535.)) + ", ";
  text +=  QString::number((int) (palette().button().color().blueF()*65535.)) +"}\n";

  valueMutex.lock();
  for (int i = 0; i < widgets.size(); i++) {
    text += widgets[i]->getWidgetLine() + "\n";
  }
  valueMutex.unlock();
  text += "</MacGUI>";
  return text;
}

void WidgetPanel::appendMessage(QString message)
{
  for (int i=0; i < consoleWidgets.size(); i++) {
    consoleWidgets[i]->appendMessage(message);
  }
}

void WidgetPanel::showTooltips(bool show)
{
  m_tooltips = show;
  for (int i=0; i < widgets.size(); i++) {
    setWidgetToolTip(widgets[i], show);
  }
}

void WidgetPanel::setWidgetToolTip(QuteWidget *widget, bool show)
{
  if (show) {
    if (widget->getChannel2Name() != "") {
      widget->setToolTip(tr("ChannelV:") + widget->getChannelName()
          + tr("\nChannelH:")+ widget->getChannel2Name());
    }
    else {
      widget->setToolTip(tr("Channel:") + widget->getChannelName());
    }
  }
  else
    widget->setToolTip("");
}

void WidgetPanel::newCurve(Curve* curve)
{
  for (int i = 0; i < graphWidgets.size(); i++) {
    graphWidgets[i]->addCurve(curve);
    qApp->processEvents(); //Kludge to allow correct resizing of graph view
    graphWidgets[i]->changeCurve(-1);
  }
}

// int WidgetPanel::getCurveIndex(Curve * curve)
// {
//   int index = -1;
//   if (!graphWidgets.isEmpty())
//     index = graphWidgets[0]->getCurveIndex(curve);
//   return index;
// }

void WidgetPanel::setCurveData(Curve *curve)
{
//   qDebug("WidgetPanel::setCurveData");
  for (int i = 0; i < graphWidgets.size(); i++) {
    graphWidgets[i]->setCurveData(curve);
  }
}

Curve * WidgetPanel::getCurveById(uintptr_t id)
{
  Curve * curve = 0;;
  if (!graphWidgets.isEmpty())
    curve = graphWidgets[0]->getCurveById(id);
  return curve;
}

void WidgetPanel::flush()
{
  // Called when running Csound to flush flush queues
  newValues.clear();
  eventQueueSize = 0; //Flush events gathered while idle
}

QString WidgetPanel::getCsladspaLines()
{
  QString text = "";
  int unsupported = 0;
  foreach(QuteWidget *widget, widgets) {
    QString line = widget->getCsladspaLine();
    if (line != "") {
      text += line + "\n";
    }
    else {
      unsupported++;
    }
  }
  qDebug() << "WidgetPanel:getCsladspaLines() " << unsupported << " Unsupported widgets";
  return text;
}

QString WidgetPanel::getCabbageLines()
{
  QString text = "";
  int unsupported = 0;
  foreach(QuteWidget *widget, widgets) {
    QString line = widget->getCabbageLine();
    if (line != "") {
      text += line;
    }
    else {
      unsupported++;
    }
  }
  qDebug() << "WidgetPanel:getCabbageLines() " << unsupported << " Unsupported widgets";
  return text;
}

void WidgetPanel::clearGraphs()
{
  for (int i = 0; i < graphWidgets.size(); i++) {
    graphWidgets[i]->clearCurves();
  }
}

void WidgetPanel::deleteWidget(QuteWidget *widget)
{
  int index = widgets.indexOf(widget);
//   qDebug("WidgetPanel::deleteWidget %i", number);
  widget->close();
  widgets.remove(index);
  if (!editWidgets.isEmpty()) {
    delete(editWidgets[index]);
    editWidgets.remove(index);
  }
  index = consoleWidgets.indexOf(dynamic_cast<QuteConsole *>(widget));
  if (index >= 0) {
    consoleWidgets.remove(index);
  }
  index = graphWidgets.indexOf(dynamic_cast<QuteGraph *>(widget));
  if (index >= 0)
    graphWidgets.remove(index);
  index = scopeWidgets.indexOf(dynamic_cast<QuteScope *>(widget));
  if (index >= 0)
    scopeWidgets.remove(index);
  widgetChanged(widget);
}

void WidgetPanel::queueEvent(QString eventLine)
{
//   qDebug("WidgetPanel::queueEvent %s", eventLine.toStdString().c_str());
  if (eventQueueSize < QUTECSOUND_MAX_EVENTS) {
    //TODO is this lock necessary?
//     eventMutex.lock();
    eventQueue[eventQueueSize] = eventLine;
    eventQueueSize++;
//     eventMutex.unlock();
  }
  else
    qDebug("Warning: event queue full, event not processed");
}

void WidgetPanel::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu;
  menu.addAction(createSliderAct);
  menu.addAction(createLabelAct);
  menu.addAction(createDisplayAct);
  menu.addAction(createScrollNumberAct);
  menu.addAction(createLineEditAct);
  menu.addAction(createSpinBoxAct);
  menu.addAction(createButtonAct);
  menu.addAction(createKnobAct);
  menu.addAction(createCheckBoxAct);
  menu.addAction(createMenuAct);
  menu.addAction(createMeterAct);
  menu.addAction(createConsoleAct);
  menu.addAction(createGraphAct);
  menu.addAction(createScopeAct);
  menu.addSeparator();
  menu.addAction(editAct);
  menu.addSeparator();
  menu.addAction(copyAct);
  menu.addAction(pasteAct);
  menu.addAction(selectAllAct);
  menu.addAction(duplicateAct);
  menu.addAction(cutAct);
  menu.addAction(deleteAct);
  menu.addAction(clearAct);
  menu.addSeparator();
  menu.addAction(propertiesAct);
  currentPosition = event->pos();
  menu.exec(event->globalPos());
}

void WidgetPanel::resizeEvent(QResizeEvent * event)
{
  QDockWidget::resizeEvent(event);
  oldSize = event->oldSize();
//   qDebug("WidgetPanel::resizeEvent()");
  emit resized(event->size());
}

void WidgetPanel::moveEvent(QMoveEvent * event)
{
  QDockWidget::moveEvent(event);
  emit moved(event->pos());
}

void WidgetPanel::newValue(QPair<QString, double> channelValue)
{
//   qDebug("WidgetPanel::newValue");
  if (!channelValue.first.isEmpty()) {
    if(newValues.contains(channelValue.first)) {
      valueMutex.lock();
      newValues[channelValue.first] = channelValue.second;
      valueMutex.unlock();
    }
    else {
      valueMutex.lock();
      newValues.insert(channelValue.first, channelValue.second);
      valueMutex.unlock();
    }
  }
//   widgetChanged();
}

void WidgetPanel::processNewValues()
{
//   qDebug("WidgetPanel::processNewValues");
  valueMutex.lock();
  QList<QString> channelNames = newValues.keys();
  valueMutex.unlock();
  foreach(QString name, channelNames) {
    for (int i = 0; i < widgets.size(); i++){
      if (widgets[i]->getChannelName() == name) {
        widgets[i]->setValue(newValues.value(name));
      }
      if (widgets[i]->getChannel2Name() == name) {
        widgets[i]->setValue2(newValues.value(name));
      }
    }
  }
  valueMutex.lock();
  newValues.clear();
  valueMutex.unlock();
}

void WidgetPanel::widgetChanged(QuteWidget* widget)
{
//   for (int i = 0; i < editWidgets.size(); i++) {
//     if (editWidgets[i]->getWidget() == widget) {
//       //TODO set edit widget size and pos to widget's
//     }
//   }
}

// void WidgetPanel::updateWidgetText()
// {
//   emit widgetsChanged(widgetsText());
// }

int WidgetPanel::createSlider(int x, int y, int width, int height, QString widgetLine)
{
//   qDebug("ioSlider x=%i y=%i w=%i h=%i", x,y, width, height);
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QuteSlider *widget= new QuteSlider(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setRange(parts[5].toDouble(), parts[6].toDouble());
  widget->setValue(parts[7].toDouble());
  if (parts.size()>8) {
    int i=8;
    QString channelName = "";
    while (parts.size()>i) {
      channelName += parts[i] + " ";
      i++;
    }
    channelName.chop(1);  //remove last space
    widget->setChannelName(channelName);
  }
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  connect(widget, SIGNAL(newValue(QPair<QString,double>)), this, SLOT(newValue(QPair<QString,double>)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createText(int x, int y, int width, int height, QString widgetLine)
{
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QStringList quoteParts = widgetLine.split('"');
  if (parts.size()<20 or quoteParts.size()<5)
    return -1;
  QStringList lastParts = quoteParts[4].split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  if (lastParts.size() < 9)
    return -1;
  QuteText *widget= new QuteText(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setType(parts[5]);
  widget->setResolution(parts[7].toDouble());
  widget->setChannelName(quoteParts[1]);
  if (quoteParts[2] == " left ")
    widget->setAlignment(0);
  else if (quoteParts[2] == " center ")
    widget->setAlignment(1);
  else if (quoteParts[2] == " right ")
    widget->setAlignment(2);
  widget->setFont(quoteParts[3]);
  widget->setFontSize(lastParts[0].toInt());
  widget->setTextColor(QColor(lastParts[1].toDouble()/256.0,
                       lastParts[2].toDouble()/256.0,
                                             lastParts[3].toDouble()/256.0));
  widget->setBgColor(QColor(lastParts[4].toDouble()/256.0,
                     lastParts[5].toDouble()/256.0,
                                           lastParts[6].toDouble()/256.0));
  widget->setBg(lastParts[7] == "background");
  widget->setBorder(lastParts[8] == "border");
  QString labelText = "";
//   int i = 9;
//   while (lastParts.size() > i) {
//     labelText += lastParts[i] + " ";
//     i++;
//   }
//   labelText.chop(1);
  labelText = widgetLine.mid(widgetLine.indexOf("border") + 7);
  widget->setText(labelText);
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createScrollNumber(int x, int y, int width, int height, QString widgetLine)
{
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QStringList quoteParts = widgetLine.split('"');
  if (parts.size()<20 or quoteParts.size()<5)
    return -1;
  QStringList lastParts = quoteParts[4].split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  if (lastParts.size() < 9)
    return -1;
  QuteScrollNumber *widget= new QuteScrollNumber(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setType(parts[5]);
  widget->setResolution(parts[7].toDouble());
  widget->setChannelName(quoteParts[1]);
  if (quoteParts[2] == " left ")
    widget->setAlignment(0);
  else if (quoteParts[2] == " center ")
    widget->setAlignment(1);
  else if (quoteParts[2] == " right ")
    widget->setAlignment(2);
  widget->setFont(quoteParts[3]);
  widget->setFontSize(lastParts[0].toInt());
  widget->setTextColor(QColor(lastParts[1].toDouble()/256.0,
                       lastParts[2].toDouble()/256.0,
                                             lastParts[3].toDouble()/256.0));
  widget->setBgColor(QColor(lastParts[4].toDouble()/256.0,
                     lastParts[5].toDouble()/256.0,
                                           lastParts[6].toDouble()/256.0));
  widget->setBg(lastParts[7] == "background");
  widget->setBorder(lastParts[8] == "border");
  QString labelText = "";
  int i = 9;
  while (lastParts.size() > i) {
    labelText += lastParts[i] + " ";
    i++;
  }
  labelText.chop(1);
  bool ok;
  widget->setValue(labelText.toDouble(&ok));
  if (!ok)
    widget->setText(labelText);
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  connect(widget, SIGNAL(newValue(QPair<QString,double>)), this, SLOT(newValue(QPair<QString,double>)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createLineEdit(int x, int y, int width, int height, QString widgetLine)
{
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QStringList quoteParts = widgetLine.split('"');
  if (parts.size()<20 or quoteParts.size()<5)
    return -1;
  QStringList lastParts = quoteParts[4].split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  if (lastParts.size() < 9)
    return -1;
  QuteLineEdit *widget= new QuteLineEdit(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setType(parts[5]);
  widget->setResolution(parts[7].toDouble());
  widget->setChannelName(quoteParts[1]);
  if (quoteParts[2] == " left ")
    widget->setAlignment(0);
  else if (quoteParts[2] == " center ")
    widget->setAlignment(1);
  else if (quoteParts[2] == " right ")
    widget->setAlignment(2);
  widget->setFont(quoteParts[3]);
  widget->setFontSize(lastParts[0].toInt());
  widget->setTextColor(QColor(lastParts[1].toDouble()/256.0,
                       lastParts[2].toDouble()/256.0,
                                             lastParts[3].toDouble()/256.0));
  widget->setBgColor(QColor(lastParts[4].toDouble()/256.0,
                     lastParts[5].toDouble()/256.0,
                                           lastParts[6].toDouble()/256.0));
  widget->setBg(lastParts[7] == "background");
//   widget->setBorder(lastParts[8] == "border");
  QString labelText = "";
  int i = 9;
  while (lastParts.size() > i) {
    labelText += lastParts[i] + " ";
    i++;
  }
  labelText.chop(1);
  widget->setText(labelText);
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createSpinBox(int x, int y, int width, int height, QString widgetLine)
{
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QStringList quoteParts = widgetLine.split('"');
  if (parts.size()<20 or quoteParts.size()<5)
    return -1;
  QStringList lastParts = quoteParts[4].split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  if (lastParts.size() < 9)
    return -1;
  QuteSpinBox *widget= new QuteSpinBox(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setType(parts[5]);
  widget->setResolution(parts[7].toDouble());
  widget->setChannelName(quoteParts[1]);
  if (quoteParts[2] == " left ")
    widget->setAlignment(0);
  else if (quoteParts[2] == " center ")
    widget->setAlignment(1);
  else if (quoteParts[2] == " right ")
    widget->setAlignment(2);
  widget->setFont(quoteParts[3]);
  widget->setFontSize(lastParts[0].toInt());
  widget->setTextColor(QColor(lastParts[1].toDouble()/256.0,
                       lastParts[2].toDouble()/256.0,
                                             lastParts[3].toDouble()/256.0));
  widget->setBgColor(QColor(lastParts[4].toDouble()/256.0,
                     lastParts[5].toDouble()/256.0,
                                           lastParts[6].toDouble()/256.0));
  widget->setBg(lastParts[7] == "background");
//   widget->setBorder(lastParts[8] == "border");
  QString labelText = "";
  int i = 9;
  while (lastParts.size() > i) {
    labelText += lastParts[i] + " ";
    i++;
  }
  labelText.chop(1);
  widget->setText(labelText);
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  connect(widget, SIGNAL(newValue(QPair<QString,double>)), this, SLOT(newValue(QPair<QString,double>)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createButton(int x, int y, int width, int height, QString widgetLine)
{
//   qDebug("WidgetPanel::createButton");
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QStringList quoteParts = widgetLine.split('"');
//   if (parts.size()<20 or quoteParts.size()>5)
//     return -1;
  QStringList lastParts = quoteParts[4].split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
//   if (lastParts.size() < 9)
//     return -1;
  QuteButton *widget= new QuteButton(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->show();
  widgets.append(widget);
  widget->setValue(parts[6].toDouble());  //value produced by button
  widget->setChannelName(quoteParts[1]);
  widget->setText(quoteParts[3]);
  widget->setFilename(quoteParts[5]);
  widget->setType(parts[5]); // setType must come after setFilename so image is loaded
  if (quoteParts.size()>6) {
    quoteParts[6].remove(0,1); //remove initial space
    widget->setEventLine(quoteParts[6]);
  }
  connect(widget, SIGNAL(queueEvent(QString)), this, SLOT(queueEvent(QString)));
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  connect(widget, SIGNAL(play()), static_cast<qutecsound *>(parent()), SLOT(runCsound()));
  connect(widget, SIGNAL(stop()), static_cast<qutecsound *>(parent()), SLOT(stop()));

  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createKnob(int x, int y, int width, int height, QString widgetLine)
{
//   qDebug("ioKnob x=%i y=%i w=%i h=%i", x,y, width, height);
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QuteKnob *widget= new QuteKnob(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setRange(parts[5].toDouble(), parts[6].toDouble());
  //TODO set resolution of knob
  widget->setValue(parts[8].toDouble());
  if (parts.size()>9) {
    int i=9;
    QString channelName = "";
    while (parts.size()>i) {
      channelName += parts[i] + " ";
      i++;
    }
    channelName.chop(1);  //remove last space
    widget->setChannelName(channelName);
  }
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  connect(widget, SIGNAL(newValue(QPair<QString,double>)), this, SLOT(newValue(QPair<QString,double>)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createCheckBox(int x, int y, int width, int height, QString widgetLine)
{
//   qDebug("ioCheckBox x=%i y=%i w=%i h=%i", x,y, width, height);
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QuteCheckBox *widget= new QuteCheckBox(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setValue(parts[5]=="on");
  if (parts.size()>6) {
    int i=6;
    QString channelName = "";
    while (parts.size()>i) {
      channelName += parts[i] + " ";
      i++;
    }
    channelName.chop(1);  //remove last space
    widget->setChannelName(channelName);
  }
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  connect(widget, SIGNAL(newValue(QPair<QString,double>)), this, SLOT(newValue(QPair<QString,double>)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createMenu(int x, int y, int width, int height, QString widgetLine)
{
//   qDebug("ioMenu x=%i y=%i w=%i h=%i", x,y, width, height);
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QStringList quoteParts = widgetLine.split('"');
  QuteComboBox *widget= new QuteComboBox(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setSize(parts[6].toInt());
  widget->setText(quoteParts[1]);
  widget->setValue(parts[5].toDouble()); //setValue must be after setText otherwise ComboBox is empty
  if (quoteParts.size() > 2)
    widget->setChannelName(quoteParts[2].remove(0,1)); //remove initial space from channel name
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  connect(widget, SIGNAL(newValue(QPair<QString,double>)), this, SLOT(newValue(QPair<QString,double>)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createMeter(int x, int y, int width, int height, QString widgetLine)
{
//   qDebug("ioMenu x=%i y=%i w=%i h=%i", x,y, width, height);
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QStringList quoteParts = widgetLine.split('"');
  if (quoteParts.size() < 5) {
    qDebug("WidgetPanel::createMeter ERROR parsing widget line!");
    return 0;
  }
  QStringList parts2 = quoteParts[4].split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  if (parts2.size() < 5) {
    qDebug("WidgetPanel::createMeter ERROR parsing widget line!");
    return 0;
  }
  QuteMeter *widget= new QuteMeter(layoutWidget);
  //TODO is setWidgetLine actually necessary?
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setColor(QColor(parts[5].toDouble()/256.0,
                       parts[6].toDouble()/256.0,
                                         parts[7].toDouble()/256.0));
  widget->setType(parts2[1]); // Important to set type before setting values since values are inverted for crosshair and point
  widget->setChannelName(quoteParts[1]);
  widget->setValue(quoteParts[2].toDouble());
  widget->setChannel2Name(quoteParts[3]);
  widget->setValue2(parts2[0].toDouble());
  widget->setPointSize(parts2[2].toInt());
  widget->setFadeSpeed(parts2[3].toInt());
  widget->setBehavior(parts2[4]);
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  connect(widget, SIGNAL(newValue(QPair<QString,double>)), this, SLOT(newValue(QPair<QString,double>)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

int WidgetPanel::createConsole(int x, int y, int width, int height, QString widgetLine)
{
//    qDebug("ioListing x=%i y=%i w=%i h=%i", x,y, width, height);
   QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
   QuteConsole *widget= new QuteConsole(layoutWidget);
   widget->setWidgetLine(widgetLine);
   widget->setWidgetGeometry(x,y,width, height);
//    connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
   connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
   widgets.append(widget);
   widget->show();
   if (editAct->isChecked()) {
     createEditFrame(widget);
   }
   setWidgetToolTip(widget, m_tooltips);
   consoleWidgets.append(widget);
   return 1;
}

int WidgetPanel::createGraph(int x, int y, int width, int height, QString widgetLine)
{
//   qDebug("ioGraph x=%i y=%i w=%i h=%i", x,y, width, height);
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
  QuteGraph *widget= new QuteGraph(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setUd(static_cast<qutecsound *>(parent())->ud);
  //Graph widget is always of type "graph" part 5 is discarded
  if (parts.size() > 6)
    widget->setValue(parts[6].toDouble());
  if (parts.size() > 7)
    widget->setZoom(parts[7].toDouble());
  else
    widget->setZoom(1.0);
  if (parts.size()>8) {
    int i = 8;
    QString channelName = "";
    while (parts.size()>i) {
      channelName += parts[i] + " ";
      i++;
    }
    channelName.chop(1);  //remove last space
    widget->setChannelName(channelName);
  }
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  graphWidgets.append(widget);
  return 1;
}

int WidgetPanel::createScope(int x, int y, int width, int height, QString widgetLine)
{
//   qDebug("WidgetPanel::createScope ioGraph x=%i y=%i w=%i h=%i", x,y, width, height);
//   qDebug("%s",widgetLine.toStdString().c_str() );
  QuteScope *widget= new QuteScope(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->setUd(static_cast<qutecsound *>(parent())->ud);
  QStringList parts = widgetLine.split(QRegExp("[\\{\\}, ]"), QString::SkipEmptyParts);
//   if (parts.size() > 5)
//     widget->setType(parts[5]);
  if (parts.size() > 6)
    widget->setValue(parts[6].toDouble()); //Value here indicates zoom level
  if (parts.size() > 7) {
    widget->setChannel((int) parts[7].toDouble()); // Channel number to display
  }
  if (parts.size() > 8) {
    int i=8;
    QString channelName = "";
    while (parts.size()>i) {
      channelName += parts[i] + " ";
      i++;
    }
    channelName.chop(1);  //remove last space
    widget->setChannelName(channelName);
  }
//   connect(static_cast<qutecsound *>(parent()), SIGNAL(updateData()), widget, SLOT(updateData()));
//   connect(widget, SIGNAL(widgetChanged(QuteWidget *)), this, SLOT(widgetChanged(QuteWidget *)));
  connect(widget, SIGNAL(deleteThisWidget(QuteWidget *)), this, SLOT(deleteWidget(QuteWidget *)));
  widgets.append(widget);
  widget->show();
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  scopeWidgets.append(widget);
  return 1;
}

int WidgetPanel::createDummy(int x, int y, int width, int height, QString widgetLine)
{
  QuteWidget *widget= new QuteDummy(layoutWidget);
  widget->setWidgetLine(widgetLine);
  widget->setWidgetGeometry(x,y,width, height);
  widget->show();
  widgets.append(widget);
  if (editAct->isChecked()) {
    createEditFrame(widget);
  }
  setWidgetToolTip(widget, m_tooltips);
  return 1;
}

void WidgetPanel::setBackground(bool bg, QColor bgColor)
{
  if (bg) {
    setPalette(QPalette(bgColor));
    setAutoFillBackground(true);
  }
  else { // =="nobackground"
    setPalette(QPalette());
    setAutoFillBackground(false);
  }
}

void WidgetPanel::activateEditMode(bool active)
{
  if (active) {
    foreach (FrameWidget *widget, editWidgets) {
      delete widget;
    }
    editWidgets.clear();
    foreach (QuteWidget * widget, widgets) {
      createEditFrame(widget);
    }
  }
  else {
    foreach (QFrame* frame, editWidgets) {
      delete(frame);
    }
    editWidgets.clear();
  }
}

void WidgetPanel::createEditFrame(QuteWidget* widget)
{
  FrameWidget * frame = new FrameWidget(layoutWidget);
  QPalette palette(QColor(Qt::red),QColor(Qt::red));
  palette.setColor(QPalette::WindowText, QColor(Qt::red));
  frame->setWidget(widget);
  frame->setPalette(palette);
  frame->setGeometry(widget->x(), widget->y(), widget->width(), widget->height());
  frame->setFrameShape(QFrame::Box);
//       frame->setMouseTracking(false);  //Only track mouse when buttons are pressed
  frame->show();
  editWidgets.append(frame);
  connect(frame, SIGNAL(popUpMenu(QPoint)), widget, SLOT(popUpMenu(QPoint)));
  connect(frame, SIGNAL(deselectAllSignal()), this, SLOT(deselectAll()));
  connect(frame, SIGNAL(moved( QPair<int, int> )), this, SLOT(widgetMoved( QPair<int, int> )));
  connect(frame, SIGNAL(resized( QPair<int, int> )), this, SLOT(widgetResized( QPair<int, int> )));
}

void WidgetPanel::widgetMoved(QPair<int, int> delta)
{
  for (int i = 0; i < widgets.size(); i++) {
    if (editWidgets[i]->isSelected()) {
      int newx = widgets[i]->x() + delta.first;
      int newy = widgets[i]->y() + delta.second;
      widgets[i]->move(newx, newy);
      editWidgets[i]->move(newx, newy);
    }
  }
}

void WidgetPanel::widgetResized(QPair<int, int> delta)
{
//   qDebug("WidgetPanel::widgetResized %i  %i", delta.first, delta.second);
  for (int i = 0; i< editWidgets.size(); i++) {
    if (editWidgets[i]->isSelected()) {
      int neww = widgets[i]->width() + delta.first;
      int newh = widgets[i]->height() + delta.second;
      widgets[i]->setWidgetGeometry(widgets[i]->x(), widgets[i]->y(), neww, newh);
      editWidgets[i]->resize(neww, newh);
    }
  }
}

void WidgetPanel::copy()
{
  clipboard.clear();
  for (int i = 0; i < editWidgets.size(); i++) {
    if (editWidgets[i]->isSelected()) {
      clipboard.append(widgets[i]->getWidgetLine());
//       qDebug("WidgetPanel::copy() %s", clipboard.last().toStdString().c_str());
    }
  }
}

void WidgetPanel::cut()
{
  WidgetPanel::copy();
  for (int i = editWidgets.size() - 1; i >= 0 ; i--) {
    if (editWidgets[i]->isSelected()) {
      deleteWidget(widgets[i]);
    }
  }
}

void WidgetPanel::paste()
{
  if (editAct->isChecked()) {
    deselectAll();
    foreach (QString line, clipboard) {
      newWidget(line);
      editWidgets.last()->select();
    }
  }
}

void WidgetPanel::paste(QPoint /*pos*/)
{
}

void WidgetPanel::duplicate()
{
  qDebug("WidgetPanel::duplicate()");
  if (editAct->isChecked()) {
    int size = editWidgets.size();
    for (int i = 0; i < size ; i++) {
      if (editWidgets[i]->isSelected()) {
        editWidgets[i]->deselect();
        newWidget(widgets[i]->getWidgetLine(), true);
        editWidgets.last()->select();
      }
    }
  }
}

void WidgetPanel::deleteSelected()
{
  for (int i = editWidgets.size() - 1; i >= 0 ; i--) {
    if (editWidgets[i]->isSelected()) {
      deleteWidget(widgets[i]);
    }
  }
}

void WidgetPanel::undo()
{
  qDebug("WidgetPanel::undo() not implemented yet");
}

void WidgetPanel::redo()
{
  qDebug("WidgetPanel::redo() not implemented yet");
}

// void  WidgetPanel::dockStateChanged(bool topLevel)
// {
//   if (!topLevel) {
//     emit resized(oldSize); //Must send size after widget has docked.
//   }
// }

void WidgetPanel::updateData()
{
  foreach (QuteScope *scope, scopeWidgets) {
    scope->updateData();
  }
  QTimer::singleShot(30, this, SLOT(updateData()));
}

void WidgetPanel::deselectAll()
{
  for (int i = 0; i< editWidgets.size(); i++) {
    editWidgets[i]->deselect();
  }
}

void WidgetPanel::selectAll()
{
  for (int i = 0; i< editWidgets.size(); i++) {
    editWidgets[i]->select();
  }
}

void WidgetPanel::selectionChanged(QRect selection)
{
//   qDebug("WidgetPanel::selectionChanged %i %i %i %i", selection.x(), selection.y(), selection.width(), selection.height());
  if (editWidgets.isEmpty())
    return; //not in edit mode
  deselectAll();
  for (int i = 0; i< widgets.size(); i++) {
    int x = widgets[i]->x();
    int y = widgets[i]->y();
    int w = widgets[i]->width();
    int h = widgets[i]->height();
    if (x > selection.x() - w && x < selection.x() + selection.width() &&
        y > selection.y() - h && y < selection.y() + selection.height() ) {
      editWidgets[i]->select();
    }
  }
}

void WidgetPanel::createSlider()
{
  createSlider(currentPosition.x(), currentPosition.y() - 20, 20, 100, QString("ioSlider {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) + "} {20, 100} 0.000000 1.000000 0.000000 slider" +QString::number(widgets.size())));
  widgetChanged();
}

void WidgetPanel::createLabel()
{
  QString line = "ioText {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) +"} {80, 25} label 0.000000 0.001000 \"\" left \"Lucida Grande\" 8 {0, 0, 0} {65535, 65535, 65535} nobackground border New Label";
  createText(currentPosition.x(), currentPosition.y() - 20, 80, 25, line);
  widgetChanged();
}

void WidgetPanel::createDisplay()
{
  QString line = "ioText {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) +"} {80, 25} display 0.000000 0.001000 \"\" left \"Lucida Grande\" 8 {0, 0, 0} {65535, 65535, 65535} nobackground border Display";
  createText(currentPosition.x(), currentPosition.y() - 20, 80, 25, line);
  widgetChanged();
}

void WidgetPanel::createScrollNumber()
{
  QString line = "ioText {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) +"} {80, 25} scroll 0.000000 0.001000 \"\" left \"Lucida Grande\" 8 {0, 0, 0} {65535, 65535, 65535} background border 0.000000";
  createScrollNumber(currentPosition.x(), currentPosition.y() - 20, 80, 25, line);
  widgetChanged();
}

void WidgetPanel::createLineEdit()
{
  QString line = "ioText {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) +"} {100, 25} edit 0.000000 0.001000 \"\" left \"Lucida Grande\" 8 {0, 0, 0} {65535, 65535, 65535} nobackground noborder Type here";
  createLineEdit(currentPosition.x(), currentPosition.y() - 20, 100, 25, line);
  widgetChanged();
}

void WidgetPanel::createSpinBox()
{
  QString line = "ioText {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) +"} {80, 25} editnum 0.000000 0.001000 \"\" left \"Lucida Grande\" 8 {0, 0, 0} {65535, 65535, 65535} nobackground noborder Type here";
  createSpinBox(currentPosition.x(), currentPosition.y() - 20, 80, 25, line);
  widgetChanged();
}

void WidgetPanel::createButton()
{
  QString line = "ioButton {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) +"} {100, 30} event 1.000000 \"button1\" \"New Button\" \"/\" i1 0 10";
  createButton(currentPosition.x(), currentPosition.y() - 20, 100, 30, line);
  widgetChanged();
}

void WidgetPanel::createKnob()
{
  createKnob(currentPosition.x(), currentPosition.y() - 20, 80, 80, QString("ioKnob {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) + "} {80, 80} 0.000000 1.000000 0.010000 0.000000 knob" +QString::number(widgets.size())));
  widgetChanged();
}

void WidgetPanel::createCheckBox()
{
  createCheckBox(currentPosition.x(), currentPosition.y() - 20, 20, 20, QString("ioCheckbox {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) + "} {20, 20} off checkbox" +QString::number(widgets.size())));
  widgetChanged();
}

void WidgetPanel::createMenu()
{
  createMenu(currentPosition.x(), currentPosition.y() - 20, 80, 30, QString("ioMenu {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) + "} {80, 30} 1 303 \"item1,item2,item3\" menu" +QString::number(widgets.size())));
  widgetChanged();
}

void WidgetPanel::createMeter()
{
  createMeter(currentPosition.x(), currentPosition.y() - 20, 30, 80, QString("ioMeter {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) + "} {30, 80} {0, 60000, 0} \"vert" + QString::number(widgets.size()) + "\" 0.000000 \"hor" + QString::number(widgets.size()) + "\" 0.000000 fill 1 0 mouse"));
  widgetChanged();
}

void WidgetPanel::createConsole()
{
  createConsole(currentPosition.x(), currentPosition.y() - 20, 320, 400, QString("ioListing {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) + "} {320, 400}"));
  widgetChanged();
}

void WidgetPanel::createGraph()
{
  createGraph(currentPosition.x(), currentPosition.y() - 20, 350, 150, QString("ioGraph {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) + "} {350, 150}"));
  widgetChanged();
}

void WidgetPanel::createScope()
{
  createScope(currentPosition.x(), currentPosition.y() - 20, 350, 150, QString("ioGraph {"+ QString::number(currentPosition.x()) +", "+ QString::number(currentPosition.y() - 20) + "} {350, 150} scope 2.000000 -1.000000"));
  widgetChanged();
}

void WidgetPanel::propertiesDialog()
{
  QDialog *dialog = new QDialog(this);
  dialog->resize(300, 300);
  dialog->setModal(true);
  QGridLayout *layout = new QGridLayout(dialog);
  bgCheckBox = new QCheckBox(dialog);
  bgCheckBox->setText("Enable Background");
  bgCheckBox->setChecked(autoFillBackground());
  layout->addWidget(bgCheckBox, 0, 0, Qt::AlignRight|Qt::AlignVCenter);
  QLabel *label = new QLabel(dialog);
//   label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  label->setText("Color");
  label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  layout->addWidget(label, 1, 0, Qt::AlignRight|Qt::AlignVCenter);
  bgButton = new QPushButton(dialog);
  QPixmap pixmap = QPixmap(64,64);
  pixmap.fill(palette().button().color());
  bgButton->setIcon(pixmap);
  layout->addWidget(bgButton, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);
  QPushButton *applyButton = new QPushButton(tr("Apply"));
  layout->addWidget(applyButton, 9, 1, Qt::AlignCenter|Qt::AlignVCenter);
  QPushButton *cancelButton = new QPushButton(tr("Cancel"));
  layout->addWidget(cancelButton, 9, 2, Qt::AlignCenter|Qt::AlignVCenter);
  QPushButton *acceptButton = new QPushButton(tr("Ok"));
  layout->addWidget(acceptButton, 9, 3, Qt::AlignCenter|Qt::AlignVCenter);
  connect(acceptButton, SIGNAL(released()), dialog, SLOT(accept()));
  connect(dialog, SIGNAL(accepted()), this, SLOT(applyProperties()));
  connect(applyButton, SIGNAL(released()), this, SLOT(applyProperties()));
  connect(cancelButton, SIGNAL(released()), dialog, SLOT(close()));
  connect(bgButton, SIGNAL(released()), this, SLOT(selectBgColor()));
  dialog->exec();
}

void WidgetPanel::applyProperties()
{
  setBackground(bgCheckBox->isChecked(), palette().button().color());
  widgetChanged();
}

void WidgetPanel::selectBgColor()
{
  QColor color = QColorDialog::getColor(palette().button().color(), this);
  if (color.isValid()) {
    setPalette(QPalette(color));
    QPixmap pixmap(64,64);
    pixmap.fill(palette().button().color());
    bgButton->setIcon(pixmap);
  }
}
