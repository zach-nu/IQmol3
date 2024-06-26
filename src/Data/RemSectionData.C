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

#include "RemSectionData.h"
#include <QDebug>


namespace IQmol {
namespace Data {


void RemSection::insert(QString const& key, QString const& value)
{
   m_rem.insert(key, value);
}


QString RemSection::value(QString const& key) const
{
   return m_rem.value(key);
}


bool RemSection::isSet(QString const& key) const
{
   return m_rem.contains(key);
}


QString RemSection::format() const
{
   QString rem("$rem\n");

   QMap<QString, QString>::const_iterator iter;
   for (iter = m_rem.begin(); iter != m_rem.end(); ++iter) {
       rem +=  "  " + iter.key() + " = " + iter.value() + "\n";
   }

   rem += "$end";

   return rem;
}


void RemSection::dump() const
{
   QMap<QString, QString>::const_iterator iter;
   for (iter = m_rem.begin(); iter != m_rem.end(); ++iter) {
       qDebug() << "  " << iter.key() << " = " << iter.value();
   }
}

} } // end namespace IQmol::Data
