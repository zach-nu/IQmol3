/*******************************************************************************
       
  Copyright (C) 2022 Andrew Gilbert
           
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

#include "PovRayParser.h"
#include "TextStream.h"

#include "Data/PovRay.h"

#include <QRegularExpression>

namespace IQmol {
namespace Parser {

bool PovRay::parse(TextStream& textStream)
{
   Data::PovRay* povray(new Data::PovRay);

   while (!textStream.atEnd()) {
      readNextDeclaration(textStream, povray);
   }

   m_dataBank.append(povray);
   return m_errors.isEmpty();
}


void PovRay::readNextDeclaration(TextStream& textStream, Data::PovRay* povray)
{
   // looks for '#declare TextureName = '
   const QRegularExpression declare("^#declare\\s+(\\S+)\\s?="); 

   QString line(textStream.nextLine());
   if (!line.contains(declare)) line = textStream.seek(declare);

   if (!textStream.atEnd()) {
      QRegularExpressionMatch match(declare.match(line));
      QStringList matches(match.capturedTexts());
 
      if (matches.size() > 1) {
         QString name(matches.at(1)); // 0 is the entire string, maybe
         // remove the initial IQmol_Texture_
         name.remove(0,14);
         name.replace("_", " ");
         line = textStream.nextBlock();
         povray->addTexture(name,line);
      }
   }
}

} } // end namespace IQmol::Parser
