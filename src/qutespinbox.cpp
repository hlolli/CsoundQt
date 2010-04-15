/*
    Copyright (C) 2008, 2009 Andres Cabrera
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

#include "qutespinbox.h"
#include <cmath>


QuteSpinBox::QuteSpinBox(QWidget* parent) : QuteText(parent)
{
  delete m_widget; //delete widget created by parent constructor
  m_widget = new QDoubleSpinBox(this);
  m_widget->setContextMenuPolicy(Qt::NoContextMenu);
  m_widget->setMouseTracking(true); // Necessary to pass mouse tracking to widget panel for _MouseX channels
  static_cast<QDoubleSpinBox*>(m_widget)->setRange(-99999.999999, 99999.999999);
  connect(static_cast<QDoubleSpinBox *>(m_widget), SIGNAL(valueChanged(double)), this, SLOT(valueChanged(double)));
//   connect(static_cast<QDoubleSpinBox*>(m_widget), SIGNAL(popUpMenu(QPoint)), this, SLOT(popUpMenu(QPoint)));
  m_type = "editnum";

  setProperty("QCS_value", (double) 0.0);
  setProperty("QCS_resolution", (double) 0.1);
  setProperty("QCS_minimum",(double)  -999999999999.0);
  setProperty("QCS_maximum", (double) 99999999999999.0);
  setProperty("QCS_randomizable", false);
  setProperty("QCS_label", QVariant()); // Remove this property which is part of parent class.
}

QuteSpinBox::~QuteSpinBox()
{
}

void QuteSpinBox::setValue(double value)
{
#ifdef  USE_WIDGET_MUTEX
  mutex.lock();
#endif
  static_cast<QDoubleSpinBox*>(m_widget)->setValue(value);
#ifdef  USE_WIDGET_MUTEX
  mutex.unlock();
#endif
  setProperty("QCS_value", value);
}

void QuteSpinBox::setText(QString /*text*/)
{
  qDebug() << "QuteSpinBox::setText not valid! setting to minimum";
  setValue(0.0);
}

//void QuteSpinBox::setResolution(double resolution)
//{
//  setProperty("QCS_resolution", resolution);
//}

double QuteSpinBox::getValue()
{
  return static_cast<QDoubleSpinBox*>(m_widget)->value();
}

QString QuteSpinBox::getWidgetLine()
{
  QString line = "ioText {" + QString::number(property("QCS_x").toInt()) + ", " + QString::number(property("QCS_y").toInt()) + "} ";
  line += "{"+ QString::number(property("QCS_width").toInt()) +", "+ QString::number(property("QCS_height").toInt()) +"} ";
  line += m_type + " ";
  line += QString::number(static_cast<QDoubleSpinBox*>(m_widget)->value(), 'f', 6) + " ";
  line += QString::number(property("QCS_resolution").toDouble(), 'f', 6) + " \"" + m_widget->property("QCS_objectName").toString() + "\" ";
  line += property("QCS_alignment").toString() + " ";
  line += "\"" + m_widget->property("QCS_font").toString() + "\" "
          + QString::number(m_widget->property("QCS_fontsize").toInt()) + " ";
  QColor color = m_widget->palette().color(QPalette::WindowText);
  line += "{" + QString::number(color.red() * 256)
      + ", " + QString::number(color.green() * 256)
      + ", " + QString::number(color.blue() * 256) + "} ";
  color = m_widget->palette().color(QPalette::Window);
  line += "{" + QString::number(color.red() * 256)
      + ", " + QString::number(color.green() * 256)
      + ", " + QString::number(color.blue() * 256) + "} ";
  line += m_widget->autoFillBackground()? "background ":"nobackground ";
  line += "noborder ";
  line += QString::number(static_cast<QDoubleSpinBox*>(m_widget)->value(), 'f', 6);
  // For this type of ioText widget, value and text are redundant. QuteCsound reads mthe value coming
  // in the text field, but writes to both.
//   qDebug("QuteText::getWidgetLine() %s", line.toStdString().c_str());
  return line;
}

QString QuteSpinBox::getCsladspaLine()
{
  QString line = "ControlPort=" + m_widget->property("QCS_objectName").toString()
                 + "|" + m_widget->property("QCS_objectName").toString() + "\n";
  line += "Range=-9999|9999";
  return line;
}

QString QuteSpinBox::getWidgetXmlText()
{
  xmlText = "";
  QXmlStreamWriter s(&xmlText);
  createXmlWriter(s);
  // Not implemented in blue

  s.writeTextElement("type", m_type);
  s.writeTextElement("value", QString::number(static_cast<QDoubleSpinBox*>(m_widget)->value()));
  s.writeTextElement("resolution", QString::number(property("QCS_resolution").toDouble(), 'f', 8));

   //These are not implemented in blue
  s.writeTextElement("alignment", property("QCS_alignment").toString());

  s.writeTextElement("font", property("QCS_font").toString());
  s.writeTextElement("fontsize", QString::number(property("QCS_fontsize").toInt()));

  QColor color = property("QCS_color").value<QColor>();
  s.writeStartElement("color");
  s.writeTextElement("r", QString::number(color.red()));
  s.writeTextElement("g", QString::number(color.green()));
  s.writeTextElement("b", QString::number(color.blue()));
  s.writeEndElement();
  color = property("QCS_bgcolor").value<QColor>();
  s.writeStartElement("bgcolor");
  s.writeAttribute("mode", property("QCS_bgcolormode").toBool()? "background":"nobackground");
  s.writeTextElement("r", QString::number(color.red()));
  s.writeTextElement("g", QString::number(color.green()));
  s.writeTextElement("b", QString::number(color.blue()));
  s.writeEndElement();

  s.writeTextElement("minimum", QString::number(property("QCS_minimum").toDouble()));
  s.writeTextElement("maximum", QString::number(property("QCS_maximum").toDouble()));
  s.writeTextElement("bordermode", property("QCS_bordermode").toString());
  s.writeTextElement("borderradius", QString::number(property("QCS_borderradius").toInt()));
  s.writeTextElement("randomizable", property("QCS_randomizable").toBool() ? "true": "false");
  s.writeEndElement();

  return xmlText;
}

QString QuteSpinBox::getWidgetType()
{
  return QString("BSBSpinBox");
}

QString QuteSpinBox::getCabbageLine()
{
  QString line = "";
  return line;
}

QString QuteSpinBox::getStringValue()
{
  return static_cast<QDoubleSpinBox *>(m_widget)->text();
}

void QuteSpinBox::applyInternalProperties()
{
//  qDebug() << "QuteSpinBox::applyInternalProperties()";

  static_cast<QDoubleSpinBox*>(m_widget)->setValue(property("QCS_value").toDouble());
  double resolution = property("QCS_resolution").toDouble();
  int i;
  for (i = 0; i < 8; i++) {//     Check for used decimal places.
    if ((resolution * pow(10, i)) == (int) (resolution * pow(10,i)) )
      break;
  }
  static_cast<QDoubleSpinBox*>(m_widget)->setDecimals(i);
  static_cast<QDoubleSpinBox*>(m_widget)->setSingleStep(resolution);
//  static_cast<QLabel*>(m_widget)->setText(property("QCS_label").toString());
  Qt::Alignment align;
  QString alignText = property("QCS_alignment").toString();
  if (alignText == "left") {
    align = Qt::AlignLeft|Qt::AlignTop;
  }
  else if (alignText == "center") {
    align = Qt::AlignHCenter|Qt::AlignTop;
  }
  else if (alignText == "right") {
    align = Qt::AlignRight|Qt::AlignTop;
  }
  static_cast<QDoubleSpinBox*>(m_widget)->setAlignment(align);
  setTextColor(property("QCS_color").value<QColor>());
  QString borderStyle = (property("QCS_bordermode").toString() == "border" ? "solid": "none");
  m_widget->setStyleSheet("QLabel { font-family:\"" + property("QCS_font").toString()
                          + "\"; font-size: " + QString::number(property("QCS_fontsize").toInt()  + QCS_FONT_OFFSET) + "pt"
                          + (property("QCS_bgcolormode").toBool() ?
                                    QString("; background-color:") + property("QCS_bgcolor").value<QColor>().name() : QString("; "))
                          + "; color:" + property("QCS_color").value<QColor>().name()
                          + "; border-color:" + property("QCS_color").value<QColor>().name()
                          + "; border-radius:" + QString::number(property("QCS_borderradius").toInt()) + "px"
                          + "; border-width: 1px"
                          + "; border-style: " + borderStyle
                          + "; }");
//  qDebug() << "QuteSpinBox::applyInternalProperties() sylesheet" <<  m_widget->styleSheet();
  QuteWidget::applyInternalProperties();
}

void QuteSpinBox::createPropertiesDialog()
{
//   qDebug("QuteSpinBox::createPropertiesDialog()");
  QuteText::createPropertiesDialog();
  dialog->setWindowTitle("SpinBox");
  QLabel *label = new QLabel(dialog);
  label->setText("Resolution");
  layout->addWidget(label, 4, 0, Qt::AlignRight|Qt::AlignVCenter);
  resolutionSpinBox = new QDoubleSpinBox(dialog);
  resolutionSpinBox->setDecimals(6);
  resolutionSpinBox->setValue(property("QCS_resolution").toDouble());
  layout->addWidget(resolutionSpinBox, 4, 1, Qt::AlignLeft|Qt::AlignVCenter);

  fontSize->hide();
  font->hide();
  border->hide();
  bg->hide();
  textColor->hide();
  bgColor->hide();
  text->setText(static_cast<QDoubleSpinBox *>(m_widget)->text());

}

void QuteSpinBox::applyProperties()
{

    switch (alignment->currentIndex()) {
    case 0:
      setProperty("QCS_alignment", "left");
      break;
    case 1:
      setProperty("QCS_alignment", "center");
      break;
    case 2:
      setProperty("QCS_alignment", "right");
      break;
    default:
      setProperty("QCS_alignment", "");
  }
  setProperty("QCS_font", font->currentFont().family());
  setProperty("QCS_fontsize", fontSize->value());
  setProperty("QCS_bgcolor", bgColor->palette().color(QPalette::Window));
  setProperty("QCS_bgcolormode", bg->isChecked());
  setProperty("QCS_color", textColor->palette().color(QPalette::Window));
  setProperty("QCS_bordermode", border->isChecked() ? "border" : "noborder");
  setProperty("QCS_resolution", resolutionSpinBox->value());
  setProperty("QCS_value", text->toPlainText().toDouble());

//  setProperty("QCS_minimum",(double)  -999999999999.0);
//  setProperty("QCS_maximum", (double) 99999999999999.0);
//  setProperty("QCS_randomizable",false);

  QuteWidget::applyProperties();  //Must be last to make sure the widgetsChanged signal is last
}
