#ifndef __dax_exec_internal_ExtractTopology_h
#define __dax_exec_internal_ExtractTopology_h

#include <dax/Types.h>
#include <dax/exec/Cell.h>
#include <dax/exec/Field.h>
#include <dax/exec/WorkMapCell.h>

#include <dax/cont/DeviceAdapter.h>
#include <dax/cont/internal/ExecutionPackageField.h>
#include <dax/cont/internal/ExecutionPackageGrid.h>

namespace dax {
namespace exec {
namespace kernel {
namespace internal {


template <typename CellType>
DAX_WORKLET void ExtractTopology(dax::exec::WorkMapCell<CellType> work,
                                 dax::exec::Field<dax::Id> &topology)
  {
  CellType cell(work.GetCell());
  dax::Id index(work.GetCellIndex());
  dax::Id offset(CellType::NUM_POINTS * index);

  //manual unrolled in an attempt to make this faster
  dax::Id* temp = topology.GetArray().GetPointer()+offset;
  for(dax::Id i=0;i < CellType::NUM_POINTS; ++i)
    {
    temp[i] = cell.GetPointIndex(i);
    }
  }

template<class CellType>
struct ExtractTopologyParameters
{
  typename CellType::TopologyType grid;
  dax::exec::Field<dax::Id> outField;
};

template<class CellType>
struct ExtractTopologyFunctor {
  DAX_EXEC_EXPORT void operator()(
      ExtractTopologyParameters<CellType> &parameters,
      dax::Id index,
      const dax::exec::internal::ErrorHandler &errorHandler)
  {
    dax::exec::WorkMapCell<CellType> work(parameters.grid, errorHandler);
    work.SetCellIndex(index);
    dax::exec::kernel::internal::ExtractTopology(work,parameters.outField);
  }
};

}
}
}
} //dax::exec::kernel::internal

namespace dax {
namespace cont {
namespace internal {

template<typename DeviceAdapter, typename GridType>
class ExtractTopology
{
public:

  /// Extract the topology for all the cells in the grid.
  /// The resulting array contains the point ids for each cell.
  ExtractTopology(const GridType& grid)
    {
    DoExtract(grid,dax::cont::ArrayHandle<dax::Id>(),false);
    }

  /// Extract a subset of the cells topology. cellsToExtract contain
  /// The cell ids to extract. The resulting array contains only
  /// the point ids for each cell, so to the point ids of the third
  /// cell would be in positions CellType::NUM_POINTS * 3 to (CellType::NUM_POINTS * 4) - 1
  ExtractTopology(const GridType& grid,
                  const dax::cont::ArrayHandle<dax::Id> &cellsToExtract)
    {
    DoExtract(grid,cellsToExtract,true);
    }

  /// Returns an array handle to the execution enviornment data that
  /// contians the extracted topology
  dax::cont::ArrayHandle<dax::Id> GetTopology();

private:
  void DoExtract(const GridType& grid,
            const dax::cont::ArrayHandle<dax::Id> &cellsToExtract,
            bool useSubSet)
    {
    typedef dax::cont::internal::ExecutionPackageGrid<GridType> GridPackageType;

    typedef typename GridPackageType::ExecutionCellType CellType;

    //construct the input grid
    GridPackageType inPGrid(grid);

    //construct the topology result array
    dax::Id numCellsToExtract=(useSubSet)? cellsToExtract.GetNumberOfEntries():
                                           grid.GetNumberOfCells();
    dax::Id size = numCellsToExtract * CellType::NUM_POINTS;

    this->Topology = dax::cont::ArrayHandle<dax::Id>(size);

    //we want the size of the points to be based on the numCells * points per cell
    dax::cont::internal::ExecutionPackageFieldInput<dax::Id,DeviceAdapter>
        result(this->Topology, size);

    //construct the parameters list for the function
    dax::exec::kernel::internal::ExtractTopologyParameters<CellType> etParams =
                                        {
                                        inPGrid.GetExecutionObject(),
                                        result.GetExecutionObject()
                                        };
    if(useSubSet)
      {
      DeviceAdapter::Schedule(
          dax::exec::kernel::internal::ExtractTopologyFunctor<CellType>(),
          etParams,
          cellsToExtract);
      }
    else
      {
      DeviceAdapter::Schedule(
        dax::exec::kernel::internal::ExtractTopologyFunctor<CellType>(),
        etParams,
        numCellsToExtract);
      }
    }

  dax::cont::ArrayHandle<dax::Id> Topology;
};

}
}
}
#endif // __dax_exec_internal_ExtractTopology_h
