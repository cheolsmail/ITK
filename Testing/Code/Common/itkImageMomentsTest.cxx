/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkImageMomentsTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/

#include "itkAffineTransform.h"
#include "itkImage.h"
#include "itkImageMoments.h"
#include "itkIndex.h"
#include "itkSize.h"


int 
main(
    int argc,
    char *argv[])
{
    /* Define acceptable (absolute) error in computed results.
       All the calculations are done in double and are well-conditioned,
       so we should be able to get within a few epsilon of the right
       values.  So choose maxerr to be 10*epsilon for IEEE 754 double. */
    double maxerr = 1.0e-15;

    /* Define the image size and physical coordinates */
    itk::Size<3> size = {{20, 40, 80}};
    double origin [3] = { 0.5,   0.5,   0.5};
    double spacing[3] = { 0.1,   0.05 , 0.025};

    /* Define positions of the test masses in index coordinates */
    unsigned short mass = 1;           // Test mass
    unsigned long point[8][3] = {
	{ 10+8, 20+12, 40+0},
	{ 10-8, 20-12, 40-0},
	{ 10+3, 20-8,  40+0},
	{ 10-3, 20+8,  40-0},
	{ 10+0, 20+0,  40+10},
	{ 10-0, 20-0,  40-10},
    };

    /* Define the expected (true) results for comparison */
    double ttm = 6.0;                      // Total mass
    double cgd[3] = {1.5,   1.5, 1.5 };    // Center of gravity
    double pmd[3] = {0.125, 0.5, 2.0 };    // Principal moments
    double pad[3][3] = {                   // Principal axes
	{ 0.0,  0.0, 1.0},
	{ 0.6, -0.8, 0.0},
	{ 0.8,  0.6, 0.0},
    };
    vnl_vector_fixed<double,3> tcg;
    tcg.set(cgd);
    vnl_vector_fixed<double,3> tpm;
    tpm.set(pmd);
    vnl_matrix_fixed<double,3,3> tpa;
    tpa.set((double *)pad);

    /* Allocate a simple test image */
    itk::Image<unsigned short, 3>::Pointer
      image = itk::Image<unsigned short,3>::New();
    itk::Image<unsigned short, 3>::RegionType region;
    region.SetSize(size);
    image->SetLargestPossibleRegion(region);
    image->SetBufferedRegion(region);

    /* Set origin and spacing of physical coordinates */
    image->SetOrigin(origin);
    image->SetSpacing(spacing);
    image->Allocate();
    
    /* Set a few mass points within the image */
    /* FIXME: The method used here to set the points is klutzy,
       but appears to be the only method currently supported. */
    itk::Index<3> index;          /* Index over pixels */
    for ( int i = 0; i < 6; i++) {
	index.SetIndex(point[i]);
	image->SetPixel(index, mass);
    }

    /* Compute the moments */
    itk::ImageMoments<unsigned short, 3>
	moments(image);
    double ctm = moments.GetTotalMass();
    vnl_vector_fixed<double,3>
        ccg = moments.GetCenterOfGravity();
    vnl_vector_fixed<double,3>
        cpm = moments.GetPrincipalMoments();
    vnl_matrix_fixed<double,3,3>
        cpa = moments.GetPrincipalAxes();

    /* Report the various non-central moments */
    // FIXME:  Indentation is not handled correctly in matrix output
    std::cout << "\nTotal mass = " << ctm << "\n";
    std::cout << "True total mass = " << ttm << "\n";
    std::cout << "\nFirst moments about index origin =\n";
    std::cout << "   " << moments.GetFirstMoments() << "\n";
    std::cout << "\nSecond moments about index origin =\n";
    std::cout << "   " << moments.GetSecondMoments() << "\n";

    /* Report the center of gravity and central moments */
    std::cout << "\nCenter of gravity =\n";
    std::cout << "   " << ccg << "\n";
    std::cout << "True center of gravity =\n";
    std::cout << "   " << tcg << "\n";
    std::cout << "\nSecond central moments =\n";
    std::cout << "   " << moments.GetCentralMoments() << "\n";

    /* Report principal moments and axes */
    std::cout << "\nPrincipal moments = \n";
    std::cout << "   " << cpm << "\n";
    std::cout << "True principal moments = \n";
    std::cout << "   " << tpm << "\n";
    std::cout << "\nPrincipal axes = \n";
    std::cout << "   " << cpa << "\n";
    std::cout << "True principal axes = \n";
    std::cout << "   " << tpa << "\n";

    /* Compute transforms between principal and physical axes */
    /* FIXME: Automatically check correctness of these results? */
    itk::ImageMoments<unsigned short, 3>::AffineTransformType
        pa2p = moments.GetPrincipalAxesToPhysicalAxesTransform();
    std::cout << "\nPrincipal axes to physical axes transform:\n";
    std::cout << pa2p << std::endl;
    itk::ImageMoments<unsigned short, 3>::AffineTransformType
        p2pa = moments.GetPhysicalAxesToPrincipalAxesTransform();
    std::cout << "\nPhysical axes to principal axes transform:\n";
    std::cout << p2pa << std::endl;

    /* Do some error checking on the transforms */
    double dist = pa2p.Metric(pa2p);
    std::cout << "Distance from self to self = " << dist << std::endl;
    itk::ImageMoments<unsigned short, 3>::AffineTransformType
        p2pa2p;
    p2pa2p.Compose(p2pa);
    p2pa2p.Compose(pa2p);
    double trerr = p2pa2p.Metric();
    std::cout << "Distance from composition to identity = ";
    std::cout << trerr << std::endl;

    /* Compute and report max abs error in computed */
    double tmerr = abs(ttm - ctm);  // Error in total mass
    double cgerr = 0.0;             // Error in center of gravity
    double pmerr = 0.0;             // Error in moments
    double paerr = 0.0;             // Error in axes
    for ( int i = 0; i < 3; ++i) {
        if ( abs(ccg[i] - tcg[i]) > cgerr )
            cgerr = abs(ccg[i] - tcg[i]);
        if ( abs(cpm[i] - tpm[i]) > pmerr )
            pmerr = abs(cpm[i] - tpm[i]);
        for (int j = 0; j < 3; ++j) {
            if (abs(cpa[i][j] - tpa[i][j]) > paerr)
                paerr = abs(cpa[i][j] - tpa[i][j]);
        }
    }
    std::cout << "\nErrors found in:\n";
    std::cout << "   Total mass        = " << tmerr << std::endl;
    std::cout << "   Center of gravity = " << cgerr << std::endl;
    std::cout << "   Principal moments = " << pmerr << std::endl;
    std::cout << "   Principal axes    = " << paerr << std::endl;
    std::cout << "   Transformations   = " << trerr << std::endl;

    /* Return error if differences are too large */
    int stat = 
        tmerr > maxerr || cgerr > maxerr ||
        pmerr > maxerr || paerr > maxerr ||
        trerr > maxerr;
    if (stat)
        std::cout << "\n*** Errors are larger than defined maximum value.\n";
    else
        std::cout << "\n*** Errors are acceptable.\n";
    return stat;
}
