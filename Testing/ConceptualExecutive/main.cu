/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

===========================================================================*/
#include <stdio.h>
#include <iostream>

#include "Filter.h"
#include "Worklets.h"
#include "StructuredGrid.h"

void createGrid(StructuredGrid *&grid)
{
  dax::Vector3 origin = dax::make_Vector3(0,0,0);
  dax::Vector3 spacing = dax::make_Vector3(.2,.2,.2);
  dax::Extent3 extents = dax::Extent3( dax::make_Id3(0,0,0), dax::make_Id3(10,10,10));
  grid = new StructuredGrid(origin,spacing,extents);
}

dax::ScalarArray TestHostPipeline()
{
  std::cout << "TestElevation Host" << std::endl;
  StructuredGrid* grid;
  createGrid(grid);

  //point coords are not real, and we don't
  //have a clean interface for this yet
  dax::ScalarArray result;
  dax::Vector3Array pointCoords;
  dax::ConvertCoordinatesToArray(grid->points(),pointCoords);

  executeCellPipeline(pointCoords,result);

  return result;
}

dax::ScalarArray TestDevicePipeline()
{
  std::cout << "TestElevation Device" << std::endl;
  StructuredGrid* grid;
  createGrid(grid);

  dax::ScalarArray result;
  dax::Vector3Array pointCoords;
  dax::ConvertCoordinatesToArray(grid->points(),pointCoords);

  dax::DeviceVector3Array devicePCorrds(pointCoords);
  dax::DeviceScalarArray devicePResult(result);

  executeCellPipeline(devicePCorrds,devicePResult);

  dax::toHost(devicePResult,result);
  return result;
}

bool validPipeline(dax::ScalarArray sa1, dax::ScalarArray sa2)
{
  std::size_t size1 = sa1.size();
  std::size_t size2 = sa2.size();
  assert(size1==size2);
  return true;
}


void buildExamplePipeline()
{

  std::cout << "Build Example Pipeline" << std::endl;
  StructuredGrid* grid;
  createGrid(grid);

  //virtual point coordinates suck shit
  dax::Vector3Array pointCoords;
  dax::ConvertCoordinatesToArray(grid->points(),pointCoords);


  Filter<worklets::Elevation> elev(&pointCoords);
  Filter<worklets::Sine> sin(elev);
  Filter<worklets::Square> sq(sin);
  Filter<worklets::Cosine> cos(sq);

  cos.run();

  Filter<worklets::Square> sqa2(sq);

  sqa2.run();


}

int main()
{
  assert(validPipeline(TestHostPipeline(),TestDevicePipeline())==true);
  buildExamplePipeline();

  return 0;
}
