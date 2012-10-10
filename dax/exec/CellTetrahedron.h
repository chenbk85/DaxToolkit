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
#ifndef __dax_exec_CellTetrahedron_h
#define __dax_exec_CellTetrahedron_h


#include <dax/Types.h>
#include <dax/exec/internal/TopologyUnstructured.h>

namespace dax { namespace exec {

class CellTetrahedron
{
public:

  /// static variable that returns the number of points per cell
  const static dax::Id NUM_POINTS = 4;
  typedef dax::Tuple<dax::Id,NUM_POINTS> PointConnectionsType;
  const static dax::Id TOPOLOGICAL_DIMENSIONS = 3;

private:
  PointConnectionsType Connections;

public:
  /// Create a cell for the given work.
  template<class ConnectionsPortalT>
  DAX_EXEC_EXPORT CellTetrahedron(
    const dax::exec::internal::TopologyUnstructured<
      CellTetrahedron,ConnectionsPortalT> &topology)
    :Connections()
    { }

  /// Get the number of points in the cell.
  DAX_EXEC_EXPORT dax::Id GetNumberOfPoints() const
  {
    return NUM_POINTS;
  }

  /// Given a vertex index for a point (0 to GetNumberOfPoints() - 1), returns
  /// the index for the point in point space.
  DAX_EXEC_EXPORT dax::Id GetPointIndex(const dax::Id vertexIndex) const
  {
    return this->GetPointIndices()[vertexIndex];
  }

  /// returns the indices for all the points in the cell.
  DAX_EXEC_EXPORT PointConnectionsType GetPointIndices() const
  {
    return this->Connections;
  }

  // method to set this cell from a portal
  template<class ConnectionsPortalT>
  DAX_EXEC_EXPORT void SetPointIndices(
      const dax::exec::internal::TopologyUnstructured<
        CellTetrahedron,ConnectionsPortalT> &topology,
      dax::Id cellIndex)
  {
    dax::Id offset = cellIndex*NUM_POINTS;
    this->Connections[0] = topology.CellConnections.Get(offset + 0);
    this->Connections[1] = topology.CellConnections.Get(offset + 1);
    this->Connections[2] = topology.CellConnections.Get(offset + 2);
    this->Connections[3] = topology.CellConnections.Get(offset + 3);
  }

  //  method to set this cell from a different tuple
  DAX_EXEC_EXPORT void SetPointIndices(
      const PointConnectionsType & cellConnections)
  {
    this->Connections = cellConnections;
  }

};

}}
#endif // __dax_exec_CellTetrahedron_h
