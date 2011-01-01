/*
    Copyright (C) 2008-2010 Andres Cabrera
    mantaraya36@gmail.com

    This file is part of QuteCsound.

    QuteCsound is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    QuteCsound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA
*/

#include <cwindow.h>
#include <csound.hpp>  // TODO These two are necessary for the WINDAT struct. Can they be moved?

#include "documentpage.h"
#include "documentview.h"
#include "csoundengine.h"
#include "liveeventframe.h"
#include "eventsheet.h"
#include "liveeventcontrol.h"
#include "qutecsound.h" // For playParent and renderParent functions (called from button reserved channels) and for connecting from console to log file
#include "opentryparser.h"
#include "types.h"
#include "dotgenerator.h"
#include "highlighter.h"
#include "widgetlayout.h"
#include "console.h"

#include "curve.h"
#include "qutebutton.h"


// TODO is is possible to move the editor to a separate child class, to be able to use a cleaner class?
DocumentPage::DocumentPage(QWidget *parent, OpEntryParser *opcodeTree):
    BaseDocument(parent, opcodeTree)
{
  init(parent, opcodeTree);
}

DocumentPage::~DocumentPage()
{
//  qDebug() << "DocumentPage::~DocumentPage()";
  disconnect(m_console, 0,0,0);
  disconnect(m_view, 0,0,0);
//  deleteAllLiveEvents(); // FIXME This is also crashing...
}

void DocumentPage::setFileName(QString name)
{
  fileName = name;
  if (name.endsWith(".csd") || name.isEmpty()) {
    m_view->setFileType(0);
  }
  else if (name.endsWith(".py")) {
    m_view->setFileType(1);
  }
  else if (name.endsWith(".xml")) {
    m_view->setFileType(2);
  }
  else if (name.endsWith(".orc")) {
    m_view->setFileType(3);
  }
  else if (name.endsWith(".sco")) {
    m_view->setFileType(4);
  }
  else {
    m_view->setFileType(-1);
  }
}

void DocumentPage::setCompanionFileName(QString name)
{
  companionFile = name;
}

void DocumentPage::setTextString(QString &text)
{
  setTextString(text, false);
}

int DocumentPage::setTextString(QString text, bool autoCreateMacCsoundSections)
{
  int ret = 0;
  deleteAllLiveEvents();
  if (!fileName.endsWith(".csd") && !fileName.isEmpty()) {
    m_view->setFullText(text, true); // Put all text since not a csd file (and not default file which has no name)
    m_view->setModified(false);
    return ret;
  }
  int baseRet = parseTextString(text);
  if (text.contains("<MacOptions>") and text.contains("</MacOptions>")) {
    QString options = text.right(text.size()-text.indexOf("<MacOptions>"));
    options.resize(options.indexOf("</MacOptions>") + 13);
    //     qDebug("<MacOptions> present. \n%s", options.toStdString().c_str());
    if (text.indexOf("</MacOptions>") + 13 < text.size() and text[text.indexOf("</MacOptions>") + 13] == '\n')
      text.remove(text.indexOf("</MacOptions>") + 13, 1); //remove final line break
    if (text.indexOf("<MacOptions>") > 0 and text[text.indexOf("<MacOptions>") - 1] == '\n')
      text.remove(text.indexOf("<MacOptions>") - 1, 1); //remove initial line break
    text.remove(text.indexOf("<MacOptions>"), options.size());
    //     qDebug("<MacOptions> present. %s", getMacOptions("WindowBounds").toStdString().c_str());
    m_macOptions = options.split('\n');
  }
  if (text.contains("<MacPresets>") and text.contains("</MacPresets>")) {
    m_macPresets = text.right(text.size()-text.indexOf("<MacPresets>"));
    m_macPresets.resize(m_macPresets.indexOf("</MacPresets>") + 12);
    if (text.indexOf("</MacPresets>") + 12 < text.size() and text[text.indexOf("</MacPresets>") + 12] == '\n')
      text.remove(text.indexOf("</MacPresets>") + 12, 1); //remove final line break
    if (text.indexOf("<MacPresets>") > 0 and text[text.indexOf("<MacPresets>") - 1] == '\n')
      text.remove(text.indexOf("<MacPresets>") - 1, 1); //remove initial line break
    text.remove(text.indexOf("<MacPresets>"), m_macPresets.size());
    //    qDebug("<MacPresets> present.");
  }
  else {
    m_macPresets = "";
  }
  if (text.contains("<MacGUI>") and text.contains("</MacGUI>")) {
    QString m_macGUI = text.right(text.size()-text.indexOf("<MacGUI>"));
    m_macGUI.resize(m_macGUI.indexOf("</MacGUI>") + 9);
    if (text.indexOf("</MacGUI>") + 9 < text.size() and text[text.indexOf("</MacGUI>") + 9] == '\n')
      text.remove(text.indexOf("</MacGUI>") + 9, 1); //remove final line break
    if (text.indexOf("<MacGUI>") > 0 and text[text.indexOf("<MacGUI>") - 1] == '\n')
      text.remove(text.indexOf("<MacGUI>") - 1, 1); //remove initial line break
    text.remove(m_macGUI);
    if (baseRet != 1) {
      //FIXME allow multiple
      m_widgetLayouts[0]->loadMacWidgets(m_macGUI);
      qDebug("<MacGUI> loaded.");
    }
  }
  else {
    if (autoCreateMacCsoundSections && baseRet != 1) {
      QString m_macGUI = "<MacGUI>\nioView nobackground {59352, 11885, 65535}\nioSlider {5, 5} {20, 100} 0.000000 1.000000 0.000000 slider1\n</MacGUI>";
      //FIXME allow multiple
      m_widgetLayouts[0]->loadMacWidgets(m_macGUI);
    }
    else {
      m_macGUI = "";
    }
  }
if (baseRet != 1) {  // Use the old options only if the new ones are not present
  // This here is for compatibility with MacCsound (copy output filename from <MacOptions> to <CsOptions>)
  QString optionsText = getMacOptions("Options:");
  if (optionsText.contains(" -o")) {
    QString outFile = optionsText.mid(optionsText.indexOf(" -o") + 1);
    int index = outFile.indexOf(" -");
    if (index > 0) {
      outFile = outFile.left(index);
    }
    optionsText.remove(outFile);
    setMacOption("Options:", optionsText);
    index = text.indexOf("<CsOptions>");
    int endindex = text.indexOf("</CsOptions>");
    if (index >= 0 and endindex > index) {
      text.remove(index, endindex - index);
      text.insert(index, "<CsOptions>\n" + outFile);
    }
    else {
      index = text.indexOf("<CsInstruments>");
      text.insert(index, "<CsOptions>\n" + outFile + "\n</CsOptions>\n");
    }
  }
  ret = 1;
}
if (baseRet != 1) {
  applyMacOptions(m_macOptions);
    qDebug("<MacOptions> loaded.");
  }
  else {
    if (autoCreateMacCsoundSections && baseRet==1) {
      QString defaultMacOptions = "<MacOptions>\nVersion: 3\nRender: Real\nAsk: Yes\nFunctions: ioObject\nListing: Window\nWindowBounds: 72 179 400 200\nCurrentView: io\nIOViewEdit: On\nOptions:\n</MacOptions>\n";
      m_macOptions = defaultMacOptions.split('\n');
      applyMacOptions(m_macOptions);
    }
    else {
      m_macOptions = QStringList();
    }
  }
  // Load Live Event Panels ------------------------
  while (text.contains("<EventPanel") and text.contains("</EventPanel>")) {
    QString liveEventsText = text.mid(text.indexOf("<EventPanel "),
                                      text.indexOf("</EventPanel>") - text.indexOf("<EventPanel ") + 13
                                      + (text[text.indexOf("</EventPanel>") + 14] == '\n' ? 1:0));
    QDomDocument doc("doc");
    doc.setContent(liveEventsText);
    QDomElement panelElement = doc.firstChildElement("EventPanel");
    QString liveText = panelElement.text();
//    qDebug() << "BaseDocument::setTextString   " << liveText;
    QString panelName = panelElement.attribute("name","");
    double tempo = panelElement.attribute("tempo","60.0").toDouble();
    double loop = panelElement.attribute("loop","8.0").toDouble();
    int posx = panelElement.attribute("x","-1").toDouble();
    int posy = panelElement.attribute("y","-1").toDouble();
    int width = panelElement.attribute("width","-1").toDouble();
    int height = panelElement.attribute("height","-1").toDouble();
    int visibleEnabled = panelElement.attribute("visible","true") == "true";
    int loopStart = panelElement.attribute("loopStart","0.0").toDouble();
    int loopEnd = panelElement.attribute("loopEnd","0.0").toDouble();

    LiveEventFrame *panel = createLiveEventPanel();
    panel->setFromText(liveText);
    panel->setName(panelName);
    panel->setTempo(tempo);
    panel->setLoopLength(loop);
    panel->setLoopRange(loopStart, loopEnd);
    m_liveEventControl->appendPanel(visibleEnabled, false, false, 0, panelName,
                                    loop, loopStart, loopEnd ,tempo);
    if (posx > 5 && posy > 5) {
      panel->move(posx, posy);
    }
    if (width > 100 && height > 80) {
      panel->resize(width, height);
    }
    panel->hide();
    text.remove(liveEventsText);
  }
//  if (m_liveFrames.size() == 0) {
//    LiveEventFrame *e = createLiveEventPanel();
//    e->setFromText(QString()); // Must set blank for undo history point
//  }
  m_view->setFullText(text,true);  // This must be last as some of the text has been removed along the way
  m_view->setModified(false);
  return ret;
}

void DocumentPage::setEditorFocus()
{
  m_view->setFocus();
}

void DocumentPage::insertText(QString text, int section)
{
  return m_view->insertText(text, section);
}

void DocumentPage::setFullText(QString text)
{
  return m_view->setFullText(text);
}

void DocumentPage::setBasicText(QString text)
{
  return m_view->setBasicText(text);
}

void DocumentPage::setOrc(QString text)
{
  return m_view->setOrc(text);
}

void DocumentPage::setSco(QString text)
{
  return m_view->setSco(text);
}

void DocumentPage::setLadspaText(QString text)
{
  return m_view->setLadspaText(text);
}

QString DocumentPage::getFullText()
{
  QString fullText;
  fullText = m_view->getFullText();
//  if (!fullText.endsWith("\n"))
//    fullText += "\n";
  if (fileName.endsWith(".csd",Qt::CaseInsensitive) or fileName == "") {
    fullText += getWidgetsText() ;
    fullText += getPresetsText() + "\n";
    if (saveOldFormat) {
      QString macOptions = getMacOptionsText();
      if (!macOptions.isEmpty()) {
        fullText += macOptions + "\n";
      }
      QString macWidgets = getMacWidgetsText();
      if (!macWidgets.isEmpty()) {
        fullText += macWidgets + "\n";
      }
      QString macPresets = getMacPresetsText();
      if (!macPresets.isEmpty()) {
        fullText += macPresets + "\n";  // Put old format for backward compatibility
      }
    }
    QString liveEventsText = "";
    if (saveLiveEvents) { // Only add live events sections if file is a csd file
      for (int i = 0; i < m_liveFrames.size(); i++) {
        liveEventsText += m_liveFrames[i]->getPlainText();
//        qDebug() << "DocumentPage::getFullText() " <<panel;
      }
      fullText += liveEventsText;
    }
  }
  else { // Not a csd file
    foreach (WidgetLayout * wl, m_widgetLayouts) {
      wl->clearWidgets(); // make sure no widgets are used.
    }
  }
  if (m_lineEnding == 1) { // Windows line ending mode
    fullText.replace("\n", "\r\n");
  }
  return fullText;
}

QString DocumentPage::getBasicText()
{
  QString text = m_view->getBasicText();
  return text;
}

QString DocumentPage::getSelectedText(int section)
{
  QString text = m_view->getSelectedText(section);
  return text;
}

QString DocumentPage::getOrc()
{
  QString text = m_view->getOrc();
  return text;
}

QString DocumentPage::getSco()
{
  QString text = m_view->getSco();
  return text;
}

QString DocumentPage::getOptionsText()
{
  QString text = m_view->getOptionsText();
  return text;
}

QString DocumentPage::getDotText()
{
  if (fileName.endsWith("sco")) {
    qDebug() << "DocumentPage::getDotText(): No dot for sco files";
    return QString();
  }
  QString orcText = getFullText();
  if (!fileName.endsWith("orc")) { //asume csd
    if (orcText.contains("<CsInstruments>") && orcText.contains("</CsInstruments>")) {
      orcText = orcText.mid(orcText.indexOf("<CsInstruments>") + 15,
                            orcText.indexOf("</CsInstruments>") - orcText.indexOf("<CsInstruments>") - 15);
    }
  }
  DotGenerator dot(fileName, orcText, m_opcodeTree);
  return dot.getDotText();
}

QString DocumentPage::getWidgetsText()
{
  //FIXME allow multiple
  QString text = m_widgetLayouts[0]->getWidgetsText();
  QDomDocument d;
  d.setContent(text);
//  QDomElement n = d.firstChildElement("bsbPanel");
//  if (!n.isNull()) {
//  }
  return d.toString();
}

QString DocumentPage::getPresetsText()
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->getPresetsText();
}

QString DocumentPage::getMacWidgetsText()
{
  QString t = getWidgetsText();  // TODO only for testing. remove later
  //FIXME allow multiple
  return m_widgetLayouts[0]->getMacWidgetsText();
}

QString DocumentPage::getMacPresetsText()
{
  return m_macPresets;
}

QString DocumentPage::getMacOptionsText()
{
  return m_macOptions.join("\n");
}

QString DocumentPage::getMacOptions(QString option)
{
  if (!option.endsWith(":"))
    option += ":";
  if (!option.endsWith(" "))
    option += " ";
  int index = m_macOptions.indexOf(QRegExp(option + ".*"));
  if (index < 0) {
    qDebug("DocumentPage::getMacOptions() Option %s not found!", option.toStdString().c_str());
    return QString("");
  }
  return m_macOptions[index].mid(option.size());
}

QString DocumentPage::getLiveEventsText()
{
  QString text;
  for (int i = 0; i < m_liveFrames.size(); i++) {
    text += m_liveFrames[i]->getPlainText();
  }
  return text;
}

QString DocumentPage::wordUnderCursor()
{
  return m_view->wordUnderCursor();
}

QRect DocumentPage::getWidgetPanelGeometry()
{
  //FIXME allow multiple
   return m_widgetLayouts[0]->getOuterGeometry();
}

void DocumentPage::setChannelValue(QString channel, double value)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->setValue(channel, value);
}

double DocumentPage::getChannelValue(QString channel)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->getValueForChannel(channel);
}

void DocumentPage::setChannelString(QString channel, QString value)
{
  for (int i = 0; i < m_widgetLayouts.size(); i++) {
    m_widgetLayouts[i]->setValue(channel, value);
  }
}

QString DocumentPage::getChannelString(QString channel)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->getStringForChannel(channel);
}

void DocumentPage::setWidgetProperty(QString channel, QString property, QVariant value)
{
  for (int i = 0; i < m_widgetLayouts.size(); i++) {
    m_widgetLayouts[i]->setWidgetProperty(channel, property, value);
  }
}

QVariant DocumentPage::getWidgetProperty(QString channel, QString property)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->getWidgetProperty(channel, property);
}

QString DocumentPage::createNewLabel(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewLabel(x, y);
}

QString DocumentPage::createNewDisplay(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewDisplay(x, y);
}

QString DocumentPage::createNewScrollNumber(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewScrollNumber(x, y);
}

QString DocumentPage::createNewLineEdit(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewLineEdit(x, y);
}

QString DocumentPage::createNewSpinBox(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewSpinBox(x, y);
}

QString DocumentPage::createNewSlider(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewSlider(x, y);
}

QString DocumentPage::createNewButton(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewButton(x, y);
}

QString DocumentPage::createNewKnob(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewKnob(x, y);
}

QString DocumentPage::createNewCheckBox(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewCheckBox(x, y);
}

QString DocumentPage::createNewMenu(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewMenu(x, y);
}

QString DocumentPage::createNewMeter(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewMeter(x, y);
}

QString DocumentPage::createNewConsole(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewConsole(x, y);
}

QString DocumentPage::createNewGraph(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewGraph(x, y);
}

QString DocumentPage::createNewScope(int x, int y)
{
  //FIXME allow multiple
  return m_widgetLayouts[0]->createNewScope(x, y);
}


EventSheet* DocumentPage::getSheet(int sheetIndex)
{
  if (sheetIndex == -1) {
    sheetIndex = 0; //FIXME find sheet
  }
  if (sheetIndex < m_liveFrames.size() && sheetIndex >= 0) {
    return m_liveFrames[sheetIndex]->getSheet();
  }
  return 0;
}

EventSheet* DocumentPage::getSheet(QString sheetName)
{
  return 0;
}
//
//void *DocumentPage::getCsound()
//{
//  return m_csEngine->getCsound();
//}

int DocumentPage::lineCount(bool countExtras)
{
  QString text;
  if (countExtras) {
    text = this->getBasicText();
  }
  else
  {
    text = this->getFullText();
  }
  return text.count("\n");
}

int DocumentPage::characterCount(bool countExtras)
{
  QString text;
  if (countExtras) {
    text = this->getBasicText();
  }
  else
  {
    text = this->getFullText();
  }
  return text.size();
}

int DocumentPage::instrumentCount()
{
  QString text = this->getBasicText();
  return text.count((QRegExp("\\n[ \\t]*instr\\s")));
}

int DocumentPage::udoCount()
{
  QString text = this->getBasicText();
  return text.count((QRegExp("\\n[ \\t]*opcode\\s")));
}

int DocumentPage::widgetCount()
{
  QString text = this->getWidgetsText();
  return text.count("<bsbObject");
}

QString DocumentPage::getFilePath()
{
  return fileName.left(fileName.lastIndexOf("/"));
}

QStringList DocumentPage::getScheduledEvents(unsigned long ksmps)
{
  QStringList events;
  // Called once after every Csound control pass
  for (int i = 0; i < m_liveFrames.size(); i++) {
    m_liveFrames[i]->getEvents(ksmps, &events);
  }
//  m_ksmpscount = ksmpscount;
  return events;
}

void DocumentPage::setModified(bool mod)
{
  // This slot is triggered by the document children whenever they are modified
  // It is also called from the main application when the file is saved to set as unmodified.
  // FIXME live frame modification should also affect here
//  qDebug() << "DocumentPage::setModified(bool mod) "<< mod;
  if (mod == true) {
    emit modified();
  }
  else {
    m_view->setModified(false);
    foreach (WidgetLayout  *wl, m_widgetLayouts) {
      wl->setModified(false);
    }
    for (int i = 0; i < m_liveFrames.size(); i++) {
      m_liveFrames[i]->setModified(false);
    }
  }
}

bool DocumentPage::isModified()
{
  if (m_view->isModified())
    return true;
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    if (wl->isModified())
      return true;
  }
  for (int i = 0; i < m_liveFrames.size(); i++) {
    if (m_liveFrames[i]->isModified())
      return true;
  }
  return false;
}

bool DocumentPage::isRunning()
{
  // TODO what to do with pause?
  return m_csEngine->isRunning() || m_pythonRunning;
}

bool DocumentPage::isRecording()
{
  // TODO what to do with pause?
  return m_csEngine->isRecording();
}

bool DocumentPage::usesFltk()
{
  return m_view->getBasicText().contains("FLpanel");
}

void DocumentPage::updateCsLadspaText()
{
  QString text = "<csLADSPA>\nName=";
  text += fileName.mid(fileName.lastIndexOf("/") + 1) + "\n";
  text += "Maker=QuteCsound\n";
  QString id = QString::number(qrand());
  text += "UniqueID=" + id + "\n";
  text += "Copyright=none\n";
  //FIXME allow multiple
  text += m_widgetLayouts[0]->getCsladspaLines();
  text += "</csLADSPA>";
  m_view->setLadspaText(text);
}

void DocumentPage::focusWidgets()
{
  //FIXME allow multiple
  //TODO is this really required?
  m_widgetLayouts[0]->setFocus();
}

QString DocumentPage::getFileName()
{
  return fileName;
}

QString DocumentPage::getCompanionFileName()
{
  return companionFile;
}

void DocumentPage::setLineEnding(int lineEndingMode)
{
  m_lineEnding = lineEndingMode;
}

void DocumentPage::copy()
{
  qDebug() << "DocumentPage::copy() " << m_widgetLayouts[0]->hasFocus();
  bool liveeventfocus = false;
  for (int i = 0; i < m_liveFrames.size(); i++) {
    if (m_liveFrames[i]->getSheet()->hasFocus()) {
      m_liveFrames[i]->getSheet()->copy();
      liveeventfocus = true;
    }
  }
  if (!liveeventfocus) {
    if (m_view->hasFocus()) {
      m_view->copy();
    }
    else  { // FIXME allow multiple layouts
      m_widgetLayouts[0]->copy();
    }
  }
}

void DocumentPage::cut()
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    if (wl->hasFocus()) {
      wl->cut();
      return;
    }
  }
  if (m_view->hasFocus())
    m_view->cut();
  else {
    for (int i = 0; i < m_liveFrames.size(); i++) {
      if (m_liveFrames[i]->getSheet()->hasFocus())
        m_liveFrames[i]->getSheet()->cut();
    }
  }
}

void DocumentPage::paste()
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    if (wl->hasFocus()) {
      wl->paste();
      return;
    }
  }
  if (m_widgetLayouts[0]->hasFocus())
    m_widgetLayouts[0]->paste();
  else if (m_view->hasFocus())
    m_view->paste();
  else {
    for (int i = 0; i < m_liveFrames.size(); i++) {
      if (m_liveFrames[i]->getSheet()->hasFocus())
        m_liveFrames[i]->getSheet()->paste();
    }
  }
}

void DocumentPage::undo()
{
//  qDebug() << "DocumentPage::undo()";
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    if (wl->hasFocus()) {
      wl->undo();
      wl->setFocus(Qt::OtherFocusReason);  // Why is this here?
      return;
    }
  }
  if (m_widgetLayouts[0]->hasFocus()) {
  }
  else if (m_view->hasFocus()) {
    m_view->undo();
  }
  else {
    for (int i = 0; i < m_liveFrames.size(); i++) {
      if (m_liveFrames[i]->getSheet()->hasFocus())
        m_liveFrames[i]->getSheet()->undo();
    }
  }
}

void DocumentPage::redo()
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    if (wl->hasFocus()) {
      wl->redo();
      wl->setFocus(Qt::OtherFocusReason);  // Why is this here?
      return;
    }
  }
  if (m_view->hasFocus())
    m_view->redo();
  else {
    for (int i = 0; i < m_liveFrames.size(); i++) {
      if (m_liveFrames[i]->getSheet()->hasFocus())
        m_liveFrames[i]->getSheet()->redo();
    }
  }
}

DocumentView *DocumentPage::getView()
{
  Q_ASSERT(m_view != 0);
  return m_view;
}

//void DocumentPage::setWidgetLayout(WidgetLayout *w)
//{
//  if (w != 0) {
//    m_widgetLayout = w;
//  }
//  else {
//    qDebug() << "DocumentPage::setWidgetLayout()  NULL widget.";
//  }
//}

CsoundEngine *DocumentPage::getEngine()
{
  return m_csEngine;
}

void DocumentPage::setTextFont(QFont font)
{
  m_view->setFont(font);
}

void DocumentPage::setTabStopWidth(int tabWidth)
{
  m_view->setTabStopWidth(tabWidth);
}

void DocumentPage::setLineWrapMode(QTextEdit::LineWrapMode wrapLines)
{
  m_view->setLineWrapMode(wrapLines);
}

void DocumentPage::setColorVariables(bool colorVariables)
{
  m_view->setColorVariables(colorVariables);
}

void DocumentPage::setAutoComplete(bool autoComplete)
{
  m_view->setAutoComplete(autoComplete);
}

QString DocumentPage::getActiveSection()
{
  return m_view->getActiveSection();
}

QString DocumentPage::getActiveText()
{
  return m_view->getActiveText();
}

void DocumentPage::print(QPrinter *printer)
{
  m_view->print(printer);
}

void DocumentPage::findReplace()
{
  m_view->findReplace();
}

void DocumentPage::findString()
{
  m_view->findString();
}

void DocumentPage::getToIn()
{
  m_view->getToIn();
}

void DocumentPage::inToGet()
{
  m_view->inToGet();
}

//void DocumentPage::setEditAct(QAction *editAct)
//{
//  m_widgetLayout->setEditAct(editAct);
//}

//void DocumentPage::setCsoundOptions(CsoundOptions &options)
//{
//  m_csEngine->setCsoundOptions(options);
//}

void DocumentPage::showWidgetTooltips(bool visible)
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    wl->showWidgetTooltips(visible);
  }
}

void DocumentPage::setKeyRepeatMode(bool keyRepeat)
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    wl->setKeyRepeatMode(keyRepeat);
  }
  m_console->setKeyRepeatMode(keyRepeat);
}

void DocumentPage::setOpenProperties(bool open)
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    wl->setProperty("openProperties", open);
  }
}

void DocumentPage::setFontOffset(double offset)
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    wl->setFontOffset(offset);
  }
}

void DocumentPage::setFontScaling(double offset)
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    wl->setFontScaling(offset);
  }
}

void DocumentPage::passWidgetClipboard(QString text)
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    wl->passWidgetClipboard(text);
  }
}

void DocumentPage::setConsoleFont(QFont font)
{
  m_console->setDefaultFont(font);
}

void DocumentPage::setConsoleColors(QColor fontColor, QColor bgColor)
{
  m_console->setColors(fontColor, bgColor);
}

void DocumentPage::setInitialDir(QString initialDir)
{
  m_csEngine->setInitialDir(initialDir);
}

//DocumentView * DocumentPage::view()
//{
//  return m_view;
//}
//
//CsoundEngine *DocumentPage::engine()
//{
//  return m_csEngine;
//}
//
//WidgetLayout * DocumentPage::widgetLayout()
//{
//  return m_widgetLayout;
//}

void DocumentPage::setScriptDirectory(QString dir)
{
  for (int i = 0; i < m_liveFrames.size(); i++) {
    m_liveFrames[i]->getSheet()->setScriptDirectory(dir);
  }
}

void DocumentPage::setDebugLiveEvents(bool debug)
{
  for (int i = 0; i < m_liveFrames.size(); i++) {
    m_liveFrames[i]->getSheet()->setDebug(debug);
  }
}

void DocumentPage::setConsoleBufferSize(int size)
{
  m_csEngine->setConsoleBufferSize(size);
}

void DocumentPage::setWidgetEnabled(bool enabled)
{
  // TODO disable widgetLayout if its not being used?
//  m_widgetLayout->setEnabled(enabled);
  m_csEngine->enableWidgets(enabled);
}

void DocumentPage::setRunThreaded(bool threaded)
{
  m_csEngine->setThreaded(threaded);  // This will take effect on next run of the engine
}

void DocumentPage::useInvalue(bool use)
{
  m_csEngine->useInvalue(use);  // This will take effect on next run of the engine
}

void DocumentPage::useOldFormat(bool use)
{
//  qDebug() << "DocumentPage::useXmlFormat " << use;
  saveOldFormat = use;
}

void DocumentPage::setPythonExecutable(QString pythonExec)
{
  m_pythonExecutable = pythonExec;
}

void DocumentPage::showLiveEventPanels(bool visible)
{
  if (fileName.endsWith(".csd")) {
    m_liveEventControl->setVisible(visible);
    for (int i = 0; i < m_liveFrames.size(); i++) {
      if (visible) {
        //        qDebug() << "DocumentPage::showLiveEventPanels  " << visible << (int) this;
        if (m_liveFrames[i]->isVisible())
          m_liveFrames[i]->raise();
        else {
          if (m_liveFrames[i]->getVisibleEnabled()) {
            m_liveFrames[i]->setWindowFlags(Qt::Window);
            m_liveFrames[i]->show();
            m_liveFrames[i]->raise();
          }
        }
      }
      else {
        m_liveFrames[i]->setWindowFlags(Qt::Widget);
        m_liveFrames[i]->hide();
      }
    }
  }
}

void DocumentPage::stopAllSlot()
{
  for (int i = 0; i < m_liveFrames.size(); i++) {
    m_liveFrames[i]->getSheet()->stopAllEvents();
  }
}

void DocumentPage::newPanelSlot()
{
  newLiveEventPanel();
}

void DocumentPage::playPanelSlot(int index)
{
   m_liveFrames[index]->getSheet()->sendAllEvents();
}

void DocumentPage::loopPanelSlot(int index, bool loop)
{
   m_liveFrames[index]->loopPanel(loop);
}

void DocumentPage::stopPanelSlot(int index)
{
  qDebug() << "DocumentPage::stopPanelSlot not implemented";
}

void DocumentPage::setPanelVisibleSlot(int index, bool visible)
{
   if (visible) {
     m_liveFrames[index]->setWindowFlags(Qt::Window);
   }
   else {
     m_liveFrames[index]->setWindowFlags(Qt::Widget);
   }
   m_liveFrames[index]->setVisible(visible);
   m_liveFrames[index]->setVisibleEnabled(visible);
}

void DocumentPage::setPanelSyncSlot(int index, int mode)
{
  qDebug() << "DocumentPage::setPanelSyncSlot not implemented";
}

void DocumentPage::setPanelNameSlot(int index, QString name)
{
   m_liveFrames[index]->setName(name);
}

void DocumentPage::setPanelTempoSlot(int index, double tempo)
{
   m_liveFrames[index]->setTempo(tempo);
}

void DocumentPage::setPanelLoopLengthSlot(int index, double length)
{
   m_liveFrames[index]->setLoopLength(length);
}

void DocumentPage::setPanelLoopRangeSlot(int index, double start, double end)
{
   m_liveFrames[index]->setLoopRange(start,end);
}

void DocumentPage::registerButton(QuteButton *b)
{
//  qDebug() << " DocumentPage::registerButton";
  connect(b, SIGNAL(play()), static_cast<qutecsound *>(parent()), SLOT(play()));
  connect(b, SIGNAL(render()), static_cast<qutecsound *>(parent()), SLOT(render()));
  connect(b, SIGNAL(pause()), static_cast<qutecsound *>(parent()), SLOT(pause()));
  connect(b, SIGNAL(stop()), static_cast<qutecsound *>(parent()), SLOT(stop()));
}

void DocumentPage::init(QWidget *parent, OpEntryParser *opcodeTree)
{
  fileName = "";
  companionFile = "";
  askForFile = true;
  readOnly = false;

  //TODO this should be set from QuteCsound configuration
  saveLiveEvents = true;

  m_view = new DocumentView(parent, opcodeTree);
  connect(m_view, SIGNAL(evaluate(QString)), this, SLOT(evaluatePython(QString)));

  // For logging of Csound output to file
  connect(m_console, SIGNAL(logMessage(QString)),
          static_cast<qutecsound *>(parent), SLOT(logMessage(QString)));

//  m_widgetLayout->show();
  m_liveEventControl = new LiveEventControl(parent);
  m_liveEventControl->hide();
  connect(m_liveEventControl, SIGNAL(closed()), this, SLOT(liveEventControlClosed()));
  connect(m_liveEventControl, SIGNAL(stopAll()), this, SLOT(stopAllSlot()));
  connect(m_liveEventControl, SIGNAL(newPanel()), this, SLOT(newPanelSlot()));
  connect(m_liveEventControl, SIGNAL(playPanel(int)), this, SLOT(playPanelSlot(int)));
  connect(m_liveEventControl, SIGNAL(loopPanel(int,bool)), this, SLOT(loopPanelSlot(int,bool)));
  connect(m_liveEventControl, SIGNAL(stopPanel(int)), this, SLOT(stopPanelSlot(int)));
  connect(m_liveEventControl, SIGNAL(setPanelVisible(int,bool)), this, SLOT(setPanelVisibleSlot(int,bool)));
  connect(m_liveEventControl, SIGNAL(setPanelSync(int,int)), this, SLOT(setPanelSyncSlot(int,int)));
  connect(m_liveEventControl, SIGNAL(setPanelNameSignal(int,QString)), this, SLOT(setPanelNameSlot(int,QString)));
  connect(m_liveEventControl, SIGNAL(setPanelTempoSignal(int,double)), this, SLOT(setPanelTempoSlot(int,double)));
  connect(m_liveEventControl, SIGNAL(setPanelLoopLengthSignal(int,double)), this, SLOT(setPanelLoopLengthSlot(int,double)));
  connect(m_liveEventControl, SIGNAL(setPanelLoopRangeSignal(int,double,double)), this, SLOT(setPanelLoopRangeSlot(int,double,double)));

  // Connect for clearing marked lines and letting inspector know text has changed
  connect(m_view, SIGNAL(contentsChanged()), this, SLOT(textChanged()));
  connect(m_view, SIGNAL(opcodeSyntaxSignal(QString)), this, SLOT(opcodeSyntax(QString)));
//   connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(moved()));

  connect(m_csEngine, SIGNAL(errorLines(QList<QPair<int, QString> >)),
          m_view, SLOT(markErrorLines(QList<QPair<int, QString> >)));
  connect(m_csEngine, SIGNAL(stopSignal()),
          this, SLOT(perfEnded()));


  //FIXME widgetlayout should have the chance of being empty
//  m_widgetLayouts.append(newWidgetLayout());
  m_csEngine->setWidgetLayout(m_widgetLayouts[0]);  // Pass first widget layout to engine

//  detachWidgets();
  saveOldFormat = true; // save Mac widgets by default
  m_pythonRunning = false;
}

void DocumentPage::deleteAllLiveEvents()
{
  for (int i = 0; i < m_liveFrames.size(); i++) {
    deleteLiveEventPanel(m_liveFrames[i]);
  }
}

WidgetLayout* DocumentPage::newWidgetLayout()
{
  WidgetLayout* wl = BaseDocument::newWidgetLayout();
  connect(wl, SIGNAL(changed()), this, SLOT(setModified()));
  connect(wl, SIGNAL(setWidgetClipboardSignal(QString)),
        this, SLOT(setWidgetClipboard(QString)));
  return wl;
}

int DocumentPage::play(CsoundOptions *options)
{
  if (fileName.endsWith(".py")) {
    m_console->reset(); // Clear consoles
    return runPython();
  }
  else {
    m_view->unmarkErrorLines();  // Clear error lines when running
    return BaseDocument::play(options);
  }
}

void DocumentPage::stop()
{
  BaseDocument::stop();
  if (m_pythonRunning == true) {
    m_pythonRunning = false;
  }
}

int DocumentPage::record(int format)
{
  if (fileName.startsWith(":/")) {
    QMessageBox::critical(static_cast<QWidget *>(parent()),
                          tr("QuteCsound"),
                          tr("You must save the examples to use Record."),
                          QMessageBox::Ok);
    return -1;
  }
  int number = 0;
  QString recName = fileName + "-000.wav";
  while (QFile::exists(recName)) {
    number++;
    recName = fileName + "-";
    if (number < 10)
      recName += "0";
    if (number < 100)
      recName += "0";
    recName += QString::number(number) + ".wav";
  }
  emit setCurrentAudioFile(recName);
  return m_csEngine->startRecording(format, recName);
}

void DocumentPage::perfEnded()
{
  emit stopSignal();
}

int DocumentPage::runPython()
{
  QProcess p;
  QDir::setCurrent(fileName.mid(fileName.lastIndexOf("/") + 1));
  p.start(m_pythonExecutable + " \"" + fileName + "\"");
  m_pythonRunning = true;
  while (!p.waitForFinished (100) && m_pythonRunning) {
    // TODO make stop button stop python too
    QByteArray sout = p.readAllStandardOutput();
    QByteArray serr = p.readAllStandardError();
    m_console->appendMessage(sout);
    m_console->appendMessage(serr);
    qApp->processEvents();
  }
  QByteArray sout = p.readAllStandardOutput();
  QByteArray serr = p.readAllStandardError();
  m_console->appendMessage(sout);
  m_console->appendMessage(serr);
  emit stopSignal();
  return p.exitCode();
}

void DocumentPage::showWidgets(bool show)
{
//  qDebug() << "DocumentPage::showWidgets" << show;
  if (!show) {
    hideWidgets();
    return;
  }
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    wl->setVisible(true);
    wl->raise();
  }
}

void DocumentPage::hideWidgets()
{
//  qDebug() << " DocumentPage::hideWidgets()";
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    wl->setVisible(false);
  }
}

//void DocumentPage::detachWidgets()
//{
//  foreach (WidgetLayout *wl, m_widgetLayouts) {
////    wl->setParent(static_cast<QWidget *>(parent()));
//    qDebug() << "DocumentPage::detachWidgets() " << wl;
//    wl->setParent(0);
//    wl->setWindowFlags(Qt::Window);
//  }
//}
//
//void DocumentPage::attachWidgets(QDockWidget *panel)
//{
//  panel->setWidget(m_widgetLayouts[0]);
////  foreach (WidgetLayout *wl, m_widgetLayouts) {
////  }
//  //FIXME fix when implementing multiple widget panels
//}

//void DocumentPage::setMacWidgetsText(QString widgetText)
//{
////   qDebug() << "DocumentPage::setMacWidgetsText: ";
//
////   document()->setModified(true);
//}


void DocumentPage::applyMacOptions(QStringList options)
{
  int index = options.indexOf(QRegExp("WindowBounds: .*"));
  if (index > 0) {
    QString line = options[index];
    QStringList values = line.split(" ");
    values.removeFirst();  //remove property name
    // FIXME allow multiple layouts
    m_widgetLayouts[0]->setOuterGeometry(values[0].toInt(),values[1].toInt(),
                                     values[2].toInt(), values[3].toInt());
  }
  else {
    qDebug ("DocumentPage::applyMacOptions() no Geometry!");
  }
}

void DocumentPage::setMacOption(QString option, QString newValue)
{
  if (!option.endsWith(":"))
    option += ":";
  if (!option.endsWith(" "))
    option += " ";
  int index = m_macOptions.indexOf(QRegExp(option + ".*"));
  if (index < 0) {
    qDebug("DocumentPage::setMacOption() Option not found!");
    return;
  }
  m_macOptions[index] = option + newValue;
  qDebug("DocumentPage::setMacOption() %s", m_macOptions[index].toStdString().c_str());
}

void DocumentPage::setWidgetPanelPosition(QPoint position)
{
  //FIXME allow multiple layout
  m_widgetLayouts[0]->setOuterGeometry(position.x(), position.y(), -1, -1);
  int index = m_macOptions.indexOf(QRegExp("WindowBounds: .*"));
  if (index < 0) {
//    qDebug ("DocumentPage::getWidgetPanelGeometry() no Geometry!");
    return;
  }
  QStringList parts = m_macOptions[index].split(" ");
  parts.removeFirst();
  QString newline = "WindowBounds: " + QString::number(position.x()) + " ";
  newline += QString::number(position.y()) + " ";
  newline += parts[2] + " " + parts[3];
  m_macOptions[index] = newline;

//   qDebug("DocumentPage::setWidgetPanelPosition() %i %i", position.x(), position.y());
}

void DocumentPage::setWidgetPanelSize(QSize size)
{
  // Slot called from resizing Widget panel container
  //FIXME allow multiple layouts
//  qDebug("DocumentPage::setWidgetPanelSize() %i %i", size.width(), size.height());
  m_widgetLayouts[0]->setOuterGeometry(-1, -1, size.width(), size.height());
  int index = m_macOptions.indexOf(QRegExp("WindowBounds: .*"));
  if (index < 0) {
//    qDebug ("DocumentPage::getWidgetPanelGeometry() no Geometry!");
    return;
  }
  QStringList parts = m_macOptions[index].split(" ");
  parts.removeFirst();
  QString newline = "WindowBounds: ";
  newline += parts[0] + " " + parts[1] + " ";
  newline += QString::number(size.width()) + " ";
  newline += QString::number(size.height());
  m_macOptions[index] = newline;
}

void DocumentPage::setWidgetEditMode(bool active)
{
//  qDebug() << "DocumentPage::setWidgetEditMode";
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    wl->setEditMode(active);
  }
}

//void DocumentPage::toggleWidgetEditMode()
//{
//  qDebug() << "DocumentPage::toggleWidgetEditMode";
//  m_widgetLayout->toggleEditMode();
//}

void DocumentPage::duplicateWidgets()
{
  foreach (WidgetLayout *wl, m_widgetLayouts) {
    if (wl->hasFocus()) {
      wl->duplicate();
    }
  }
}

void DocumentPage::jumpToLine(int line)
{
//  qDebug() << "DocumentPage::jumpToLine " << line;
  m_view->jumpToLine(line);
}

void DocumentPage::comment()
{
//  qDebug() << "DocumentPage::comment()";
  m_view->comment();
}

void DocumentPage::uncomment()
{
  m_view->uncomment();
}

void DocumentPage::indent()
{
  m_view->indent();
}

void DocumentPage::unindent()
{
  m_view->unindent();
}

void DocumentPage::killToEnd()
{
  m_view->killToEnd();
}

void DocumentPage::killLine()
{
  m_view->killLine();
}

void DocumentPage::autoComplete()
{
  m_view->autoComplete();
}

void DocumentPage::newLiveEventPanel(QString text)
{
  LiveEventFrame *e = createLiveEventPanel(text);
  m_liveEventControl->appendPanel(true, false, false, 0, "",
                                    8.0, 0.0, 0.0 , 60.0);
  e->setWindowFlags(Qt::Window);
//  e->setVisible(false);
  e->show();  //Assume that since slot was called must be visible
}

LiveEventFrame * DocumentPage::createLiveEventPanel(QString text)
{
//  qDebug() << "DocumentPage::newLiveEventPanel()";
  LiveEventFrame *e = new LiveEventFrame("Live Event", m_liveEventControl, Qt::Window);
//  e->setVisible(false);
//  e->setAttribute(Qt::WA_DeleteOnClose, false);
  e->getSheet()->setRowCount(1);
  e->getSheet()->setColumnCount(6);

  if (!text.isEmpty()) {
    e->setFromText(text);
  }
  m_liveFrames.append(e);
//  connect(e, SIGNAL(closed()), this, SLOT(liveEventFrameClosed()));
  connect(e, SIGNAL(newFrameSignal(QString)), this, SLOT(newLiveEventPanel(QString)));
  connect(e, SIGNAL(deleteFrameSignal(LiveEventFrame *)), this, SLOT(deleteLiveEventPanel(LiveEventFrame *)));
  connect(e, SIGNAL(renamePanel(LiveEventFrame *, QString)), this, SLOT(renamePanel(LiveEventFrame *,QString)));
  connect(e, SIGNAL(setLoopRangeFromPanel(LiveEventFrame *, double, double)),
          this, SLOT(setPanelLoopRange(LiveEventFrame *,double, double)));
  connect(e, SIGNAL(setTempoFromPanel(LiveEventFrame *, double)),
          this, SLOT(setPanelTempo(LiveEventFrame *,double)));
  connect(e, SIGNAL(setLoopLengthFromPanel(LiveEventFrame *, double)),
          this, SLOT(setPanelLoopLength(LiveEventFrame *,double)));
  connect(e->getSheet(), SIGNAL(sendEvent(QString)),this,SLOT(queueEvent(QString)));
  connect(e->getSheet(), SIGNAL(modified()),this,SLOT(setModified()));
  return e;
}

void DocumentPage::deleteLiveEventPanel(LiveEventFrame *frame)
{
//  qDebug() << "deleteLiveEventPanel(LiveEventFrame *frame)";
  int index = m_liveFrames.indexOf(frame);
  if (index >= 0) {  // Frames should already have been deleted by parent document view widget
    if (frame != 0) {
      disconnect(frame, 0,0,0);
      disconnect(frame->getSheet(), 0,0,0);
      m_liveFrames.removeAt(index);
      m_liveEventControl->removePanel(index);
      frame->close();
    }
  }
  else {
    qDebug() << "DocumentPage::deleteLiveEventPanel frame not found";
  }
}

void DocumentPage::textChanged()
{
//  qDebug() << "DocumentPage::textChanged()";
  setModified(true);
  emit currentTextUpdated();
}

//void DocumentPage::liveEventFrameClosed()
//{
////  qDebug() << "DocumentPage::liveEventFrameClosed()";
//  LiveEventFrame *e = dynamic_cast<LiveEventFrame *>(QObject::sender());
////  if (e != 0) { // This shouldn't really be necessary but just in case
//  bool shown = false;
//  for (int i = 0; i < m_liveFrames.size(); i++) {
//    if (m_liveFrames[i]->isVisible()
//      && m_liveFrames[i] != e)  // frame that called has not been called yet
//      shown = true;
//  }
//  emit liveEventsVisible(shown);
////  }
//}

void DocumentPage::liveEventControlClosed()
{
  qDebug()<< "DocumentPage::liveEventControlClosed()";
  showLiveEventPanels(false);
  emit liveEventsVisible(false);
}

void DocumentPage::renamePanel(LiveEventFrame *panel,QString newName)
{
  int index = m_liveFrames.indexOf(panel);
  if (index >= 0) {
    m_liveEventControl->renamePanel(index, newName);
    panel->setName(newName);
  }
}

void DocumentPage::setPanelLoopRange(LiveEventFrame *panel, double start, double end)
{
  int index = m_liveFrames.indexOf(panel);
  if (index >= 0) {
    m_liveEventControl->setPanelLoopRange(index, start, end);
  }
}

void DocumentPage::setPanelLoopLength(LiveEventFrame *panel, double length)
{
  int index = m_liveFrames.indexOf(panel);
  if (index >= 0) {
    m_liveEventControl->setPanelLoopLength(index, length);
  }
}

void DocumentPage::setPanelTempo(LiveEventFrame *panel, double tempo)
{
  int index = m_liveFrames.indexOf(panel);
  if (index >= 0) {
    m_liveEventControl->setPanelTempo(index, tempo);
  }
}

void DocumentPage::opcodeSyntax(QString message)
{
  emit opcodeSyntaxSignal(message);
}

void DocumentPage::setWidgetClipboard(QString message)
{
  emit setWidgetClipboardSignal(message);
}

void DocumentPage::evaluatePython(QString code)
{
  emit evaluatePythonSignal(code);
}
