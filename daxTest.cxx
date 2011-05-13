/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "dAPI.cl.h"


#include "daxCellGradientModule.h"
#include "daxCellGradientModule2.h"
#include "daxCellDataToPointDataModule.h"
#include "daxElevationModule.h"
#include "daxExecutive.h"
#include "daxOptions.h"

#include <vector>
#include <string.h>
#include <assert.h>
#include <boost/progress.hpp>

#define PIPELINE_BASE 1 // Distance->CellGradient
#define PIPELINE_C2P 2// Distance->CellGradient->CellToPoint
#define PIPELINE_Gradient2 3 // Distance->CellGradient->CellToPoint->CellGradient

#define uchar unsigned char
struct daxArrayCore
{
  uchar Type; // 0 -- irregular
            // 1 -- image-data points array
            // 2 -- image-data connections array
  uchar Rank;
  uchar Shape[2];
} __attribute__((__packed__));

struct daxImageDataData
{
  float Spacing[3];
  float Origin[3];
  unsigned int Extents[6];
} __attribute__((__packed__));

void daxExecute(int num_items,
  int num_cores, daxArrayCore* cores,
  int num_in_arrays, float** in_arrays, size_t* in_arrays_size_in_bytes,
  int num_out_arrays, float** out_arrays, size_t* out_arrays_size_in_bytes,
  int num_kernels, const std::string* kernels)
{
}

#include <boost/program_options.hpp>
#include <vector>
#include <string.h>

namespace po = boost::program_options;

int main(int argc, char** argv)
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("pipeline", po::value<int>(), "Pipeline mode (1,2 or 3) default: 1")
    ("dimensions", po::value<int>(), "Dimensions (default 256)")
    ("help", "Generate this help message");

  po::variables_map variables;
  po::store(po::parse_command_line(argc, argv, desc), variables);
  po::notify(variables);

  if (variables.count("help") != 0)
    {
    cout << desc << endl;
    return 1;
    }

 
  int DIMENSION = 256;
  int PIPELINE = PIPELINE_BASE;
  if (variables.count("dimensions") == 1)
    {
    DIMENSION = variables["dimensions"].as<int>();
    }
  if (variables.count("pipeline") == 1)
    {
    PIPELINE = variables["pipeline"].as<int>();
    }

  daxExecutivePtr executive(new daxExecutive());
  daxModulePtr elevation(new daxElevationModule());
  daxModulePtr gradient(new daxCellGradientModule());
  daxModulePtr cd2pd(new daxCellDataToPointDataModule());
  daxModulePtr gradient2(new daxCellGradientModule2());

  std::vector<std::string> kernels;
  kernels.push_back(elevation->GetCleandupFunctorCode());
  kernels.push_back(gradient->GetCleandupFunctorCode());
  if (PIPELINE >= PIPELINE_C2P)
    {
    kernels.push_back(cd2pd->GetCleandupFunctorCode());
    if (PIPELINE >= PIPELINE_Gradient2)
      {
      kernels.push_back(gradient2->GetCleandupFunctorCode());
      }
    }

  executive->Connect(elevation, "output", gradient, "input_point");
  if (PIPELINE >= PIPELINE_C2P)
    {
    executive->Connect(gradient, "output_cell", cd2pd, "in_cell_array");
    if (PIPELINE >= PIPELINE_Gradient2)
      {
      executive->Connect(cd2pd, "out_point_array", gradient2, "input_point");
      }
    }
  kernels.push_back(executive->GetKernel());

  // this pipeline has 10 arrays, 7 are global (6-in, 1-out) while other 3 are
  // internal arrays.
  daxArrayCore cores[10];
  float* global_arrays[7];
  size_t global_array_size_in_bytes[7];

  // First set up the array cores.
  
  // Output from Elevation
  cores[0].Type = 0; //unstructured.
  cores[0].Rank = 0;
  cores[0].Shape[0] = cores[0].Shape[1] = 0;

  // Input: Elevation (positions)
  cores[1].Type = 1; // input image points coordinates
  cores[1].Rank = 1; // vectors
  cores[1].Shape[0] = 3; cores[1].Shape[1] = 0;

  // Output from CellGradient
  cores[2].Type = 0; // irregular
  cores[2].Rank = 1;
  cores[2].Shape[0] = 3; cores[2].Shape[1] = 0;

  // Input: CellGradient(positions);
  cores[3] = cores[1];

  // Input: CellGradient(connections):
  cores[4].Type = 2; // image cell.
  cores[4].Rank = 1;
  cores[4].Shape[0] = 8; cores[4].Shape[1] = 0;

  // Output: CellDataToPointData 
  // type matches the input i.e the output from cell gradient.
  cores[5] = cores[2];

  // Input: CellDataToPointData(cell_links)
  cores[6].Type = 3; // image cell-link
  cores[6].Rank = 1;
  cores[6].Shape[0] = 8; cores[6].Shape[1] = 0;

  // Output: CellGradient2
  cores[7] = cores[2];

  // Input: CellGradient2(positions)
  cores[8] = cores[3];

  // Input: CellGradient2(connections)
  cores[9] = cores[4];

  // Now allocate and initialize all the global arrays (7:in, 1:out)
  daxImageDataData points;
  points.Spacing[0] = points.Spacing[1] = points.Spacing[2] = 1.0f;
  points.Origin[0] = points.Origin[1] = points.Origin[2] = 0.0f;
  points.Extents[0] = points.Extents[2] = points.Extents[4] = 0;
  points.Extents[1] = points.Extents[3] = points.Extents[5] = DIMENSION -1;

  // Elevation(positions)
  global_arrays[0] = reinterpret_cast<float*>(&points);
  global_array_size_in_bytes[0] = sizeof(points);

  // CellGradient(positions) == Elevation(positions)
  global_arrays[1] = global_arrays[0];
  global_array_size_in_bytes[1] = global_array_size_in_bytes[0];

  // CellGradient(connections) (~) Elevation(positions) for 3d image
  global_arrays[2] = reinterpret_cast<float*>(&points);
  global_array_size_in_bytes[2] = sizeof(points);

  // CellGradient(cell_links) (~) Elevation(positions) for 3d image
  global_arrays[3] = reinterpret_cast<float*>(&points);
  global_array_size_in_bytes[3] = sizeof(points);

  // CellGradient2(positions) == Elevation(positions)
  global_arrays[4] = global_arrays[0];
  global_array_size_in_bytes[4] = global_array_size_in_bytes[0];

  // CellGradient2(connections) (~) Elevation(positions) for 3d image
  global_arrays[5] = reinterpret_cast<float*>(&points);
  global_array_size_in_bytes[5] = sizeof(points);

  // CellGradient2(output)
  int output_dims = (PIPELINE == PIPELINE_C2P)? DIMENSION: DIMENSION-1;
  global_arrays[6] = new float[output_dims*output_dims*output_dims* 3];
  for (int cc=0; cc < (output_dims*output_dims*output_dims*3); cc++)
    {
    global_arrays[6][cc] = -1;
    }
  global_array_size_in_bytes[6] = output_dims*output_dims*output_dims*sizeof(float)*3;

  boost::timer timer;
  //daxExecute(10, cores, 6, global_arrays, global_array_size_in_bytes,
  //  1, &global_arrays[6], &global_array_size_in_bytes[6],
  //  kernels.size(), &kernels[0]);
  int num_inputs = 3;
  if (PIPELINE == PIPELINE_C2P)
    {
    num_inputs = 4;
    }
  else if (PIPELINE == PIPELINE_Gradient2)
    {
    num_inputs = 6;
    }
  daxExecute(output_dims*output_dims*output_dims,
    10, cores, num_inputs, global_arrays, global_array_size_in_bytes,
    1, &global_arrays[6], &global_array_size_in_bytes[6],
    kernels.size(), &kernels[0]);
  cout << "Total Time ("<< DIMENSION << "^3) : "<< timer.elapsed() << endl;
  cout << "Output (1): " << global_arrays[6][0] << endl;
  for (int cc=0; cc < global_array_size_in_bytes[6]/sizeof(float); cc++)
    {
    cout << global_arrays[6][cc] << endl;
    if (cc == 11)
      {
      break;
      }
    }

  delete [] global_arrays[6];
  return 0;
}
