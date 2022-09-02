![example workflow](https://github.com/Nosenzor/TBB_ShowCase/actions/workflows/lukka_ci.yml/badge.svg)
# TBB :: The Threading Building Blocks

## Intro :
This is a small showcases to demo some of the TBB features.
TBB is a powerful multithreading library, this repo try to show good and some bad usages of multithreading.

## Keep in mind :
*Always measure and profile to understand what is your performance problem*

## Building examples

Tests are built through the cmake command through CMakeList.txt hierarchy of files and subdirectory so tests are built Every time you call the master CMakeList.txt. [cf How to build](../../../../README.md)

The building script has been modified to build the tbb test cases.

 [Tbb tests cpp source can be found here : src/cpp/test/tbb/tbb_test.cc ](src/cpp/test/tbb/tbb_test.cc) 

## Running tests

from the root/master directory you can launch :
### Launch all tests

```
bin/tbb_tests --log_level=all 
```
### Launch Demo by Demo tests :

1. **Demo 1 :  "Cartesian to Polar"**
This "test" will show how it's easy to use *parallel_for, parallel_for_each, parallel_reduce, parallel_invoke, parallel_sort*. 
It will also show that multithreading is not always the best solution to speed up thing, you need to take care about lock contentions, like mutexes.

* Run this *show case* : 

```
bin/tbb_tests --log_level=all --run_test=Tests_tbb/tbb_Cartesian_to_Polar
```

2. **Demo 2 : "TBB's Containers"**
TBB provide thread safe and lock-free containers (lock-free mean with low contention and implementation avoid using mutexes)
TBB's containers are : *concurrent_vector, concurrent_unordered_set, concurrent_unordered_map and cocurrent_queue*.
Note that there is no ordered map or set because it is not possible to write a lock-free ordered map and thus if you really need order you should use STL's one with mutex.

The demo show that stl containers are not thread safe, it also shows that in single threaded apps STL containers are faster than TBB's containers, but TBB's containers are faster than STL containers plus mutexes in multihreaded apps.

* Run this *show case* :

```
bin/tbb_tests --log_level=all --run_test=Tests_tbb/tbb_ContainersSTL
```

3. **Demo 3 : "TBB's containers and STL's algorithms_units_tests"**

This demo shows that TBB's containers are perfectly compatible with STL algorithms.

* Run this *show case* :

```
bin/tbb_tests --log_level=all --run_test=Tests_tbb/tbb_ContainersSTL
```

4. **Demo 4 : "TBB's allocator"**

When app heavily do dynamic allocations on [heap](https://www.learncpp.com/cpp-tutorial/the-stack-and-the-heap/), the memory allocator can spend a lot of time (sometimes most or you time is spent doint allocations) to find an adress in the memory to put your objects.

TBB provide a special allocator, (In my opinion it's based on a pool allocator) that can really improve speed of allocate. **Always test and profile on your OS and your computer what's really happen. OSes evolve regularly and improve the general purpose allocator, and the default allocator can be be faster or slower on the new OS release than the TBB allocator**

The demo show how to use the TBB allocator.

* Run this *show case* :

```
bin/tbb_tests --log_level=all --run_test=Tests_tbb/tbb_Allocator
```

5. **Demo 5 : "Handling exceptions"**

The demo show how to handle exception raised from a thread-worker :

* Run this *show case* :

```
bin/tbb_tests --log_level=all --run_test=Tests_tbb/tbb_Exception
```

5. **Demo 6 : "Using shared_ptr "**

The demo show that **STL *shared_ptr* are thread-safe ! BUT thread-safe pointers does not mean that the pointee will be thread-safe**

* Run this *show case* :

```
bin/tbb_tests --log_level=all --run_test=Tests_tbb/tbb_SharedPtrs
```


## Help :
* [TBB Help (intel)](https://www.threadingbuildingblocks.org/docs/help/index.htm)
