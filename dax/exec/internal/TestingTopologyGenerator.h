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
#ifndef __dax_exec_internal_TestingTopologyGenerator_h
#define __dax_exec_internal_TestingTopologyGenerator_h

#include <dax/Types.h>

#include <dax/exec/Cell.h>

#include <dax/exec/internal/ArrayPortalFromIterators.h>
#include <dax/exec/internal/GridTopologies.h>

#include <dax/internal/Testing.h>

// This use of the STL vector only works in non-CUDA unit tests.
#include <iterator>
#include <vector>

namespace dax {
namespace exec {
namespace internal {

template<class TopologyType>
class TestTopology
{
public:
  typedef typename TopologyType::CellType CellType;

private:
  TopologyType Topology;
  std::vector<dax::Vector3> CoordinatesArray;
  std::vector<dax::Id> ConnectionsArray;

public:
  TestTopology() {
    this->BuildTopology(this->Topology,
                        this->CoordinatesArray,
                        this->ConnectionsArray);
  }

  dax::Id GetNumberOfPoints() const {
    return numberOfPoints(this->Topology);
  }
  dax::Id GetNumberOfCells() const {
    return numberOfCells(this->Topology);
  }

  TopologyType GetTopology() const { return this->Topology; }

  dax::Tuple<dax::Id, CellType::NUM_POINTS>
  GetCellConnections(dax::Id cellId) const
  {
    return TestTopology::GetCellConnectionsImpl(this->Topology, cellId);
  }

  CellType GetCell(dax::Id cellId) const
  {
    return CellType(this->Topology, cellId);
  }

  dax::Vector3 GetPointCoordinates(dax::Id pointIndex) const {
    return this->CoordinatesArray[pointIndex];
  }

  dax::Tuple<dax::Vector3,CellType::NUM_POINTS>
  GetCellVertexCoordinates(dax::Id cellIndex) const {
    dax::Tuple<dax::Id, CellType::NUM_POINTS> cellConnections =
        this->GetCellConnections(cellIndex);
    dax::Tuple<dax::Vector3,CellType::NUM_POINTS> coordinates;
    for (dax::Id index = 0; index < CellType::NUM_POINTS; index++)
      {
      coordinates[index] = this->GetPointCoordinates(cellConnections[index]);
      }
    return coordinates;
  }

  template<class ArrayPortalType>
  dax::Tuple<typename ArrayPortalType::ValueType, CellType::NUM_POINTS>
  GetFieldValues(dax::Id cellId, const ArrayPortalType &portal) const {
    typedef typename ArrayPortalType::ValueType ValueType;
    dax::Tuple<dax::Id, CellType::NUM_POINTS> cellConnections =
        this->GetCellConnections(cellId);
    dax::Tuple<ValueType,CellType::NUM_POINTS> fieldValues;
    for (dax::Id index = 0; index < CellType::NUM_POINTS; index++)
      {
      fieldValues[index] = portal.Get(cellConnections[index]);
      }
    return fieldValues;
  }

  template<class IteratorType>
  dax::Tuple<typename std::iterator_traits<IteratorType>::value_type,
             CellType::NUM_POINTS>
  GetFieldValuesIterator(dax::Id cellId, IteratorType begin) const {
    typedef typename std::iterator_traits<IteratorType>::value_type ValueType;
    dax::Tuple<dax::Id, CellType::NUM_POINTS> cellConnections =
        this->GetCellConnections(cellId);
    dax::Tuple<ValueType,CellType::NUM_POINTS> fieldValues;
    for (dax::Id index = 0; index < CellType::NUM_POINTS; index++)
      {
      fieldValues[index] = *(begin + cellConnections[index]);
      }
    return fieldValues;
  }

private:
  static dax::exec::internal::TopologyUniform GetCoreTopology()
  {
    dax::exec::internal::TopologyUniform topology;
    topology.Origin = dax::make_Vector3(1.0, -0.5, 13.0);
    topology.Spacing = dax::make_Vector3(2.5, 6.25, 1.0);
    topology.Extent.Min = dax::make_Id3(5, -2, -7);
    topology.Extent.Max = dax::make_Id3(20, 4, 10);
    return topology;
  }

  static dax::Tuple<dax::Id,8> GetCellConnectionsImpl(
      const dax::exec::internal::TopologyUniform &topology,
      dax::Id flatCellIndex)
  {
    //this only works for voxel/hexahedron
    const dax::Id3 cellVertexToPointIndex[8] = {
      dax::make_Id3(0, 0, 0),
      dax::make_Id3(1, 0, 0),
      dax::make_Id3(1, 1, 0),
      dax::make_Id3(0, 1, 0),
      dax::make_Id3(0, 0, 1),
      dax::make_Id3(1, 0, 1),
      dax::make_Id3(1, 1, 1),
      dax::make_Id3(0, 1, 1)
    };
    const dax::Extent3 extent = topology.Extent;

    dax::Tuple<dax::Id,8> connections;

    dax::Id3 ijkCell = dax::flatIndexToIndex3Cell(flatCellIndex, extent);
    for(dax::Id relativePointIndex = 0;
        relativePointIndex < 8;
        relativePointIndex++)
      {
      dax::Id3 ijkPoint = ijkCell+cellVertexToPointIndex[relativePointIndex];

      dax::Id pointIndex = index3ToFlatIndex(ijkPoint, extent);
      connections[relativePointIndex] = pointIndex;
      }
    return connections;
  }

  static void BuildTopology(dax::exec::internal::TopologyUniform &topology,
                            std::vector<dax::Vector3> &coords,
                            std::vector<dax::Id> &daxNotUsed(connections))
  {
    topology = TestTopology::GetCoreTopology();

    dax::Id numPoints = dax::exec::internal::numberOfPoints(topology);
    coords.resize(numPoints);
    for (dax::Id index = 0; index < numPoints; index++)
      {
      coords[index] = dax::exec::internal::pointCoordiantes(topology, index);
      }
  }

  template<class PT>
  static dax::Tuple<dax::Id,8> GetCellConnectionsImpl(
      const dax::exec::internal::TopologyUnstructured<
          dax::exec::CellHexahedron,PT> &daxNotUsed(topology),
      dax::Id cellIndex)
  {
    return TestTopology::GetCellConnectionsImpl(TestTopology::GetCoreTopology(),
                                                cellIndex);
  }

  template<class PT>
  static void BuildTopology(dax::exec::internal::TopologyUnstructured
                            <dax::exec::CellHexahedron,PT>
                            &topology,
                            std::vector<dax::Vector3> &coordArray,
                            std::vector<dax::Id> &connectArray)
  {
    // Base this topology on the uniform topology.
    dax::exec::internal::TopologyUniform uniformTopology =
        TestTopology::GetCoreTopology();

    typedef dax::exec::CellHexahedron CellType;

    topology.NumberOfCells =
        dax::exec::internal::numberOfCells(uniformTopology);
    topology.NumberOfPoints =
        dax::exec::internal::numberOfPoints(uniformTopology);

    coordArray.resize(topology.NumberOfPoints);

    // Make point coordiantes.
    for (dax::Id pointIndex = 0;
         pointIndex < topology.NumberOfPoints;
         pointIndex++)
      {
      dax::Vector3 coord =
          dax::exec::internal::pointCoordiantes(uniformTopology, pointIndex);
      // Perterb coordinates so that they are not axis aligned.
      coord[0] += dax::Scalar(0.5)*coord[2];
      coord[1] += coord[2];
      coordArray[pointIndex] = coord;
      }

    // Make connections
    connectArray.reserve(topology.NumberOfCells*CellType::NUM_POINTS);

    for(dax::Id flatCellIndex = 0;
        flatCellIndex < topology.NumberOfCells;
        flatCellIndex++)
      {
      dax::Tuple<dax::Id,CellType::NUM_POINTS> pointConnections =
          TestTopology::GetCellConnectionsImpl(uniformTopology, flatCellIndex);
      for(dax::Id relativePointIndex = 0;
          relativePointIndex < CellType::NUM_POINTS;
          relativePointIndex++)
        {
        connectArray.push_back(pointConnections[relativePointIndex]);
        }
      }
    DAX_TEST_ASSERT(dax::Id(connectArray.size())
                    == topology.NumberOfCells*CellType::NUM_POINTS,
                    "Bad connection array size.");

    topology.CellConnections = PT(connectArray.begin(), connectArray.end());
  }

  template<class PT>
  static dax::Tuple<dax::Id,4> GetCellConnectionsImpl(
      const dax::exec::internal::TopologyUnstructured<
          dax::exec::CellTetrahedron,PT> &,
      dax::Id cellIndex)
  {
    // Tetrahedron meshes are a Freudenthal subdivision of hexahedra.
    dax::Tuple<dax::Id,8> hexConnections =
        TestTopology::GetCellConnectionsImpl(TestTopology::GetCoreTopology(),
                                             cellIndex/6);
    dax::Tuple<dax::Id,4> tetConnections;
    switch (cellIndex%6)
      {
      case 0:
        tetConnections[0] = hexConnections[0];
        tetConnections[1] = hexConnections[1];
        tetConnections[2] = hexConnections[2];
        tetConnections[3] = hexConnections[4];
        break;
      case 1:
        tetConnections[0] = hexConnections[1];
        tetConnections[1] = hexConnections[5];
        tetConnections[2] = hexConnections[2];
        tetConnections[3] = hexConnections[4];
        break;
      case 2:
        tetConnections[0] = hexConnections[5];
        tetConnections[1] = hexConnections[6];
        tetConnections[2] = hexConnections[2];
        tetConnections[3] = hexConnections[4];
        break;
      case 3:
        tetConnections[0] = hexConnections[6];
        tetConnections[1] = hexConnections[7];
        tetConnections[2] = hexConnections[2];
        tetConnections[3] = hexConnections[4];
        break;
      case 4:
        tetConnections[0] = hexConnections[7];
        tetConnections[1] = hexConnections[3];
        tetConnections[2] = hexConnections[2];
        tetConnections[3] = hexConnections[4];
        break;
      case 5:
        tetConnections[0] = hexConnections[3];
        tetConnections[1] = hexConnections[0];
        tetConnections[2] = hexConnections[2];
        tetConnections[3] = hexConnections[4];
        break;
      default:
        DAX_TEST_FAIL("Should not get here.");
      }
    return tetConnections;
  }

  template<class PT>
  static void BuildTopology(dax::exec::internal::TopologyUnstructured
                            <dax::exec::CellTetrahedron,PT>
                            &topology,
                            std::vector<dax::Vector3> &coordArray,
                            std::vector<dax::Id> &connectArray)
  {
    // Base this topology on the hexahedron topology.
    dax::exec::internal::TopologyUnstructured<
        dax::exec::CellHexahedron,PT> hexTopology;
    std::vector<dax::Id> hexConnections;
    BuildTopology(hexTopology, coordArray, hexConnections);

    typedef dax::exec::CellTetrahedron CellType;

    // Our tetrahedron mesh will be a Freudenthal tetrahedronization of a
    // hexahedron mesh.  This results in 6 tetrahedra for each hexahedron.
    // It is not minimal, but good enough for tests.
    topology.NumberOfCells = hexTopology.NumberOfCells * 6;
    topology.NumberOfPoints = hexTopology.NumberOfPoints;

    // Make connections.
    connectArray.reserve(topology.NumberOfCells*CellType::NUM_POINTS);

    for (dax::Id cellIndex = 0; cellIndex < topology.NumberOfCells; cellIndex++)
      {
      dax::Tuple<dax::Id,CellType::NUM_POINTS> pointConnections
          = TestTopology::GetCellConnectionsImpl(topology, cellIndex);

      connectArray.push_back(pointConnections[0]);
      connectArray.push_back(pointConnections[1]);
      connectArray.push_back(pointConnections[2]);
      connectArray.push_back(pointConnections[3]);
      }
    DAX_TEST_ASSERT(dax::Id(connectArray.size())
                    == topology.NumberOfCells*CellType::NUM_POINTS,
                    "Bad connection array size.");

    topology.CellConnections = PT(connectArray.begin(), connectArray.end());
  }

  template<class PT>
  static dax::Tuple<dax::Id,6> GetCellConnectionsImpl(
      const dax::exec::internal::TopologyUnstructured<
          dax::exec::CellWedge,PT> &,
      dax::Id cellIndex)
  {
    // Wedge meshes are based on hex meshes.  They have 2x the cells.
    // See the comment in BuildTopology.
    dax::Tuple<dax::Id,8> hexConnections =
        TestTopology::GetCellConnectionsImpl(TestTopology::GetCoreTopology(),
                                             cellIndex/2);
    dax::Tuple<dax::Id,6> wedgeConnections;
    if (cellIndex%2 == 0)
      {
      wedgeConnections[0] = hexConnections[0];
      wedgeConnections[1] = hexConnections[3];
      wedgeConnections[2] = hexConnections[2];
      wedgeConnections[3] = hexConnections[4];
      wedgeConnections[4] = hexConnections[7];
      wedgeConnections[5] = hexConnections[6];
      }
    else // cellIndex%2 == 1
      {
      wedgeConnections[0] = hexConnections[0];
      wedgeConnections[1] = hexConnections[2];
      wedgeConnections[2] = hexConnections[1];
      wedgeConnections[3] = hexConnections[4];
      wedgeConnections[4] = hexConnections[6];
      wedgeConnections[5] = hexConnections[5];
      }
    return wedgeConnections;
  }

  template<class PT>
  static void BuildTopology(dax::exec::internal::TopologyUnstructured
                            <dax::exec::CellWedge, PT> &topology,
                            std::vector<dax::Vector3> &coordArray,
                            std::vector<dax::Id> &connectArray)
  {
    // Base this topology on the hexahedron topology.
    dax::exec::internal::TopologyUnstructured<
        dax::exec::CellHexahedron,PT> hexTopology;
    std::vector<dax::Id> hexConnections;
    BuildTopology(hexTopology, coordArray, hexConnections);

    typedef dax::exec::CellWedge CellType;

    // Our wedge mesh will have two wedges for each hexahedron. The wedges are
    // formed by cutting the hexahedron along the x/y diagonal.
    topology.NumberOfCells = hexTopology.NumberOfCells * 2;
    topology.NumberOfPoints = hexTopology.NumberOfPoints;

    // Make connections.
    connectArray.reserve(topology.NumberOfCells*CellType::NUM_POINTS);

    for (dax::Id cellIndex = 0; cellIndex < topology.NumberOfCells; cellIndex++)
      {
      dax::Tuple<dax::Id,CellType::NUM_POINTS> pointConnections
          = TestTopology::GetCellConnectionsImpl(topology, cellIndex);

      connectArray.push_back(pointConnections[0]);
      connectArray.push_back(pointConnections[1]);
      connectArray.push_back(pointConnections[2]);
      connectArray.push_back(pointConnections[3]);
      connectArray.push_back(pointConnections[4]);
      connectArray.push_back(pointConnections[5]);
      }
    DAX_TEST_ASSERT(dax::Id(connectArray.size())
                    == topology.NumberOfCells*CellType::NUM_POINTS,
                    "Bad connection array size.");

    topology.CellConnections = PT(connectArray.begin(), connectArray.end());
  }

  template<class PT>
  static dax::Tuple<dax::Id,3> GetCellConnectionsImpl(
      const dax::exec::internal::TopologyUnstructured<
          dax::exec::CellTriangle,PT> &daxNotUsed(topology),
      dax::Id cellIndex)
  {
    // Triangle meshes are based on hex meshes.  They have 2x the cells.
    // See the comment in BuildTopology.
    dax::Tuple<dax::Id,8> hexConnections =
        TestTopology::GetCellConnectionsImpl(TestTopology::GetCoreTopology(),
                                             cellIndex/2);
    dax::Tuple<dax::Id,3> triConnections;
    if (cellIndex%2 == 0)
      {
      triConnections[0] = hexConnections[0];
      triConnections[1] = hexConnections[5];
      triConnections[2] = hexConnections[7];
      }
    else // cellIndex%2 == 1
      {
      triConnections[0] = hexConnections[6];
      triConnections[1] = hexConnections[3];
      triConnections[2] = hexConnections[1];
      }
    return triConnections;
  }

  template<class PT>
  static void BuildTopology(dax::exec::internal::TopologyUnstructured
                            <dax::exec::CellTriangle, PT> &topology,
                            std::vector<dax::Vector3> &coordArray,
                            std::vector<dax::Id> &connectArray)
  {
    // Base this topology on the hexahedron topology.
    dax::exec::internal::TopologyUnstructured<
        dax::exec::CellHexahedron,PT> hexTopology;
    std::vector<dax::Id> hexConnections;
    BuildTopology(hexTopology, coordArray, hexConnections);

    typedef dax::exec::CellTriangle CellType;

    // Our triangle mesh will have two triangles for each hexahedron. The edges
    // of the triangles come from diagonals of the faces. The two triangles do
    // not touch either and (as follows by pigeonhole principle) all faces have
    // exactly one diagonal that contributes to one triangle. The diagonals are
    // consistent between neighboring hexahedrons so that triangles from
    // different hexahedrons have conformal connections that form planar
    // meshes.
    topology.NumberOfCells = hexTopology.NumberOfCells * 2;
    topology.NumberOfPoints = hexTopology.NumberOfPoints;

    // Make connections.
    connectArray.reserve(topology.NumberOfCells*CellType::NUM_POINTS);

    for (dax::Id cellIndex = 0; cellIndex < topology.NumberOfCells; cellIndex++)
      {
      dax::Tuple<dax::Id,CellType::NUM_POINTS> pointConnections
          = TestTopology::GetCellConnectionsImpl(topology, cellIndex);

      connectArray.push_back(pointConnections[0]);
      connectArray.push_back(pointConnections[1]);
      connectArray.push_back(pointConnections[2]);
      }
    DAX_TEST_ASSERT(dax::Id(connectArray.size())
                    == topology.NumberOfCells*CellType::NUM_POINTS,
                    "Bad connection array size.");

    topology.CellConnections = PT(connectArray.begin(), connectArray.end());
  }

  template<class PT>
  static dax::Tuple<dax::Id,4> GetCellConnectionsImpl(
      const dax::exec::internal::TopologyUnstructured<
          dax::exec::CellQuadrilateral,PT> &daxNotUsed(topology),
      dax::Id cellIndex)
  {
    // Quadrilateral meshes are based on hex meshes.
    dax::Tuple<dax::Id,8> hexConnections =
        TestTopology::GetCellConnectionsImpl(TestTopology::GetCoreTopology(),
                                             cellIndex);
    dax::Tuple<dax::Id,4> quadConnections;
    quadConnections[0] = hexConnections[0];
    quadConnections[1] = hexConnections[2];
    quadConnections[2] = hexConnections[6];
    quadConnections[3] = hexConnections[4];

    return quadConnections;
  }

  template<class PT>
  static void BuildTopology(dax::exec::internal::TopologyUnstructured
                            <dax::exec::CellQuadrilateral,PT> &topology,
                            std::vector<dax::Vector3> &coordArray,
                            std::vector<dax::Id> &connectArray)
  {
    // Base this topology on the hexahedron topology.
    dax::exec::internal::TopologyUnstructured<
        dax::exec::CellHexahedron,PT> hexTopology;
    std::vector<dax::Id> hexConnections;
    BuildTopology(hexTopology, coordArray, hexConnections);

    typedef dax::exec::CellQuadrilateral CellType;

    // Our mesh will have one quadrilateral for each hexahedron that cuts the
    // hexahedron diagonally in the x-y direction.

    topology.NumberOfCells = hexTopology.NumberOfCells;
    topology.NumberOfPoints = hexTopology.NumberOfPoints;

    // Make connections.
    connectArray.reserve(topology.NumberOfCells*CellType::NUM_POINTS);

    for (dax::Id cellIndex = 0; cellIndex < topology.NumberOfCells; cellIndex++)
      {
      dax::Tuple<dax::Id,CellType::NUM_POINTS> pointConnections
          = TestTopology::GetCellConnectionsImpl(topology, cellIndex);

      connectArray.push_back(pointConnections[0]);
      connectArray.push_back(pointConnections[1]);
      connectArray.push_back(pointConnections[2]);
      connectArray.push_back(pointConnections[3]);
      }
    DAX_TEST_ASSERT(dax::Id(connectArray.size())
                    == topology.NumberOfCells*CellType::NUM_POINTS,
                    "Bad connection array size.");

    topology.CellConnections = PT(connectArray.begin(), connectArray.end());
  }

  template<class PT>
  static dax::Tuple<dax::Id,2> GetCellConnectionsImpl(
      const dax::exec::internal::TopologyUnstructured<
          dax::exec::CellLine,PT> &daxNotUsed(topology),
      dax::Id cellIndex)
  {
    // Line meshes are based on hex meshes.  They have 7x the cells.
    // See the comment in BuildTopology.
    dax::Tuple<dax::Id,8> hexConnections =
        TestTopology::GetCellConnectionsImpl(TestTopology::GetCoreTopology(),
                                             cellIndex/7);
    dax::Tuple<dax::Id,2> lineConnections;
    switch (cellIndex%7)
      {
      case 0:
        lineConnections[0] = hexConnections[0];
        lineConnections[1] = hexConnections[1];
        break;
      case 1:
        lineConnections[0] = hexConnections[0];
        lineConnections[1] = hexConnections[3];
        break;
      case 2:
        lineConnections[0] = hexConnections[0];
        lineConnections[1] = hexConnections[4];
        break;
      case 3:
        lineConnections[0] = hexConnections[0];
        lineConnections[1] = hexConnections[6];
        break;
      case 4:
        lineConnections[0] = hexConnections[1];
        lineConnections[1] = hexConnections[7];
        break;
      case 5:
        lineConnections[0] = hexConnections[2];
        lineConnections[1] = hexConnections[4];
        break;
      case 6:
        lineConnections[0] = hexConnections[3];
        lineConnections[1] = hexConnections[5];
        break;
      default:
        DAX_TEST_FAIL("Should not get here.");
      }
    return lineConnections;
  }

  template<class PT>
  static void BuildTopology(dax::exec::internal::TopologyUnstructured
                            <dax::exec::CellLine,PT> &topology,
                            std::vector<dax::Vector3> &coordArray,
                            std::vector<dax::Id> &connectArray)
  {
    // Base this topology on the hexahedron topology.
    dax::exec::internal::TopologyUnstructured<
        dax::exec::CellHexahedron,PT> hexTopology;
    std::vector<dax::Id> hexConnections;
    BuildTopology(hexTopology, coordArray, hexConnections);

    typedef dax::exec::CellLine CellType;

    // Our line mesh will have 7 lines for each hexahedron.  One line for
    // each of the 3 edges connected to vertex 0 and one line for each
    // of the 4 major diagonals.
    topology.NumberOfCells = hexTopology.NumberOfCells * 7;
    topology.NumberOfPoints = hexTopology.NumberOfPoints;

    // Make connections.
    connectArray.reserve(topology.NumberOfCells*CellType::NUM_POINTS);

    for (dax::Id cellIndex = 0; cellIndex < topology.NumberOfCells; cellIndex++)
      {
      dax::Tuple<dax::Id,CellType::NUM_POINTS> pointConnections
          = TestTopology::GetCellConnectionsImpl(topology, cellIndex);

      connectArray.push_back(pointConnections[0]);
      connectArray.push_back(pointConnections[1]);
      }
    DAX_TEST_ASSERT(dax::Id(connectArray.size())
                    == topology.NumberOfCells*CellType::NUM_POINTS,
                    "Bad connection array size.");

    topology.CellConnections = PT(connectArray.begin(), connectArray.end());
  }

  template<class PT>
  static dax::Tuple<dax::Id,1> GetCellConnectionsImpl(
      const dax::exec::internal::TopologyUnstructured<
          dax::exec::CellVertex,PT> &daxNotUsed(topology),
      dax::Id cellIndex)
  {
    // Vertex meshes are based on hex meshes.  Each cell is just the first
    // vertex of the hex cell.
    dax::Tuple<dax::Id,8> hexConnections =
        TestTopology::GetCellConnectionsImpl(TestTopology::GetCoreTopology(),
                                             cellIndex/7);
    dax::Tuple<dax::Id,1> vertexConnections;
    vertexConnections[0] = hexConnections[0];
    return vertexConnections;
  }

  template<class PT>
  static void BuildTopology(dax::exec::internal::TopologyUnstructured
                            <dax::exec::CellVertex,PT> &topology,
                            std::vector<dax::Vector3> &coordArray,
                            std::vector<dax::Id> &connectArray)
  {
    // Base this topology on the hexahedron topology.
    dax::exec::internal::TopologyUnstructured<
        dax::exec::CellHexahedron,PT> hexTopology;
    std::vector<dax::Id> hexConnections;
    BuildTopology(hexTopology, coordArray, hexConnections);

    typedef dax::exec::CellVertex CellType;

    // Our line mesh will have 7 lines for each hexahedron.  One line for
    // each of the 3 edges connected to vertex 0 and one line for each
    // of the 4 major diagonals.
    topology.NumberOfCells = hexTopology.NumberOfCells;
    topology.NumberOfPoints = hexTopology.NumberOfPoints;

    // Make connections.
    connectArray.reserve(topology.NumberOfCells*CellType::NUM_POINTS);

    for (dax::Id cellIndex = 0; cellIndex < topology.NumberOfCells; cellIndex++)
      {
      dax::Tuple<dax::Id,CellType::NUM_POINTS> pointConnections
          = TestTopology::GetCellConnectionsImpl(topology, cellIndex);

      connectArray.push_back(pointConnections[0]);
      }
    DAX_TEST_ASSERT(dax::Id(connectArray.size())
                    == topology.NumberOfCells*CellType::NUM_POINTS,
                    "Bad connection array size.");

    topology.CellConnections = PT(connectArray.begin(), connectArray.end());
  }
};

template<class FunctionType>
void TryAllTopologyTypes(FunctionType function)
{
  typedef dax::exec::internal::ArrayPortalFromIterators<
      std::vector<dax::Id>::iterator> ConnectionPortal;

  std::cout << "--- dax::exec::CellVertex" << std::endl;
  TestTopology<dax::exec::internal::TopologyUnstructured
      <dax::exec::CellVertex,ConnectionPortal> > vertexTopology;
  function(vertexTopology);

  std::cout << "--- dax::exec::CellLine" << std::endl;
  TestTopology<dax::exec::internal::TopologyUnstructured
      <dax::exec::CellLine,ConnectionPortal> > lineTopology;
  function(lineTopology);

  std::cout << "--- dax::exec::CellTriangle" << std::endl;
  TestTopology<dax::exec::internal::TopologyUnstructured
      <dax::exec::CellTriangle,ConnectionPortal> > triangleTopology;
  function(triangleTopology);

  std::cout << "--- dax::exec::CellQuadrilateral" << std::endl;
  TestTopology<dax::exec::internal::TopologyUnstructured
      <dax::exec::CellQuadrilateral,ConnectionPortal> >
      quadrilateralTopology;
  function(quadrilateralTopology);

  std::cout << "--- dax::exec::CellVoxel" << std::endl;
  TestTopology<dax::exec::internal::TopologyUniform> voxelTopology;
  function(voxelTopology);

  std::cout << "--- dax::exec::CellHexahedron" << std::endl;
  TestTopology<dax::exec::internal::TopologyUnstructured
      <dax::exec::CellHexahedron,ConnectionPortal> > hexahedronTopology;
  function(hexahedronTopology);

  std::cout << "--- dax::exec::CellTetrahedron" << std::endl;
  TestTopology<dax::exec::internal::TopologyUnstructured
      <dax::exec::CellTetrahedron,ConnectionPortal> > tetrahedronTopology;
  function(tetrahedronTopology);

  std::cout << "--- dax::exec::CellWedge" << std::endl;
  TestTopology<dax::exec::internal::TopologyUnstructured
      <dax::exec::CellWedge,ConnectionPortal> > wedgeTopology;
  function(wedgeTopology);
}

}
}
} // namespace dax::exec::internal

#endif //__dax_exec_internal_TestingTopologyGenerator_h