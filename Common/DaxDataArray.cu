/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "DaxDataArray.h"
#include "assert.h"

//-----------------------------------------------------------------------------
DaxDataArray DaxDataArray::CreateAndCopy(
  eType type, eDataType dataType,
  unsigned int data_size_in_bytes, void* raw_data)
{
  DaxDataArray array = Create(type, dataType, data_size_in_bytes);
  assert(array.SizeInBytes == data_size_in_bytes);
  if (data_size_in_bytes > 0)
    {
    assert(array.RawData != NULL);
    cudaMemcpy(array.RawData, raw_data, data_size_in_bytes,
      cudaMemcpyHostToDevice);
    }
  return array;
}

//-----------------------------------------------------------------------------
DaxDataArray DaxDataArray::Create(
  eType type, eDataType dataType, unsigned int data_size_in_bytes)
{
  DaxDataArray array;
  array.Type = type;
  array.DataType = dataType;
  array.SizeInBytes = data_size_in_bytes;
  array.RawData = NULL;
  if (data_size_in_bytes > 0)
    {
    cudaMalloc(&array.RawData, data_size_in_bytes);
    }
  return array;
}