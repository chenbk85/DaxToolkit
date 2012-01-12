/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __dax_cont_internal_ArrayContainerExecution_h
#define __dax_cont_internal_ArrayContainerExecution_h

#include <dax/Types.h>

// TODO: Come up with a better way to choose the appropriate implementation
// for ArrayContainerExecution.
#ifdef DAX_CUDA
#include <dax/cuda/cont/internal/ArrayContainerExecution.h>
namespace dax {
namespace cont {
namespace internal {
template<typename T>
class ArrayContainerExecution
    : public dax::cuda::cont::internal::ArrayContainerExecution<T>
{ };
}
}
}
#else

#include <dax/internal/DataArray.h>

#include <dax/cont/internal/IteratorContainer.h>

#include <vector>

#include <assert.h>

namespace dax {
namespace cont {
namespace internal {

/// Manages an execution environment array, which may need to be allocated
/// on seperate device memory.
template<typename T>
class ArrayContainerExecution
{
public:
  typedef T ValueType;

  /// On inital creation, no memory is allocated on the device.
  ///
  ArrayContainerExecution() { }

  /// Allocates an array on the device large enough to hold the given number of
  /// entries.
  ///
  void Allocate(dax::Id numEntries) { this->DeviceArray.resize(numEntries); }

  /// Copies the data pointed to by the passed in \c iterators (assumed to be
  /// in the control environment), into the array in the execution environment
  /// managed by this class.
  ///
  template<class IteratorType>
  void CopyFromControlToExecution(
      const dax::cont::internal::IteratorContainer<IteratorType> &);

  /// Copies the data from the array in the execution environment managed by
  /// this class into the memory passed in the \c iterators (assumed to be in
  /// the control environment).
  ///
  template<class IteratorType>
  void CopyFromExecutionToControl(
      const dax::cont::internal::IteratorContainer<IteratorType> &);

  /// Frees any resources (i.e. memory) on the device.
  ///
  void ReleaseResources() { this->Allocate(0); }

  /// Gets a DataArray that is valid in the execution environment.
  dax::internal::DataArray<ValueType> GetExecutionArray();

private:
  ArrayContainerExecution(const ArrayContainerExecution &); // Not implemented
  void operator=(const ArrayContainerExecution &);          // Not implemented

  std::vector<ValueType> DeviceArray;
};

//-----------------------------------------------------------------------------
template<class T>
template<class IteratorType>
inline void ArrayContainerExecution<T>::CopyFromControlToExecution(
    const dax::cont::internal::IteratorContainer<IteratorType> &iterators)
{
  assert(iterators.IsValid());
  assert(iterators.GetNumberOfEntries()
         == static_cast<dax::Id>(this->DeviceArray.size()));
  std::copy(iterators.GetBeginIterator(),
            iterators.GetEndIterator(),
            this->DeviceArray.begin());
}

//-----------------------------------------------------------------------------
template<class T>
template<class IteratorType>
inline void ArrayContainerExecution<T>::CopyFromExecutionToControl(
    const dax::cont::internal::IteratorContainer<IteratorType> &iterators)
{
  assert(iterators.IsValid());
  assert(iterators.GetNumberOfEntries()
         == static_cast<dax::Id>(this->DeviceArray.size()));
  std::copy(this->DeviceArray.begin(),
            this->DeviceArray.end(),
            iterators.GetBeginIterator());
}

//-----------------------------------------------------------------------------
template<class T>
inline dax::internal::DataArray<T>
ArrayContainerExecution<T>::GetExecutionArray()
{
  ValueType *rawPointer = &this->DeviceArray[0];
  dax::Id numEntries = this->DeviceArray.size();
  return dax::internal::DataArray<ValueType>(rawPointer, numEntries);
}

}
}
}

#endif // DAX_CUDA

#endif //__dax_cont_internal_ArrayContainerExecution_h
