IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
ENDIF()

if(APPLE AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0" AND NOT WITH_BOOST_THREAD)
    MESSAGE(FATAL_ERROR "cmake must be run with '-DWITH_BOOST_THREAD=1' in this toolset.")
endif()

if(NOT MSVC AND NOT (APPLE AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0") )
    IF (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0")
        SET(CMAKE_CXX_FLAGS "-pthread -std=c++0x ${CMAKE_CXX_FLAGS}")
    elseif(CMAKE_COMPILER_IS_GNUCXX)
        SET(CMAKE_CXX_FLAGS "-pthread -std=c++11 ${CMAKE_CXX_FLAGS}")
    else()
        SET(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    endif()
endif()

if(WITH_BOOST_THREAD)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    SET(VIGRANUMPY_CORE_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_CHRONO_LIBRARY})
endif()

# note special treatment of target vigranumpy_core: 
# module name is automatically changed into vigranumpycore
VIGRA_ADD_NUMPY_MODULE(core 
  SOURCES 
    vigranumpycore.cxx
    converters.cxx
    axistags.cxx
    multi_array_chunked.cxx
  LIBRARIES   
    ${VIGRANUMPY_IMPEX_LIBRARIES} ${VIGRANUMPY_CORE_LIBRARIES}
  VIGRANUMPY)

VIGRA_ADD_NUMPY_MODULE(impex 
  SOURCES
    impex.cxx
  LIBRARIES   
    ${VIGRANUMPY_IMPEX_LIBRARIES}
  VIGRANUMPY)   
     
VIGRA_ADD_NUMPY_MODULE(sampling 
  SOURCES
    sampling.cxx
  VIGRANUMPY)
   
VIGRA_ADD_NUMPY_MODULE(filters SOURCES
    kernel.cxx
    convolution.cxx
    filters.cxx
    tensors.cxx
    morphology.cxx
  VIGRANUMPY)
   
VIGRA_ADD_NUMPY_MODULE(analysis SOURCES
    segmentation.cxx
    edgedetection.cxx
    interestpoints.cxx
    accumulator.cxx
    accumulator-region-singleband.cxx
    accumulator-region-multiband.cxx
  VIGRANUMPY)
   
VIGRA_ADD_NUMPY_MODULE(learning SOURCES
    random_forest_old.cxx
    random_forest.cxx
    learning.cxx
  LIBRARIES   
    ${VIGRANUMPY_IMPEX_LIBRARIES}
  VIGRANUMPY)
   
VIGRA_ADD_NUMPY_MODULE(colors SOURCES
    colors.cxx
  VIGRANUMPY)
   
VIGRA_ADD_NUMPY_MODULE(noise SOURCES
    noise.cxx
  VIGRANUMPY)
   
VIGRA_ADD_NUMPY_MODULE(geometry SOURCES
    geometry.cxx
  VIGRANUMPY)
   
VIGRA_ADD_NUMPY_MODULE(optimization SOURCES
    optimization.cxx
  VIGRANUMPY)
