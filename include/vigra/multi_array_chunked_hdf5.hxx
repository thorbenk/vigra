/************************************************************************/
/*                                                                      */
/*     Copyright 2012-2014 by Ullrich Koethe and Thorben Kroeger        */
/*                                                                      */
/*    This file is part of the VIGRA computer vision library.           */
/*    The VIGRA Website is                                              */
/*        http://hci.iwr.uni-heidelberg.de/vigra/                       */
/*    Please direct questions, bug reports, and contributions to        */
/*        ullrich.koethe@iwr.uni-heidelberg.de    or                    */
/*        vigra@informatik.uni-hamburg.de                               */
/*                                                                      */
/*    Permission is hereby granted, free of charge, to any person       */
/*    obtaining a copy of this software and associated documentation    */
/*    files (the "Software"), to deal in the Software without           */
/*    restriction, including without limitation the rights to use,      */
/*    copy, modify, merge, publish, distribute, sublicense, and/or      */
/*    sell copies of the Software, and to permit persons to whom the    */
/*    Software is furnished to do so, subject to the following          */
/*    conditions:                                                       */
/*                                                                      */
/*    The above copyright notice and this permission notice shall be    */
/*    included in all copies or substantial portions of the             */
/*    Software.                                                         */
/*                                                                      */
/*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND    */
/*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES   */
/*    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND          */
/*    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT       */
/*    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,      */
/*    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      */
/*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR     */
/*    OTHER DEALINGS IN THE SOFTWARE.                                   */                
/*                                                                      */
/************************************************************************/

#ifndef VIGRA_MULTI_ARRAY_CHUNKED_HDF5_HXX
#define VIGRA_MULTI_ARRAY_CHUNKED_HDF5_HXX

#include <queue>

#include "multi_array_chunked.hxx"
#include "hdf5impex.hxx"

// Bounds checking Macro used if VIGRA_CHECK_BOUNDS is defined.
#ifdef VIGRA_CHECK_BOUNDS
#define VIGRA_ASSERT_INSIDE(diff) \
  vigra_precondition(this->isInside(diff), "Index out of bounds")
#else
#define VIGRA_ASSERT_INSIDE(diff)
#endif

namespace vigra {

template <unsigned int N, class T, class Alloc = std::allocator<T> >
class ChunkedArrayHDF5
: public ChunkedArray<N, T>
{
  public:
    
    class Chunk
    : public ChunkBase<N, T>
    {
      public:
        typedef typename MultiArrayShape<N>::type  shape_type;
        typedef T value_type;
        typedef value_type * pointer;
        typedef value_type & reference;
        
        Chunk()
        : ChunkBase<N, T>(),
          array_(0)
        {}
        
        ~Chunk()
        {
            write();
        }
    
        void reshape(shape_type const & shape, shape_type const & start, 
                     ChunkedArrayHDF5 * array)
        {
            vigra_precondition(this->pointer_ == (void*)0,
                "ChunkedArrayCompressed::Chunk::reshape(): chunk was already allocated.");
            this->shape_ = shape;
            this->strides_ = detail::defaultStride(shape);
            start_ = start;
            array_ = array;
        }
        
        void write()
        {
            pointer p = this->pointer_.exchange(0, threading::memory_order_release);
            if(p != 0)
            {
                if(!array_->file_.isReadOnly())
                {
                    herr_t status = array_->file_.writeBlock(array_->dataset_, start_, storage_);
                    vigra_postcondition(status >= 0,
                        "ChunkedArrayHDF5: write to dataset failed.");
                }
                storage_.swap(MultiArray<N, T>());
            }
        }
        
        pointer read()
        {
            pointer p = this->pointer_.load(threading::memory_order_acquire);
            if(p == 0)
            {
                storage_.reshape(this->shape_);
                herr_t status = array_->file_.readBlock(array_->dataset_, start_, this->shape_, storage_);
                vigra_postcondition(status >= 0,
                    "ChunkedArrayHDF5: read from dataset failed.");
                p = storage_.data();
                this->pointer_.store(p, threading::memory_order_release);
            }
            return p;
        }
        
        MultiArray<N, T> storage_;
        shape_type start_;
        ChunkedArrayHDF5 * array_;
    };
    
    typedef MultiArray<N, Chunk> ChunkStorage;
    typedef typename ChunkStorage::difference_type  shape_type;
    typedef T value_type;
    typedef value_type * pointer;
    typedef value_type & reference;
    
    ChunkedArrayHDF5(HDF5File const & file, std::string const & dataset,
                     HDF5File::OpenMode mode,
                     shape_type const & shape,
                     shape_type const & chunk_shape=shape_type(), 
                     int cache_max = -1)
    : ChunkedArray<N, T>(shape, chunk_shape, cache_max),
      file_(file),
      dataset_name_(dataset),
      dataset_(),
      outer_array_()
    {
        bool exists = file_.existsDataset(dataset_name_);
        
        vigra_precondition(exists || mode != HDF5File::OpenReadOnly,
            "ChunkedArrayHDF5(): cannot open non-existing dataset in read-only mode.");
            
        if(!exists || mode == HDF5File::New)
        {
            vigra_precondition(prod(shape) > 0,
                "ChunkedArrayHDF5(): invalid shape.");
            dataset_ = file_.createDatasetImpl<T>(dataset_name_, 
                                                  shape, this->chunk_shape_, 
                                                  ZLIB_FAST);
        }
        file_.setReadOnly(mode == HDF5File::OpenReadOnly);
        init(shape);
    }
    
    ChunkedArrayHDF5(HDF5File const & file, std::string const & dataset,
                     HDF5File::OpenMode mode,
                     CompressionMethod compression,
                     shape_type const & shape,
                     shape_type const & chunk_shape=shape_type(), 
                     int cache_max = -1)
    : ChunkedArray<N, T>(shape, chunk_shape, cache_max),
      file_(file),
      dataset_name_(dataset),
      dataset_(),
      outer_array_()
    {
        bool exists = file_.existsDataset(dataset_name_);
        
        vigra_precondition(exists || mode != HDF5File::OpenReadOnly,
            "ChunkedArrayHDF5(): cannot open non-existing dataset in read-only mode.");
            
        if(!exists || mode == HDF5File::New)
        {
            vigra_precondition(prod(shape) > 0,
                "ChunkedArrayHDF5(): invalid shape.");
            vigra_precondition(compression != LZ4,
                "ChunkedArrayHDF5(): LZ4 compression is unsupported.");
            dataset_ = file_.createDatasetImpl<T>(dataset_name_, 
                                                  shape, this->chunk_shape_, 
                                                  compression);
        }
        file_.setReadOnly(mode == HDF5File::OpenReadOnly);
        init(shape);
    }
    
    ChunkedArrayHDF5(HDF5File const & file, std::string const & dataset,
                     int cache_max = -1)
    : ChunkedArray<N, T>(shape_type(), shape_type(), cache_max),
      file_(file),
      dataset_name_(dataset),
      dataset_(),
      outer_array_()
    {
        vigra_precondition(file_.existsDataset(dataset_name_),
                           "ChunkedArrayHDF5(file, dataset): dataset does not exist.");
                           
        file_.setReadOnly();
        init();
    }
    
    void init(shape_type const & desired_shape = shape_type())
    {
        dataset_ = file_.getDatasetHandleShared(dataset_name_);
            
        // check shape
        ArrayVector<hsize_t> fileShape(file_.getDatasetShape(dataset_name_));
        typedef detail::HDF5TypeTraits<T> TypeTraits;
        if(TypeTraits::numberOfBands() > 1)
        {
            vigra_precondition(fileShape.size() == N+1,
                "ChunkedArrayHDF5(file, dataset): dataset has wrong dimension.");
            vigra_precondition(fileShape[0] == TypeTraits::numberOfBands(),
                "ChunkedArrayHDF5(file, dataset): dataset has wrong number of bands.");
            if(prod(desired_shape) > 0)
                for(unsigned int k=0; k<N; ++k)
                    vigra_precondition(fileShape[k+1] == desired_shape[k],
                        "ChunkedArrayHDF5(file, dataset, shape): shape mismatch between dataset and shape argument.");
            this->shape_ = shape_type(fileShape.begin()+1);
        }
        else
        {
            vigra_precondition(fileShape.size() == N,
                "ChunkedArrayHDF5(file, dataset): dataset has wrong dimension.");
            if(prod(desired_shape) > 0)
                for(unsigned int k=0; k<N; ++k)
                    vigra_precondition(fileShape[k] == desired_shape[k],
                        "ChunkedArrayHDF5(file, dataset, shape): shape mismatch between dataset and shape argument.");
            this->shape_ = shape_type(fileShape.begin());
        }
        outer_array_.reshape(detail::computeChunkArrayShape(this->shape_, this->bits_, this->mask_));
        
        // set shape of the chunks
        typename ChunkStorage::iterator i   = outer_array_.begin(), 
                                        end = outer_array_.end();
        for(; i != end; ++i)
        {
            shape_type start = i.point()*this->chunk_shape_;
            i->reshape(min(this->chunk_shape_, this->shape_ - start),
                       start,
                       this);
        }
    }
    
    ~ChunkedArrayHDF5()
    {
        // make sure that chunks are written to disk before the destructor of 
        // file_ is called
        outer_array_.swap(ChunkStorage());
    }
    
    void flushToDisk()
    {
        file_.flushToDisk();
    }
    
    virtual bool isReadOnly() const
    {
        return file_.isReadOnly();
    }
    
    virtual shape_type chunkArrayShape() const
    {
        return outer_array_.shape();
    }
    
    virtual pointer loadChunk(ChunkBase<N, T> * chunk)
    {
        return static_cast<Chunk *>(chunk)->read();
    }
    
    virtual void unloadChunk(ChunkBase<N, T> * chunk)
    {
        static_cast<Chunk *>(chunk)->write();
    }
    
    virtual Chunk * lookupChunk(shape_type const & index)
    {
        return &outer_array_[index];
    }
    
    virtual std::string backend() const
    {
        return "ChunkedArrayHDF5<'" + file_.filename() + "/" + dataset_name_ + "'>";
    }
    
    std::string const & datasetName() const
    {
        return dataset_name_;
    }
    
    HDF5File file_;
    std::string dataset_name_;
    HDF5HandleShared dataset_;
    ChunkStorage outer_array_;  // the array of chunks
};

} // namespace vigra

#undef VIGRA_ASSERT_INSIDE

#endif /* VIGRA_MULTI_ARRAY_CHUNKED_HDF5_HXX */