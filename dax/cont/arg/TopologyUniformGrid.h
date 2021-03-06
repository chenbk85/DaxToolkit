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
#ifndef __dax_cont_arg_TopologyUniformGrid_h
#define __dax_cont_arg_TopologyUniformGrid_h

#include <dax/Types.h>
#include <dax/internal/Tags.h>
#include <dax/cont/arg/ConceptMap.h>
#include <dax/cont/arg/Topology.h>
#include <dax/cont/sig/Tag.h>

#include <dax/exec/arg/TopologyCell.h>
#include <dax/cont/UniformGrid.h>

#include <boost/mpl/if.hpp>

namespace dax { namespace cont { namespace arg {

/// \headerfile TopologyUniformGrid.h dax/cont/arg/TopologyUniformGrid.h
/// \brief Map an uniform grid to an execution side cell topology parameter
template <typename Tags, typename DeviceTag >
class ConceptMap<Topology(Tags), dax::cont::UniformGrid< DeviceTag > >
{
  typedef dax::cont::UniformGrid< DeviceTag > GridType;

  //use mpl::if_ to determine the type for ExecArg
  typedef typename boost::mpl::if_<
      typename Tags::template Has<dax::cont::sig::Out>,
      typename GridType::TopologyStructExecution,
      typename GridType::TopologyStructConstExecution>::type TopologyType;

  typedef dax::exec::arg::TopologyCell<Tags,TopologyType> ExecGridType;
  GridType Grid;
  TopologyType Topology;

public:
  //All Topology binding classes must export the cell tag and grid tag
  //This allows us to do better scheduling based on cell / grid types
  typedef typename GridType::CellTag CellTypeTag;
  typedef typename GridType::GridTypeTag GridTypeTag;

  typedef GridType ContArg;
  typedef ExecGridType ExecArg;
  typedef typename dax::cont::arg::SupportedDomains<dax::cont::sig::Cell>::Tags DomainTags;

  DAX_CONT_EXPORT ConceptMap(GridType g): Grid(g) {}

  DAX_CONT_EXPORT ExecArg GetExecArg() { return ExecGridType(Topology); }

  //All topology fields are required by scheduler to expose the cont arg
  DAX_CONT_EXPORT const ContArg& GetContArg() const { return this->Grid; }

  DAX_CONT_EXPORT void ToExecution(dax::Id, boost::false_type)
    { /* Input  */
    this->Topology = this->Grid.PrepareForInput();
    }

  //we need to pass the number of elements to allocate
  DAX_CONT_EXPORT void ToExecution(dax::Id size)
    {
    ToExecution(size,typename Tags::template Has<dax::cont::sig::Out>());
    }

  DAX_CONT_EXPORT dax::Id GetDomainLength(sig::Point) const
    {
    return Grid.GetNumberOfPoints();
    }

  DAX_CONT_EXPORT dax::Id GetDomainLength(sig::Cell) const
    {
    return Grid.GetNumberOfCells();
    }
};

/// \headerfile TopologyUniformGrid.h dax/cont/arg/TopologyUniformGrid.h
/// \brief Map an uniform grid to an execution side cell topology parameter
template <typename Tags, typename DeviceTag >
class ConceptMap<Topology(Tags), const dax::cont::UniformGrid< DeviceTag > >
{
  typedef dax::cont::UniformGrid< DeviceTag > GridType;
  typedef typename GridType::TopologyStructConstExecution TopologyType;
  typedef dax::exec::arg::TopologyCell<Tags,TopologyType> ExecGridType;
  GridType Grid;
  TopologyType Topology;

public:
  //All Topology binding classes must export the cell tag and grid tag
  //This allows us to do better scheduling based on cell / grid types
  typedef typename GridType::CellTag CellTypeTag;
  typedef typename GridType::GridTypeTag GridTypeTag;

  typedef GridType ContArg;
  typedef ExecGridType ExecArg;
  typedef typename dax::cont::arg::SupportedDomains<dax::cont::sig::Cell>::Tags DomainTags;

  ConceptMap(GridType g): Grid(g) {}

  ExecArg GetExecArg() { return ExecGridType(Topology); }

  //All topology fields are required by scheduler to expose the cont arg
  DAX_CONT_EXPORT const ContArg& GetContArg() const { return this->Grid; }

  void ToExecution(dax::Id, boost::false_type)
    { /* Input  */
    this->Topology = this->Grid.PrepareForInput();
    }

  //we need to pass the number of elements to allocate
  void ToExecution(dax::Id size)
    {
    ToExecution(size,typename Tags::template Has<dax::cont::sig::Out>());
    }

  dax::Id GetDomainLength(sig::Point) const
    {
    return Grid.GetNumberOfPoints();
    }

  dax::Id GetDomainLength(sig::Cell) const
    {
    return Grid.GetNumberOfCells();
    }
};


}}} // namespace dax::cont::arg

#endif //__dax_cont_arg_TopologyUniformGrid_h
