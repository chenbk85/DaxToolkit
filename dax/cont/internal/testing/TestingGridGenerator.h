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

#ifndef __dax_cont_internal_TestingGridGenerator_h
#define __dax_cont_internal_TestingGridGenerator_h

#include <dax/CellTag.h>
#include <dax/CellTraits.h>
#include <dax/Types.h>

#include <dax/cont/ArrayContainerControlBasic.h>
#include <dax/cont/ArrayHandle.h>
#include <dax/cont/DeviceAdapter.h>
#include <dax/cont/UniformGrid.h>
#include <dax/cont/UnstructuredGrid.h>

#include <dax/cont/internal/testing/Testing.h>

#include <vector>

#include <iostream>

namespace dax {
namespace cont {
namespace internal {

template<
    class GridType,
    class ArrayContainerControlTag = DAX_DEFAULT_ARRAY_CONTAINER_CONTROL_TAG,
    class DeviceAdapterTag = DAX_DEFAULT_DEVICE_ADAPTER_TAG>
struct TestGrid
{
private:
  //custom storage container that changes the size based on the grid type
  //so that we don't have to store extra information for uniform grid
  const dax::Id Size;
  GridType Grid;
  template<typename GT> struct GridStorage {};
  template<typename U, class UCCT, class DAT>
  struct GridStorage<dax::cont::UnstructuredGrid<U,UCCT,UCCT,DAT> >
    {
    std::vector<dax::Id> topology;
    std::vector<dax::Vector3> points;
    };
  GridStorage<GridType> Info;

  typedef typename GridType::TopologyStructConstExecution TopoType;

public:
  typedef typename GridType::CellTag CellTag;
  TestGrid(const dax::Id& size)
    :Size(size),
     Grid(),
     Info()
    {
    this->BuildGrid(this->Grid);
    }

  /// Enable pointer-like dereference syntax. Returns a pointer to the
  /// contained object.
  ///
  const GridType* operator->() const
    {
    return &this->Grid;
    }

  /// Get a raw pointer to the contained object.
  ///
  const GridType& GetRealGrid() const
    {
    return this->Grid;
    }

  /// This convienience function allows you to generate the point coordinate
  /// field (since there is no consistent way to get it from the grid itself.
  ///
  dax::Vector3 GetPointCoordinates(dax::Id index)
  {
    return this->Grid.ComputePointCoordinates(index);
  }

private:
  TopoType GetTopology() const
  {
    return this->Grid.PrepareForInput();
  }

public:
  //get the cell connections (aka topology) at a given cell id
  dax::exec::CellVertices<CellTag> GetCellConnections(dax::Id cellId) const
  {
    return this->GetTopology().GetCellConnections(cellId);
  }

  /// This convienience function allows you to generate the Cell
  /// point coordinates for any given data set
  dax::exec::CellField<dax::Vector3,CellTag>
  GetCellVertexCoordinates(dax::Id cellIndex) const
  {
    typedef typename GridType::PointCoordinatesType CoordType;

    //get the point ids for this cell
    dax::exec::CellVertices<CellTag> cellConnections =
        this->GetCellConnections(cellIndex);

    //get all the points for data set
    CoordType allCoords = this->Grid.GetPointCoordinates();

    dax::exec::CellField<dax::Vector3,CellTag> coordinates;
    for (dax::Id index = 0; index < coordinates.NUM_VERTICES; index++)
      {
      coordinates[index] = allCoords.GetPortalConstControl().Get(cellConnections[index]);
      }
    return coordinates;
  }

  ~TestGrid()
  {
    std::cout << "Test grid destroyed.  "
              << "Any use of the grid after this point is an error."
              << std::endl;
  }

private:

  // .......................................................... MakeArrayHandle
  template<typename T>
  dax::cont::ArrayHandle<T, ArrayContainerControlTag, DeviceAdapterTag>
  MakeArrayHandle(const std::vector<T> &array)
  {
    return dax::cont::make_ArrayHandle(array,
                                       ArrayContainerControlTag(),
                                       DeviceAdapterTag());
  }

  // ........................................................... MakeInfoPoints
  void MakeInfoPoints(dax::cont::UniformGrid<DeviceAdapterTag> &uniform)
    {
      //copy the point info over to the unstructured grid
      this->Info.points.clear();
      for(dax::Id i=0; i <uniform.GetNumberOfPoints(); ++i)
        {
        this->Info.points.push_back(uniform.ComputePointCoordinates(i));
        }
    }

  // ......................................................... MakeInfoTopology
  void MakeInfoTopology(dax::cont::UniformGrid<DeviceAdapterTag> &uniform,
                        const dax::Id vertexIdList[],
                        dax::Id numPointsPerCell,
                        dax::Id totalCells)
    {
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
      this->Info.topology.clear();
      const dax::Extent3 extents = uniform.GetExtent();
      for(dax::Id i=0; i <uniform.GetNumberOfCells(); ++i)
        {
        dax::Id3 ijkCell = dax::flatIndexToIndex3Cell(i,extents);
        for(dax::Id j=0; j < numPointsPerCell*totalCells; ++j)
          {
          dax::Id3 ijkPoint = ijkCell + cellVertexToPointIndex[vertexIdList[j]];

          dax::Id pointIndex = index3ToFlatIndex(ijkPoint,extents);
          this->Info.topology.push_back(pointIndex);
          }
        }
    }

  // .............................................................. UniformGrid
  void BuildGrid(dax::cont::UniformGrid<DeviceAdapterTag> &grid)
    {
    grid.SetExtent(dax::make_Id3(0, 0, 0), dax::make_Id3(Size-1, Size-1, Size-1));
    }

  // ............................................................... Hexahedron
  void BuildGrid(
    dax::cont::UnstructuredGrid<
    dax::CellTagHexahedron,
    ArrayContainerControlTag,ArrayContainerControlTag,DeviceAdapterTag>
    &grid)
    {
      //we need to make a volume grid
      dax::cont::UniformGrid<DeviceAdapterTag> uniform;
      this->BuildGrid(uniform);

      //copy the point info over to the unstructured grid
      this->MakeInfoPoints(uniform);

      dax::Id numPointsPerCell =
          dax::CellTraits<dax::CellTagHexahedron>::NUM_VERTICES;
      dax::Id totalCells = 1;
      const dax::Id vertexIdList[] =
        {
          0,1,2,3,4,5,6,7
        };

      this->MakeInfoTopology(uniform,
                             vertexIdList,
                             numPointsPerCell,
                             totalCells);

      grid = dax::cont::UnstructuredGrid<
             dax::CellTagHexahedron,
             ArrayContainerControlTag,
             ArrayContainerControlTag,
             DeviceAdapterTag>(
          this->MakeArrayHandle(this->Info.topology),
          this->MakeArrayHandle(this->Info.points));
    }

  // .............................................................. Tetrahedron
  void BuildGrid(
    dax::cont::UnstructuredGrid<
    dax::CellTagTetrahedron,
    ArrayContainerControlTag,ArrayContainerControlTag,DeviceAdapterTag>
    &grid)
    {
      dax::cont::UniformGrid<DeviceAdapterTag> uniform;
      this->BuildGrid(uniform);

      this->MakeInfoPoints(uniform);

      dax::Id numPointsPerCell
          = dax::CellTraits<dax::CellTagTetrahedron>::NUM_VERTICES;
      dax::Id totalCells = 2;
      const dax::Id vertexIdList[] =
        {
          0,1,4,3,                // Front-Low-Left
          6,7,2,5                 // Back-Top-Right
        };

      this->MakeInfoTopology(uniform,
                             vertexIdList,
                             numPointsPerCell,
                             totalCells);

      grid = dax::cont::UnstructuredGrid<
             dax::CellTagTetrahedron,
             ArrayContainerControlTag,
             ArrayContainerControlTag,
             DeviceAdapterTag>(
          this->MakeArrayHandle(this->Info.topology),
          this->MakeArrayHandle(this->Info.points));
    }

  // .................................................................... Wedge
  void BuildGrid(
    dax::cont::UnstructuredGrid<
    dax::CellTagWedge,
    ArrayContainerControlTag,ArrayContainerControlTag,DeviceAdapterTag>
    &grid)
    {
      dax::cont::UniformGrid<DeviceAdapterTag> uniform;
      this->BuildGrid(uniform);

      this->MakeInfoPoints(uniform);

      dax::Id numPointsPerCell
          = dax::CellTraits<dax::CellTagWedge>::NUM_VERTICES;
      dax::Id totalCells = 2;
      const dax::Id vertexIdList[] =
        {
          4,5,6,0,1,2,                // right-quad -> left bottom edge
          2,3,0,6,7,4                 // left-quad  -> right top edge
        };
      this->MakeInfoTopology(uniform,
                             vertexIdList,
                             numPointsPerCell,
                             totalCells);

      grid = dax::cont::UnstructuredGrid<
             dax::CellTagWedge,
             ArrayContainerControlTag,
             ArrayContainerControlTag,
             DeviceAdapterTag>(
          this->MakeArrayHandle(this->Info.topology),
          this->MakeArrayHandle(this->Info.points));
    }

  // ................................................................. Triangle
  void BuildGrid(
    dax::cont::UnstructuredGrid<
    dax::CellTagTriangle,
    ArrayContainerControlTag,ArrayContainerControlTag,DeviceAdapterTag>
    &grid)
    {
      //we need to make a volume grid
      dax::cont::UniformGrid<DeviceAdapterTag> uniform;
      this->BuildGrid(uniform);

      this->MakeInfoPoints(uniform);

      dax::Id numPointsPerCell =
          dax::CellTraits<dax::CellTagTriangle>::NUM_VERTICES;
      dax::Id totalCells = 12;
      const dax::Id vertexIdList[] =
        {
          0,1,2,2,3,0,                    // Front
          0,3,7,7,4,0,                    // Left
          0,4,5,5,1,0,                    // Bottom
          6,5,4,4,7,6,                    // Back
          6,2,3,3,7,6,                    // Top
          6,5,1,1,2,6                     // Right
        };

      this->MakeInfoTopology(uniform,
                             vertexIdList,
                             numPointsPerCell,
                             totalCells);

      grid = dax::cont::UnstructuredGrid<
             dax::CellTagTriangle,
             ArrayContainerControlTag,
             ArrayContainerControlTag,
             DeviceAdapterTag>(
          this->MakeArrayHandle(this->Info.topology),
          this->MakeArrayHandle(this->Info.points));
    }

  // ............................................................ Quadrilateral
  void BuildGrid(
    dax::cont::UnstructuredGrid<
    dax::CellTagQuadrilateral,
    ArrayContainerControlTag,ArrayContainerControlTag,DeviceAdapterTag>
    &grid)
    {
      dax::cont::UniformGrid<DeviceAdapterTag> uniform;
      this->BuildGrid(uniform);

      this->MakeInfoPoints(uniform);

      dax::Id numPointsPerCell =
          dax::CellTraits<dax::CellTagQuadrilateral>::NUM_VERTICES;
      dax::Id totalCells = 6;
      const dax::Id vertexIdList[] =
        {
          0,1,2,3,              // Front
          0,3,7,4,              // Left
          0,4,5,1,              // Bottom
          6,5,4,7,              // Back
          6,2,3,7,              // Top
          6,5,1,2               // Right
        };

      this->MakeInfoTopology(uniform,
                             vertexIdList,
                             numPointsPerCell,
                             totalCells);

      grid = dax::cont::UnstructuredGrid<
             dax::CellTagQuadrilateral,
             ArrayContainerControlTag,
             ArrayContainerControlTag,
             DeviceAdapterTag>(
          this->MakeArrayHandle(this->Info.topology),
          this->MakeArrayHandle(this->Info.points));
    }

  // ..................................................................... Line
  void BuildGrid(
    dax::cont::UnstructuredGrid<
    dax::CellTagLine,
    ArrayContainerControlTag,ArrayContainerControlTag,DeviceAdapterTag>
    &grid)
    {
      dax::cont::UniformGrid<DeviceAdapterTag> uniform;
      this->BuildGrid(uniform);

      this->MakeInfoPoints(uniform);

      dax::Id numPointsPerCell =
          dax::CellTraits<dax::CellTagLine>::NUM_VERTICES;
      dax::Id totalCells = 12;
      const dax::Id vertexIdList[] =
        {
          0,1,1,2,2,3,3,0,      // Front
          0,3,3,7,7,4,4,0,      // Left
          0,4,4,5,5,1,1,0,      // Bottom
          6,5,5,4,4,7,7,6,      // Back
          6,2,2,3,3,7,7,6,      // Top
          6,5,5,1,1,2,2,6       // Right
        };

      this->MakeInfoTopology(uniform,
                             vertexIdList,
                             numPointsPerCell,
                             totalCells);

      grid = dax::cont::UnstructuredGrid<
              dax::CellTagLine,
              ArrayContainerControlTag,
              ArrayContainerControlTag,
              DeviceAdapterTag>(
          this->MakeArrayHandle(this->Info.topology),
          this->MakeArrayHandle(this->Info.points));
    }

  // ................................................................... Vertex
  void BuildGrid(
    dax::cont::UnstructuredGrid<
    dax::CellTagVertex,
    ArrayContainerControlTag,ArrayContainerControlTag,DeviceAdapterTag>
    &grid)
    {
      dax::cont::UniformGrid<DeviceAdapterTag> uniform;
      this->BuildGrid(uniform);

      this->MakeInfoPoints(uniform);

      dax::Id numPointsPerCell =
          dax::CellTraits<dax::CellTagVertex>::NUM_VERTICES;
      dax::Id totalCells = 8;
      const dax::Id vertexIdList[] =
        {
          0,1,2,3,4,5,6,7
        };

      this->MakeInfoTopology(uniform,
                             vertexIdList,
                             numPointsPerCell,
                             totalCells);

      grid = dax::cont::UnstructuredGrid<
              dax::CellTagVertex,
              ArrayContainerControlTag,
              ArrayContainerControlTag,
              DeviceAdapterTag>(
          this->MakeArrayHandle(this->Info.topology),
          this->MakeArrayHandle(this->Info.points));
    }
};


struct GridTesting
{
private:
  template<class DerivedFunctorType,
           class ArrayContainerControlTag,
           class DeviceAdapterTag>
  struct TryAllGridTypesFunctor
  {
    TryAllGridTypesFunctor(const DerivedFunctorType &functor)
      : Functor(functor) {  }

    template<class CellTag>
    void operator()(CellTag)
    {
      this->CallFunctor(CellTag(), typename dax::CellTraits<CellTag>::GridTag());
    }

  private:
    DerivedFunctorType Functor;
    template<class CellTag>
    void CallFunctor(CellTag, GridTagUniform)
    {
      // This will probably have to change if we support 2D uniform grids.
      dax::cont::UniformGrid<DeviceAdapterTag> grid;
      this->Functor(grid);
    }
    template<class CellTag>
    void CallFunctor(CellTag, GridTagUnstructured)
    {
      dax::cont::UnstructuredGrid<
          CellTag,
          ArrayContainerControlTag,
          ArrayContainerControlTag,
          DeviceAdapterTag> grid;
      this->Functor(grid);
    }
  };

public:
  /// Runs templated \p function on all the grid types defined in Dax. This is
  /// helpful to test templated functions that should work on all grid types. If the
  /// function is supposed to work on some subset of grids or cells, then \p check can
  /// be set to restrict the types of cell tags used. The dax::internal::Testing
  /// structure has some CellCheck* classes the cover the most common types of
  /// cells.
  ///
  template<class FunctionType,
           class CheckType,
           class ArrayContainerControlTag,
           class DeviceAdapterTag>
  static void TryAllGridTypes(FunctionType function,
                              CheckType check,
                              ArrayContainerControlTag,
                              DeviceAdapterTag)
  {
    TryAllGridTypesFunctor<FunctionType,ArrayContainerControlTag,DeviceAdapterTag>
        cellToGridFunctor(function);
    dax::internal::Testing::TryAllCells(cellToGridFunctor, check);
  }
  template<class FunctionType,
           class ArrayContainerControlTag,
           class DeviceAdapterTag>
  static void TryAllGridTypes(FunctionType function,
                              ArrayContainerControlTag,
                              DeviceAdapterTag)
  {
    TryAllGridTypes(function,
                    dax::internal::Testing::CellCheckAlwaysTrue(),
                    ArrayContainerControlTag(),
                    DeviceAdapterTag());
  }
  template<class FunctionType, class CheckType>
  static void TryAllGridTypes(FunctionType function, CheckType check)
  {
    TryAllGridTypes(function,
                    check,
                    DAX_DEFAULT_ARRAY_CONTAINER_CONTROL_TAG(),
                    DAX_DEFAULT_DEVICE_ADAPTER_TAG());
  }
  template<class FunctionType>
  static void TryAllGridTypes(FunctionType function)
  {
    TryAllGridTypes(function, dax::internal::Testing::CellCheckAlwaysTrue());
  }

};


}
}
} //namespace dax::cont::internal

#endif //  __dax_cont_internal_TestingGridGenerator_h
