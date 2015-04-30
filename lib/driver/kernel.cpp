#include "isaac/driver/kernel.h"
#include "isaac/driver/buffer.h"
#include <iostream>

namespace isaac
{

namespace driver
{

Kernel::Kernel(Program const & program, const char * name) : backend_(program.backend_), address_bits_(program.context().device().address_bits()), h_(backend_)
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA:
      cu_params_store_.reserve(32);
      cu_params_.reserve(32);
      cuda::check(cuModuleGetFunction(h_.cu.get(), *program.h_.cu, name));\
      break;
#endif
    case OPENCL:
      *h_.cl = cl::Kernel(*program.h_.cl, name);
      break;
    default:
      throw;
  }
}

void Kernel::setArg(unsigned int index, std::size_t size, void* ptr)
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA:
      if(index + 1> cu_params_store_.size())
      {
        cu_params_store_.resize(index+1);
        cu_params_.resize(index+1);
      }
      cu_params_store_[index].reset(malloc(size), free);
      memcpy(cu_params_store_[index].get(), ptr, size);
      cu_params_[index] = cu_params_store_[index].get();
      break;
#endif
    case OPENCL:
      h_.cl->setArg(index, size, ptr);
      break;
    default:
      throw;
  }
}

void Kernel::setArg(unsigned int index, Buffer const & data)
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA:
    {
      setArg(index, sizeof(CUdeviceptr), data.h_.cu.get()); break;
    }
#endif
    case OPENCL: h_.cl->setArg(index, *data.h_.cl); break;
    default: throw;
  }
}

void Kernel::setSizeArg(unsigned int index, size_t N)
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA:
    {
      int NN = N;
      setArg(index, sizeof(int), &NN);
      break;
    }
#endif
    case OPENCL:
      if(address_bits_==32){
        cl_int NN = N;
        h_.cl->setArg(index, 4, &NN);
      }
      else if(address_bits_==64)
      {
        cl_long NN = N;
        h_.cl->setArg(index, 8, &NN);
      }
      else
        throw;
      break;

    default: throw;
  }
}

}

}
