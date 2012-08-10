//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2012 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================

#include <dax/cont/UnstructuredGrid.h>

#include <dax/exec/CellHexahedron.h>

#include <dax/cont/internal/TestingGridGenerator.h>
#include <dax/cont/internal/Testing.h>

namespace {

void TestUnstructuredGrid()
{
  const dax::Id DIM = 5;

  typedef dax::cont::UnstructuredGrid<dax::exec::CellHexahedron> GridType;
  dax::cont::internal::TestGrid<GridType> gridGen(DIM);
  GridType grid = gridGen.GetRealGrid();

  std::cout << "Test basic information." << std::endl;
  DAX_TEST_ASSERT(grid.GetNumberOfCells() == (DIM-1)*(DIM-1)*(DIM-1),
                  "Wrong number of cells.");
  DAX_TEST_ASSERT(grid.GetNumberOfPoints() == DIM*DIM*DIM,
                  "Wrong number of points.");

  std::cout << "Test execution structure." << std::endl;
  GridType::TopologyStructConstExecution topology = grid.PrepareForInput();
  DAX_TEST_ASSERT(grid.GetNumberOfCells() == topology.NumberOfCells,
                  "Execution structure has wrong number of cells.");
  DAX_TEST_ASSERT(grid.GetNumberOfPoints() == topology.NumberOfPoints,
                  "Execution structure has wrong number of points.");

  GridType::CellConnectionsType::PortalConstControl connections
      = grid.GetCellConnections().GetPortalConstControl();
  for (dax::Id index = 0;
       index < grid.GetNumberOfCells() * dax::exec::CellHexahedron::NUM_POINTS;
       index++)
    {
    DAX_TEST_ASSERT(connections.Get(index)==topology.CellConnections.Get(index),
                    "Bad connection.");
    }
}

} // anonymous namespace

int UnitTestUnstructuredGrid(int, char *[])
{
  return dax::cont::internal::Testing::Run(TestUnstructuredGrid);
}