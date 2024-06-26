/*******************************************************************************
         
  Copyright (C) 2011-2015 Andrew Gilbert
      
  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.
         
  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software  
  Foundation, either version 3 of the License, or (at your option) any later  
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.
      
  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.
   
********************************************************************************/

#include "ShaderDialog.h"
#include "ShaderLibrary.h"
#include "Preferences.h"
#include "QMsgBox.h"

#include <QDebug>
#include <QDir>


namespace IQmol {

ShaderDialog::ShaderDialog(ShaderLibrary& library, QWidget* parent) : QDialog(parent),
   m_shaderLibrary(library)
{
   m_dialog.setupUi(this);

  setupShaderTab();

  if (!m_shaderLibrary.filtersAvailable()) {
      m_dialog.shaderFilterTabWidget->removeTab(1);
   }else {
      // setupEffectsTab();
      m_dialog.shaderFilterTabWidget->removeTab(1); // disable for the moment
   }

   setupPovRayTab();
}


void ShaderDialog::setupShaderTab()
{
   // This is determined by s_maxSliders
   m_labels[0]  = m_dialog.label0;
   m_labels[1]  = m_dialog.label1;
   m_labels[2]  = m_dialog.label2;
   m_labels[3]  = m_dialog.label3;
   m_labels[4]  = m_dialog.label4;
   m_labels[5]  = m_dialog.label5;

   // This is determined by s_maxSliders
   m_sliders[0] = m_dialog.slider0;
   m_sliders[1] = m_dialog.slider1;
   m_sliders[2] = m_dialog.slider2;
   m_sliders[3] = m_dialog.slider3;
   m_sliders[4] = m_dialog.slider4;
   m_sliders[5] = m_dialog.slider5;

   // This is determined by s_maxCheckBoxes
   m_checkBoxes[0] = m_dialog.checkBox0;
   m_checkBoxes[1] = m_dialog.checkBox1;
   m_checkBoxes[2] = m_dialog.checkBox2;
   m_checkBoxes[3] = m_dialog.checkBox3;
   m_checkBoxes[4] = m_dialog.checkBox4;
   m_checkBoxes[5] = m_dialog.checkBox5;

   // This is determined by s_maxLightBoxes
   m_lightBoxes[0] = m_dialog.lightBox0;
   m_lightBoxes[1] = m_dialog.lightBox1;
   m_lightBoxes[2] = m_dialog.lightBox2;
   m_lightBoxes[3] = m_dialog.lightBox3;
   m_lightBoxes[4] = m_dialog.lightBox4;
   m_lightBoxes[5] = m_dialog.lightBox5;

   for (int i = 0; i < s_maxSliders; ++i) {
       connect(m_sliders[i], SIGNAL(valueChanged(int)), 
          this, SLOT(installShaderParameters(int)));
   }

   for (int i = 0; i < s_maxCheckBoxes; ++i) {
       connect(m_checkBoxes[i], SIGNAL(stateChanged(int)), 
          this, SLOT(installShaderParameters(int)));
   }

   for (int i = 0; i < s_maxLightBoxes; ++i) {
       connect(m_lightBoxes[i], SIGNAL(stateChanged(int)), 
          this, SLOT(installShaderParameters(int)));
   }

   QStringList shaderNames(m_shaderLibrary.availableShaders());
   m_dialog.shaderCombo->addItems(shaderNames);

   int index(m_dialog.shaderCombo->findText(Preferences::DefaultShader()));
   if (index < 0) index = m_dialog.shaderCombo->findText(ShaderLibrary::NoShader);

   m_dialog.shaderCombo->setCurrentIndex(index);
   on_shaderCombo_currentIndexChanged(index);
}


void ShaderDialog::setupEffectsTab()
{
   connect(m_dialog.antialias, SIGNAL(clicked(bool)), 
      this, SLOT(installFilterParameters(bool)));
   connect(m_dialog.border, SIGNAL(clicked(bool)), 
      this, SLOT(installFilterParameters(bool)));
   connect(m_dialog.ambientOcclusion, SIGNAL(clicked(bool)), 
      this, SLOT(installFilterParameters(bool)));

   connect(m_dialog.aoRadius, SIGNAL(valueChanged(int)), 
      this, SLOT(installFilterParameters(int)));
   connect(m_dialog.aoStrength, SIGNAL(valueChanged(int)), 
      this, SLOT(installFilterParameters(int)));
   connect(m_dialog.aoStrength, SIGNAL(valueChanged(int)), 
      this, SLOT(installFilterParameters(int)));

   copyFilterParametersToDialog(Preferences::DefaultFilterParameters());
}





void ShaderDialog::on_shaderCombo_currentIndexChanged(int)
{
   QString name(m_dialog.shaderCombo->currentText());

   if (!m_shaderLibrary.bindShader(name)) {
      QString msg("Shader not found: ");
      msg += name;
      QMsgBox::warning(this, "IQmol", msg);
      return;
   }

   hideOptionControls();
   copyParametersToDialog(m_shaderLibrary.uniformUserVariableList(name));
   updated();
}


void ShaderDialog::hideOptionControls()
{
   for (int i = 0; i < s_maxSliders; ++i) {
       m_labels[i]->hide();
       m_sliders[i]->hide();
   }
   for (int i = 0; i < s_maxCheckBoxes; ++i) {
       m_checkBoxes[i]->hide();
   }
   for (int i = 0; i < s_maxLightBoxes; ++i) {
       m_lightBoxes[i]->hide();
   }

   m_dialog.lightBox->hide();
   m_dialog.optionBox->hide();
}


void ShaderDialog::copyParametersToDialog(QVariantMap const& map)
{
   QString name;
   double val;
   bool ok;

   hideOptionControls();
   int sliderCount(0);
   int checkBoxCount(0);
   int lightBoxCount(0);

   for (QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
       name = iter.key();

       if (name.startsWith("user_light_")) {
          // lights are either on or off, no dimmers here
          name.replace("user_light_","");
          name.replace("_"," ");
          m_dialog.lightBox->show();
          if (lightBoxCount < s_maxLightBoxes) {
             m_lightBoxes[lightBoxCount]->show();
             m_lightBoxes[lightBoxCount]->setText(name);
             m_lightBoxes[lightBoxCount]->setChecked(iter.value().toBool());
             ++lightBoxCount;
          }

       } else if (name.startsWith("user_")) {
          name.replace("user_","");
          name.replace("_"," ");

          switch (iter.value().type()) {
             case QVariant::Bool:
                if (checkBoxCount < s_maxCheckBoxes) {
                   m_dialog.optionBox->show();
                   m_checkBoxes[checkBoxCount]->show();
                   m_checkBoxes[checkBoxCount]->setText(name);
                   m_checkBoxes[checkBoxCount]->setChecked(iter.value().toBool());
                   ++checkBoxCount;
                }
             break;
             case QVariant::Double:
             case QMetaType::Float:
                val = iter.value().toDouble(&ok);  
                if (ok && val <= 1.0 && sliderCount < s_maxSliders) {  
                   m_labels[sliderCount]->show();
                   m_labels[sliderCount]->setText(name);
                   m_sliders[sliderCount]->show();
                   m_sliders[sliderCount]->setValue(100*val);
                   ++sliderCount;
                }
             break;
             default:
                qDebug() << "Unsupported QVariant type in ShaderDialog" << iter.value().type();
             break;
          }
      }
   }  
}


QVariantMap ShaderDialog::getParametersFromDialog()
{
   QVariantMap map;
   for (int i = 0; i < s_maxSliders; ++i) {
       if (m_sliders[i]->isVisible()) {
          QString name(m_labels[i]->text().replace(" ","_"));
          name.prepend("user_");
          double value(m_sliders[i]->value()/100.0);
          map.insert(name, QVariant(value));
       }
   }

   for (int i = 0; i < s_maxCheckBoxes; ++i) {
       if (m_checkBoxes[i]->isVisible()) {
          QString name(m_checkBoxes[i]->text().replace(" ","_"));
          name.prepend("user_");
          bool value(m_checkBoxes[i]->isChecked());
          map.insert(name, QVariant(value));
       }
   }

   for (int i = 0; i < s_maxLightBoxes; ++i) {
       if (m_lightBoxes[i]->isVisible()) {
          QString name(m_lightBoxes[i]->text().replace(" ","_"));
          name.prepend("user_light_");
          bool value(m_lightBoxes[i]->isChecked());
          map.insert(name, QVariant(value));
       }
   }
 
   return map;
}


void ShaderDialog::installShaderParameters(int)
{
   QString name(m_dialog.shaderCombo->currentText());
   m_shaderLibrary.setUniformVariables(name, getParametersFromDialog());
   updated();
}


void ShaderDialog::on_saveAsDefault_clicked(bool)
{
   qDebug() << "Setting Default Shader" << m_dialog.shaderCombo->currentText();
   qDebug() << getParametersFromDialog();
   qDebug() << "Setting Default Shader";
   qDebug() << getFilterParametersFromDialog();
   qDebug() << "Setting Default PovRay Parameters";
   qDebug() << getPovRayParametersFromDialog();

   Preferences::DefaultShader(m_dialog.shaderCombo->currentText());
   Preferences::DefaultShaderParameters(getParametersFromDialog());
   Preferences::DefaultFilterParameters(getFilterParametersFromDialog());
   Preferences::DefaultPovRayParameters(getPovRayParametersFromDialog());
}


void ShaderDialog::on_ambientOcclusion_clicked(bool tf)
{
   m_dialog.aoOptionsGroupBox->setEnabled(tf);
}


void ShaderDialog::copyFilterParametersToDialog(QVariantMap const& map)
{

   // Don't allow these to be enabled from the Preferences
/*
   bool tf;
   tf = map.contains("Antialias") ? map.value("Antialias").toBool() : false;
   m_dialog.antialias->setChecked(tf);

   tf = map.contains("Border") ? map.value("Border").toBool() : false;
   m_dialog.border->setChecked(tf);

   tf = map.contains("AmbientOcclusion") ? map.value("AmbientOcclusion").toBool() : false;
   m_dialog.ambientOcclusion->setChecked(tf);
   m_dialog.aoOptionsGroupBox->setEnabled(tf);
*/

   double value;
   
   value = map.contains("AORadius") ? map.value("AORadius").toDouble() : 0.0;
   m_dialog.aoRadius->setValue(int(100.0*value));

   value = map.contains("AOStrength") ? map.value("AOStrength").toDouble() : 0.0;
   m_dialog.aoStrength->setValue(int(100.0*value));

   value = map.contains("AOTotal") ? map.value("AOTotal").toDouble() : 0.0;
   m_dialog.aoTotal->setValue(int(100.0*value));
}


QVariantMap ShaderDialog::getFilterParametersFromDialog()
{
   QVariantMap map;
   map.insert("Antialias", QVariant(m_dialog.antialias->isChecked()));
   map.insert("Border", QVariant(m_dialog.border->isChecked()));
   map.insert("AmbientOcclusion", QVariant(m_dialog.ambientOcclusion->isChecked()));
   map.insert("AORadius", QVariant(m_dialog.aoRadius->value()/100.0));
   map.insert("AOStrength", QVariant(m_dialog.aoStrength->value()/100.0));
   map.insert("AOTotal", QVariant(m_dialog.aoTotal->value()/100.0));
   return map;
}


void ShaderDialog::installFilterParameters()
{
   m_shaderLibrary.setFilterVariables(getFilterParametersFromDialog());
   updated();
}


// POV-Ray tab

void ShaderDialog::setupPovRayTab()
{
   QStringList textures;
   textures << "None"
            << "Chrome"
            << "Crumpled"
            << "Rippled"
            << "Rough"
            << "Marble"
            << "BlackMarble";

   m_dialog.atomTexture->clear();
   m_dialog.atomTexture->addItems(textures);
   m_dialog.bondTexture->clear();
   m_dialog.bondTexture->addItems(textures);

   textures << "Skin"
            << "Swirl"
            << "Bubble"
            << "Shattered"
            << "Mesh";

   m_dialog.surfaceTexture->clear();
   m_dialog.surfaceTexture->addItems(textures);

   loadPovRayImages();

   on_atomTexture_currentIndexChanged(0);
   on_surfaceTexture_currentIndexChanged(0);
   connect(m_dialog.generatePovRay, SIGNAL(clicked(bool)), 
          this, SLOT(generatePovRay(bool)));
}


void ShaderDialog::on_background_currentIndexChanged(int index)
{
   QPalette pal;
   QString label(m_dialog.background->currentText());

   if (label.contains("None")) {
      pal.setColor(QPalette::Window, Qt::lightGray);
   }else if (label.contains("Default")) {
      pal.setColor(QPalette::Window, Preferences::BackgroundColor());
   }else if (label.contains("White")) {
      pal.setColor(QPalette::Window, Qt::white);
   }else if (label.contains("Black")) {
      pal.setColor(QPalette::Window, Qt::black);
   }

   m_dialog.atomSampleFrame->setAutoFillBackground(true);
   m_dialog.atomSampleFrame->setPalette(pal);
   m_dialog.surfaceSampleFrame->setAutoFillBackground(true);
   m_dialog.surfaceSampleFrame->setPalette(pal);
}


void ShaderDialog::on_atomTexture_currentIndexChanged(int index)
{
   QString label(m_dialog.atomTexture->currentText());
   label += "-structure";
   QString style("background-image: url(");
   style += m_povRayImages[label] + ");";
   m_dialog.atomSampleFrame->setStyleSheet(style);
   on_background_currentIndexChanged(0);
}


void ShaderDialog::on_surfaceTexture_currentIndexChanged(int index)
{
   QString label(m_dialog.surfaceTexture->currentText());
   label += "-surface";
   QString style("background-image: url(");
   style += m_povRayImages[label] + ");";
   m_dialog.surfaceSampleFrame->setStyleSheet(style);
   on_background_currentIndexChanged(0);
}


void ShaderDialog::loadPovRayImages()
{
   QDir dir(Preferences::ShaderDirectory());

   dir.cd("images");
   QDir::Filters filters(QDir::Files | QDir::Readable);
   QStringList contents(dir.entryList(filters));

   QStringList::iterator iter;
   for (iter = contents.begin(); iter != contents.end(); ++iter) {
       QString fname(*iter);
       QFileInfo info(dir, fname);
       fname = fname.replace(".png","",Qt::CaseInsensitive);
       qDebug() << "setting path " << fname << "->" << info.filePath(); 
       m_povRayImages[fname] = info.filePath();
   }

}
   


void ShaderDialog::generatePovRay(bool)
{
   m_shaderLibrary.setPovRayVariables(getPovRayParametersFromDialog());
   generatePovRay();
}


void ShaderDialog::povRayTextureChanged(int)
{
   // This is where we need to load image files based on the selections
   QString atomTexture(m_dialog.atomTexture->currentText());
   QString bondTexture(m_dialog.bondTexture->currentText());
   QString surfaceTexture(m_dialog.surfaceTexture->currentText());

   qDebug() << "POV-Ray texture change" << atomTexture << " " 
            << bondTexture << " " << surfaceTexture;
}


QVariantMap ShaderDialog::getPovRayParametersFromDialog()
{
   QVariantMap map;

   map.insert("background",     QVariant(m_dialog.background->currentText()));
   map.insert("atomTexture",    QVariant(m_dialog.atomTexture->currentText()));
   map.insert("bondTexture",    QVariant(m_dialog.bondTexture->currentText()));
   map.insert("surfaceTexture", QVariant(m_dialog.surfaceTexture->currentText()));

   map.insert("lightFront",     QVariant(m_dialog.povLightFront->isChecked()));
   map.insert("lightHighlight", QVariant(m_dialog.povLightHighlight->isChecked()));
   map.insert("lightLeft",      QVariant(m_dialog.povLightLeft->isChecked()));
   map.insert("lightLower",     QVariant(m_dialog.povLightLower->isChecked()));

   return map;
}

} // end namespace IQmol
