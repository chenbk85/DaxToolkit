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

#ifndef __dax_cont_DeviceAdapter_h
#define __dax_cont_DeviceAdapter_h

#include <dax/internal/ExportMacros.h>

#ifndef DAX_DEFAULT_DEVICE_ADAPTER
#ifdef DAX_CUDA
#include <dax/cuda/cont/DeviceAdapterCuda.h>
#else // DAX_CUDA
#ifdef DAX_OPENMP
#include <dax/openmp/cont/DeviceAdapterOpenMP.h>
#else // DAX_OPENMP
#include <dax/cont/DeviceAdapterDebug.h>
#endif // DAX_OPENMP
#endif // DAX_CUDA
#endif // DAX_DEFAULT_DEVICE_ADAPTER

namespace dax {
namespace cont {

/// \class DeviceAdapter DeviceAdapter.h dax/cont/DeviceAdapter.h
/// \brief Interface between generic Dax classes and parallel devices.
///
/// A DeviceAdapter is a class with several classes static methods that provide
/// mechanisms to run algorithms on a type of parallel device. The class
/// dax::cont::DeviceAdapter does not actually exist. Rather, this
/// documentation is provided to describe the interface for a DeviceAdapter.
/// Loading the dax/cont/DeviceAdapter.h header file will set a default device
/// adapter appropriate for the current compile environment. The default
/// adapter can be overloaded by including the header file for a different
/// adapter (for example, DeviceAdapterDebug.h). This overloading should be
/// done \em before loading in any other Dax header files. Failing to do so
/// could create inconsistencies in the default adapter used amongst classes.

/// \fn template<class Functor, class Parameters> const char *DeviceAdapter::Schedule(Functor functor, Parameters parameters, dax::Id numInstances)
/// \brief Schedule many instances of a function to run on concurrent threads.
///
/// Calls the \c functor on several threads. This is the function used in the
/// control environment to spawn activity in the execution environment.
/// \c functor is a function-like object that can be invoked with the calling
/// specification <tt>functor(Parameters parameters, dax::Id index,
/// dax::exec::internal::ErrorHandler errorHandler)</tt>. The first argument is
/// the \c parameters passed through. The second argument uniquely identifies
/// the thread or instance of the invocation. There should be one invocation
/// for each index in the range [0, \c numInstances]. The third argment
/// contains an ErrorHandler that can be used to raise an error in the functor.
/// If an error is raised, the string that was raised is returned. Otherwise,
/// NULL or a zero-length string is returned.

/// \class template<class T> DeviceAdapter::ArrayContainerExecution<T>
/// \brief Class that manages data in the execution environment.
///
/// This is a class that is responsible for allocating data in the execution
/// environment and copying data back and forth between control and execution
/// environment. It is also expected that this class will automatically release
/// any resources in its destructor.

/// \fn void DeviceAdapter::ArrayContainerExecution::Allocate(dax::Id numEntries)
///
/// Allocates an array on the device large enough to hold the given number of
/// entries.

/// \fn template<class Iterator> void DeviceAdapter::ArrayContainerExecution::CopyFromControlToExecution(const dax::cont::internal::IteratorContainer<Iterator> &iterators)
///
/// Copies the data pointed to by the passed in \c iterators (assumed to be
/// in the control environment), into the array in the execution environment
/// managed by this class.

/// \fn template<class Iterator> void DeviceAdapter::ArrayContainerExecution::CopyFromExecutionToControl(const dax::cont::internal::IteratorContainer<Iterator> &iterators)
///
/// Copies the data from the array in the execution environment managed by
/// this class into the memory passed in the \c iterators (assumed to be in
/// the control environment).

/// \fn void DeviceAdapter::ArrayContainerExecution::ReleaseResources()
///
/// Frees any resources (i.e. memory) on the device.

/// \fn dax::internal::DataArray<T> DeviceAdapter::ArrayContainerExecution::GetExecutionArray()
///
/// Gets a DataArray that is valid in the execution environment.


}
}

#endif //__dax_cont_DeviceAdapter_h