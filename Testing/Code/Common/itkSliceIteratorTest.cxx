/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkSliceIteratorTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkImage.h"
#include "itkNeighborhood.h"
#include "itkSliceIterator.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodIterator.h"
#include <iostream>

template< class T, unsigned int N >
void FillRegionSequential(itk::SmartPointer< itk::Image<T, N> > I)
{
  unsigned int iDim, ArrayLength, i;
  itk::Size<N> Index;
  unsigned long int Location[N];
  unsigned int mult;
  T value;
  
  itk::ImageRegionIterator<itk::Image<T, N> > data(I, I->GetRequestedRegion());

  Index = (I->GetRequestedRegion()).GetSize();
  
  for (ArrayLength=1, iDim = 0; iDim<N; ++iDim)
  {
    Location[iDim] =0;
    ArrayLength*=Index[iDim];
  }
  
  for (i=0; i<ArrayLength; ++i, ++data)
  {
    for (iDim=0, mult=1, value=0; iDim<N; ++iDim, mult*=10)
    {
      value += mult *  Location[N-iDim-1];
    }
    data.Set( value );
          
    iDim = N-1;
          bool done=false;
          while(!done)
            {
            ++Location[iDim];
            if(Location[iDim] == Index[(N-1)-iDim])
              { Location[iDim] = 0; }
            else
              { done = true; }
            if(iDim == 0)
              { done = true; }
            else
              { --iDim; }
            }
  }
}

template< class T, unsigned int VDimension >
void PrintRegion(itk::SmartPointer< itk::Image<T, VDimension> > I)
{
  unsigned int iDim;
  long rsz[VDimension];
  long Location[VDimension];
  
  memcpy(rsz, I->GetRequestedRegion().GetSize().m_Size,
         sizeof(unsigned long) * VDimension);
  memset(Location, 0, sizeof(unsigned long) * VDimension); 
  for (iDim = 0; iDim < VDimension; ++iDim)
    {
      std::cout << "iDim = " << iDim << std::endl;
      std::cout << "\tRegionSize = "
           << I->GetRequestedRegion().GetSize().m_Size[iDim]
           << std::endl;
      std::cout << "\tRegionStartIndex = "
           << I->GetRequestedRegion().GetIndex()[iDim] << std::endl;
    }
  
  itk::ImageRegionIterator<itk::Image<T, VDimension> > iter( I, I->GetRequestedRegion());
    
  for (; ! iter.IsAtEnd(); ++iter)
    {
      std::cout << iter.Get() << " ";

      iDim=VDimension-1;
      bool done=false;
      while(!done)
        {
        ++Location[iDim];
        if(Location[iDim]==rsz[(VDimension-1)-iDim])
          {
          std::cout << std::endl;
          Location[iDim]=0;
          }
        else
          { done = true; }
        if(iDim == 0)
          { done = true; }
        else
          { --iDim; }
        }
    }
}

template <class TContainer>
void PrintSlice(TContainer s)
{
  std::cout << "[" ;
  for (s=s.Begin(); s < s.End(); s++)
    {
      std::cout << *s << " ";
    }
  std::cout << "]" << std::endl;
  
}


int itkSliceIteratorTest(int, char**)
{

  try {
    
      itk::ImageRegion<2> reg;
      itk::Size<2> hoodRadius;
      itk::Size<2> imgSize;
      itk::Index<2> zeroIndex;
      zeroIndex[0]=zeroIndex[1]=0;
      imgSize[0]=imgSize[1]=20;
      hoodRadius[0]=hoodRadius[1]=2;
      reg.SetIndex(zeroIndex);
      reg.SetSize(imgSize);
      
      std::slice hslice(10, 5, 1); // slice through the horizontal center
      std::slice vslice(2, 5, 5);  // slice through the vertical center
      itk::Neighborhood<int, 2> temp;
      itk::SliceIterator<int, itk::Neighborhood<int,2> > hnsi(&temp, hslice);
      itk::SliceIterator<int, itk::Neighborhood<int,2> > vnsi(&temp, vslice);
      itk::Neighborhood<int, 2> op;
      op.SetRadius(hoodRadius);
      
      itk::Index<2> idx;
      idx[0]=idx[1]=0;
      
      itk::Image<int, 2>::Pointer ip = itk::Image<int,2>::New();
      ip->SetRequestedRegion(reg);
      ip->SetBufferedRegion(reg);
      ip->SetLargestPossibleRegion(reg);
      ip->Allocate();
      
      FillRegionSequential<int, 2>(ip);
      PrintRegion<int,2>(ip);
      
      itk::NeighborhoodIterator<itk::Image<int,2> >
        it(hoodRadius, ip, reg);
      
      for (it.GoToBegin(); !it.IsAtEnd(); ++it)
        {
          temp = it.GetNeighborhood();
          temp.Print(std::cout);
          PrintSlice(hnsi);
          PrintSlice(vnsi);
        }
  }
  catch (itk::ExceptionObject &err)
    {
      (&err)->Print(std::cerr);
      return 2;
    }
  return EXIT_SUCCESS;
}
