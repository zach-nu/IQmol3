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

#include "Data/Density.h"
#include "Math/Matrix.h"
#include "Util/QsLog.h"
#include <QDebug>


namespace IQmol {
namespace Data {


Density::Density(SurfaceType const& surfaceType, QList<double> const& elements, 
   QString const& label, bool square) : m_surfaceType(surfaceType), m_label(label)
{
   // This assumes a symmetric density
   unsigned nElements(elements.size());

   if (square) {
      m_nBasis = round(std::sqrt(nElements));
      if (m_nBasis*m_nBasis != nElements) {
         qDebug() << "Invalid number of square density matrix elements";
         return;
      }

      m_elements.resize((m_nBasis*(m_nBasis+1))/2);
      qDebug() << "m_nBasis set to      " << m_nBasis;
      qDebug() << "Resizing elements to " << m_elements.size();

      size_t k(0);
      for (size_t i(0); i < m_nBasis; ++i) {
          m_elements[k] = elements[i*m_nBasis+i];
          ++k;
          for (size_t j(i+1); j < m_nBasis; ++j, ++k) {
              m_elements[k] = elements[i*m_nBasis+j];
          }
      }

   }else {
      m_nBasis = round((std::sqrt(1.0+8.0*nElements) -1.0)/2.0);
      if ((m_nBasis*(m_nBasis+1))/2 != nElements) {
         qDebug() << "Invalid number of density matrix elements";
         return;
      }
   
      m_elements.resize(nElements);
      for (unsigned i = 0; i < nElements; ++i) {
          m_elements[i] = elements[i];
      }
   }
}


Density::Density(SurfaceType const& surfaceType, Matrix const& matrix, 
   QString const& label) : m_surfaceType(surfaceType), m_label(label)
{
   m_nBasis = 0;
   if (matrix.size1() != matrix.size2()) {
      QLOG_ERROR() << "Non-square matrix passed to Density constructor";
   }

   m_nBasis = matrix.size1();
   m_elements.resize(m_nBasis*(m_nBasis+1)/2);

   unsigned k(0);
   for (unsigned i = 0; i < m_nBasis; ++i) {
       for (unsigned j = 0; j <= i; ++j, ++k) {
           m_elements[k] = matrix(i,j);
       }
   }
}


void Density::dump() const
{
   qDebug() << "Density dump:" << m_label;
   qDebug() << m_nBasis << "basis functions and" << m_elements.size() << "elements";
   qDebug() << "Number of basis functions" << m_nBasis;
   m_surfaceType.dump();
}

} } // end namespace IQmol::Data
