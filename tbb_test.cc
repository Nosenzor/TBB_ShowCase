#include <algorithm>
#include <array>
#include <vector>
#include <cmath>
#include <random>
#include <memory>
#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <numeric>
//#include <execution>
#include "tbb/tbb.h"
#include "tbb/parallel_for_each.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_sort.h"
#include "tbb/parallel_reduce.h"
#include "tbb/blocked_range.h"
#include "tbb/concurrent_vector.h"
#include "tbb/scalable_allocator.h"
#include "tbb/task.h"
#include "tbb/task_scheduler_observer.h"
#include "tbb/task_group.h"
#include "tbb/task_arena.h"


#include "boost/test/unit_test.hpp"
BOOST_AUTO_TEST_SUITE(Tests_tbb)


BOOST_AUTO_TEST_CASE(tbb_Cartesian_to_Polar)
{
   const bool SavePolarBears = false;
   const bool ActivateCompose = true;

   const bool ActivateGenerator = true;

   constexpr int NbPoints = 100000000; // one hundred million 

   std::vector<std::array<double, 3>> CartPoints_Seq(NbPoints), CartPoints_Par(NbPoints);

   std::random_device rd;  //Will be used to obtain a seed for the random number engine
   std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
   std::uniform_real_distribution<double> dist(-10000., 10000.);
   auto fRandomPt = [&dist, &gen](std::array<double, 3>& CartPt)
   {
      CartPt[0] = dist(gen);
      CartPt[1] = dist(gen);
      CartPt[2] = dist(gen);
   };


   // Fill the vector of point sequentially
   if (ActivateGenerator)
   {
      auto Start = std::chrono::high_resolution_clock::now();
      std::for_each(begin(CartPoints_Seq), end(CartPoints_Seq), fRandomPt);
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("Points Generation [SEQ] " << time_span.count() << "s");
   }
   // Fill the vector of points parallel
   {
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::parallel_for_each(begin(CartPoints_Par), end(CartPoints_Par), fRandomPt);
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("Points Generation [PAR] " << time_span.count() << "s");
   }
   std::vector<std::array<double, 3>> CartPoints_Copy = CartPoints_Seq;
   // Fill the vector of points parallel using thread local storage
   {

      auto fRandomPtTL = [](std::array<double, 3>& CartPt)
      {
         thread_local std::random_device TLrd;  //Will be used to obtain a seed for the random number engine
         thread_local std::mt19937 TLgen(TLrd()); //Standard mersenne_twister_engine seeded with rd()
         thread_local std::uniform_real_distribution<double> TLdist(-10000., 10000.);
         CartPt[0] = TLdist(TLgen);
         CartPt[1] = TLdist(TLgen);
         CartPt[2] = TLdist(TLgen);
      };
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::parallel_for_each(begin(CartPoints_Par), end(CartPoints_Par), fRandomPtTL);
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("Points Generation [TLPAR] " << time_span.count() << "s");
   }

   std::vector<std::array<double, 3>> PolarPoints_Seq(NbPoints), PolarPoints_Par(NbPoints);

   auto fCart2Pol = [](const std::array<double, 3>& CartPt)->std::array<double, 3>
   {
      std::array<double, 3> ToReturn;
      ToReturn[0] = std::hypot(CartPt[0], CartPt[1], CartPt[2]);
      ToReturn[1] = std::acos(CartPt[0] / std::hypot(CartPt[0], CartPt[1])) * (CartPt[1] < 0 ? -1 : 1);
      ToReturn[2] = std::acos(CartPt[2] / ToReturn[0]);
      return ToReturn;
   };
   //PolarPoints_Seq[0]=fCart2Pol(CartPoints_Seq[0]);

   // Transform the vector of points sequentially
   {
      auto Start = std::chrono::high_resolution_clock::now();
      std::transform(begin(CartPoints_Seq), end(CartPoints_Seq), begin(PolarPoints_Seq), fCart2Pol);
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("Points Transfo [SEQ] " << time_span.count() << "s");
   }

   // Transform the vector of points parallel
   {
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::parallel_for(size_t(0), PolarPoints_Par.size(), [&fCart2Pol, &CartPoints_Par, &PolarPoints_Par](size_t i) {
         PolarPoints_Par[i] = fCart2Pol(CartPoints_Par[i]);
         });
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("Points Transfo [PAR] " << time_span.count() << "s");
   }


   auto fSortPolar = [](const std::array<double, 3>& PolrPt1, const std::array<double, 3>& PolrPt2) -> bool
   {
      return PolrPt1[0] < PolrPt2[0];
   };
   // Sort the vector of points by radius sequentially
   {
      auto Start = std::chrono::high_resolution_clock::now();
      std::sort(begin(PolarPoints_Seq), end(PolarPoints_Seq), fSortPolar);
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("Sort By Radius [SEQ] " << time_span.count() << "s");
   }

   // Sort the vector of points by radius Parallel
   {
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::parallel_sort(begin(PolarPoints_Par), end(PolarPoints_Par), fSortPolar);
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("Sort By Radius [PAR] " << time_span.count() << "s");
   }

   //computing sum of radius seq

   //computing sum of radius seq
   {
      auto fSumPolarR = [](double lsum, const std::array<double, 3>& PolrPt1) -> double
      {
         return PolrPt1[0] + lsum;
      };
      auto Start = std::chrono::high_resolution_clock::now();
	 [[maybe_unused]] double Sum = std::accumulate(begin(PolarPoints_Seq), end(PolarPoints_Seq), 0., fSumPolarR);
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("SUM RADIUS [SEQ] " << time_span.count() << "s");
   }

   {
      auto fSumPolarR = [](double lsum, const std::array<double, 3>& PolrPt1) -> double
      {
         return PolrPt1[0] + lsum;
      };


      struct Sum {
         double value;
         Sum() : value(0.) {}
		 Sum(Sum& s, tbb::split) { value = s.value; }
         void operator()(const tbb::blocked_range< std::vector<std::array<double, 3>>::iterator>& range) {
            auto temp = value;
            for (auto a = range.begin(); a != range.end(); ++a) {
               temp += (*a)[0];
            }
            value = temp;
         }
         void join(Sum& rhs) { value += rhs.value; }
      };

      Sum total;
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::parallel_reduce(tbb::blocked_range< std::vector<std::array<double, 3>>::iterator>(PolarPoints_Par.begin(), PolarPoints_Par.end(), 1000),
         total);
      //total.value;
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("SUM RADIUS [PAR] " << time_span.count() << "s");
   }


   //computing sum of radius seq
   if (!SavePolarBears)
   {
      //computing sum of radius seq
      auto fSumPolarR2 = [](double lsum, const std::array<double, 3>& PolrPt1) -> double
      {
         //return PolrPt1[0] + lsum;
         return std::log(std::exp(PolrPt1[0]) * std::exp(lsum));// This a very stupid way to mak a simple sum :-)
      };
      auto Start = std::chrono::high_resolution_clock::now();
	 [[maybe_unused]] double Sum = std::accumulate(begin(PolarPoints_Seq), end(PolarPoints_Seq), 0., fSumPolarR2);
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("SUM RADIUS KILLING POLAR BEAR :-( [SEQ] " << time_span.count() << "s");
   }

   if (!SavePolarBears)
   {
//      auto fSumPolarR2 = [](double lsum, const std::array<double, 3>& PolrPt1) -> double
//      {
//         //return PolrPt1[0] + lsum;
//         return std::log(std::exp(PolrPt1[0]) * std::exp(lsum));// This a very stupid way to mak a simple sum :-)
//      };


      struct Sum {
         double value;
         Sum() : value(0.) {}
         Sum(Sum& s, tbb::split) { value = s.value; }
         void operator()(const tbb::blocked_range< std::vector<std::array<double, 3>>::iterator>& range) {
            auto temp = value;
            for (auto a = range.begin(); a != range.end(); ++a) {
               temp = std::log(std::exp((*a)[0]) * std::exp(temp));
            }
            value = temp;
         }
         void join(Sum& rhs) { value += rhs.value; }
      };

      Sum total;
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::parallel_reduce(tbb::blocked_range< std::vector<std::array<double, 3>>::iterator>(PolarPoints_Par.begin(), PolarPoints_Par.end(), 1000),
         total);
      //total.value;
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("SUM RADIUS KILLING POLAR BEAR :-( [PAR] " << time_span.count() << "s");
   }

   if (ActivateCompose)
   {
      //Show composition and simple tasks
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::parallel_invoke(
         [&PolarPoints_Par, &CartPoints_Par, &fCart2Pol]() {  // First Call
            tbb::parallel_for(size_t(0), PolarPoints_Par.size(),
               [&fCart2Pol, &CartPoints_Par, &PolarPoints_Par](size_t i) {
                  PolarPoints_Par[i] = fCart2Pol(CartPoints_Par[i]);
               });
         },

         [&CartPoints_Copy]() { // Second Call 

            tbb::parallel_sort(begin(CartPoints_Copy), end(CartPoints_Copy));
         },

            []() {  // Third Call

            std::cout << "Hello : ";
            for (int i = 0;i < 1000;i++)
            {
               std::cout << i << " ";
            }
         }
         );

      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("\n DOING 3 THINGS AT THE SAME TIME :-) " << time_span.count() << "s");
   }

}


BOOST_AUTO_TEST_CASE(tbb_Containers)
{

   constexpr size_t NbToPush = 1e7; //one billion

   {
      auto Start = std::chrono::high_resolution_clock::now();
      std::vector<int> Test;
      for (size_t i = 0;i < NbToPush;i++)
         Test.push_back(i);
      if (Test.size() != NbToPush)
         BOOST_TEST_MESSAGE("Error Size has not be reached");
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("PUSH BACK [SEQ] " << time_span.count() << "s");
   }

   if (0) ///Activate this first
   {
      BOOST_TEST_MESSAGE("Start Pushback unsafe ...");
      auto Start = std::chrono::high_resolution_clock::now();
      std::vector<int> Test;
      tbb::parallel_for(size_t(0), NbToPush, [&Test](size_t i) { std::vector<int> Local; Test.push_back(i);} );
      if (Test.size() != NbToPush)
         BOOST_TEST_MESSAGE("Error Size has not be reached");
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("PUSH BACK NO MUTEX [PAR] " << time_span.count() << "s");
   }

   if (1)
   {
      std::mutex Mtx; //will be use to protect push_back
      auto Start = std::chrono::high_resolution_clock::now();
      std::vector<int> Test;
      tbb::parallel_for(size_t(0), NbToPush, [&Test, &Mtx](size_t i) {
         std::lock_guard<std::mutex> Lock(Mtx);
         Test.push_back(i);
         });
      if (Test.size() != NbToPush)
         BOOST_TEST_MESSAGE("Error Size has not be reached");
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("PUSH BACK PAR MUTEX [PAR] " << time_span.count() << "s");
   }

   if (1)
   {
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::concurrent_vector<size_t> Test;
      tbb::parallel_for(size_t(0), NbToPush, [&Test](size_t i) {Test.push_back(i);});
      if (Test.size() != NbToPush)
         BOOST_TEST_MESSAGE("Error Size has not be reached");
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("PUSH BACK TBBVEC [PAR] " << time_span.count() << "s");
   }

   {
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::concurrent_vector<int> Test;
      for (size_t i = 0;i < NbToPush;i++)
         Test.push_back(i);
      if (Test.size() != NbToPush)
         BOOST_TEST_MESSAGE("Error Size has not be reached");
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("PUSH BACK SEQ ON TBB [SEQ] " << time_span.count() << "s");
   }


}


BOOST_AUTO_TEST_CASE(tbb_ContainersSTL)
{

   tbb::concurrent_vector<int> Test(1e7);
   std::iota(Test.begin(), Test.end(), 0);


   {
      auto Start = std::chrono::high_resolution_clock::now();
	 [[maybe_unused]] auto Nb3Multiple = std::count_if(Test.begin(), Test.end(), [](int i)->bool { return i % 3 == 0; });
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("Multiple 3 [SEQ] " << time_span.count() << "s");
   }

   // {
   //    auto Start = std::chrono::high_resolution_clock::now();
   //    auto Nb3Multiple = std::count_if(std::execution::par, begin(Test), end(Test), [](int i)->bool { return i % 3 == 0; });
   //    std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
   //    BOOST_TEST_MESSAGE("Multiple 3 [PAR] " << time_span.count() << "s");
   // }

}

BOOST_AUTO_TEST_CASE(tbb_Allocator)
{
   constexpr size_t NbAllocs = 1e8;

   if (1)
   {
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::concurrent_vector<size_t> Test;
      tbb::parallel_for(size_t(0), NbAllocs, [&Test](size_t i) {Test.push_back(i);});
      if (Test.size() != NbAllocs)
         BOOST_TEST_MESSAGE("Error Size has not be reached");
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("PUSH BACK TBBVEC [PAR] " << time_span.count() << "s");
   }

   if (1)
   {
      auto Start = std::chrono::high_resolution_clock::now();
      tbb::concurrent_vector<size_t, tbb::scalable_allocator<size_t>> Test;
      tbb::parallel_for(size_t(0), NbAllocs, [&Test](size_t i) {Test.push_back(i);});
      if (Test.size() != NbAllocs)
         BOOST_TEST_MESSAGE("Error Size has not be reached");
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("PUSH BACK TBBVEC ALLOC [PAR] " << time_span.count() << "s");

   }

   if (1)
   {
      auto Start = std::chrono::high_resolution_clock::now();

      tbb::parallel_for(size_t(0), NbAllocs, [](size_t i) {
         char* dummyc = new char;
         int* dummyi = new int;
         delete dummyc;
         double* dummyd = new double;
         delete dummyd;
         delete dummyi;
         });
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("STD ALLOC TIME [PAR] " << time_span.count() << "s");

   }

   // Allocation
   if (1)
   {
      auto Start = std::chrono::high_resolution_clock::now();

      tbb::scalable_allocator<char> CharAllocator;
      tbb::scalable_allocator<int> IntAllocator;
      tbb::scalable_allocator<double> DoubleAllocator;
      tbb::parallel_for(size_t(0), NbAllocs, [&CharAllocator, &IntAllocator, &DoubleAllocator](size_t i)
         {

            char* dummyc = CharAllocator.allocate(sizeof(char));
            int* dummyi = IntAllocator.allocate(sizeof(int));;
            CharAllocator.deallocate(dummyc, sizeof(char));
            double* dummyd = DoubleAllocator.allocate(sizeof(double));
            DoubleAllocator.deallocate(dummyd, sizeof(double));
            IntAllocator.deallocate(dummyi, sizeof(int));
         });
      std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
      BOOST_TEST_MESSAGE("PPL ALLOC TIME [PAR] " << time_span.count() << "s");

   }



   // {
   //    auto Start = std::chrono::high_resolution_clock::now();
   //    auto Nb3Multiple = std::count_if(std::execution::par, begin(Test), end(Test), [](int i)->bool { return i % 3 == 0; });
   //    std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - Start);
   //    BOOST_TEST_MESSAGE("Multiple 3 [PAR] " << time_span.count() << "s");
   // }

}

BOOST_AUTO_TEST_CASE(tbb_Exception)
{

#if 0
   {

      struct Update {
         void operator()(const tbb::blocked_range<int>& r) const {
            for (int i = r.begin(); i != r.end(); ++i)
               if (i < Data.size()) {
                  ++Data[i];
               }
               else {
                  // Cancel related tasks.
                  if (current_context()->cancel_group_execution())
                     std::cout << "Index " << i << " caused cancellation\n";
                  return;
               }
         }
      };
      Data.resize(1000);
      tbb::parallel_for(tbb::blocked_range<int>(0, 2000), Update());


   }
#endif

   {
      bool Data[1000][1000];
      try {
         tbb::parallel_for(0, 1000, 1,
            [&Data](int i) {
               tbb::task_group_context root(tbb::task_group_context::isolated);
               tbb::parallel_for(0, 1000, 1,
                  [&i, &Data](int j) {
                     Data[i][j] = true;
                  },
                  root);
               throw "oops";
            });
      }
      catch (...) {
         std::cout << "An error has been caught\n";
      }

   }
}


BOOST_AUTO_TEST_CASE(tbb_SharedPtrs)
{

   {
      struct MYInt
      {
         MYInt():m_val(0){}
         size_t m_val;
      };
      std::shared_ptr<MYInt> ptr=std::make_shared<MYInt>();
      constexpr size_t UpperLimit=1e6;
      tbb::parallel_for(size_t(0), size_t(UpperLimit), [ptr](size_t i){ //please note the capture by copy
        ptr->m_val+=i; // ... killing some more polar bears sorry :-(
      });

      if( UpperLimit*(UpperLimit-1)/2 !=  ptr->m_val)
      BOOST_TEST_MESSAGE("shared_ptr SUM IS WRONG "<<  ptr->m_val);
      else
      {
         BOOST_TEST_MESSAGE("shared_ptr SUM IS OK ");
      } 
   }

#if 0
// THIS JUST TO SHOW THAT A UNIQUE PTR CAN NOT BE SHARED BETWEEN THREADS IT DOES NOT COMPILE
   {
      std::unique_ptr<int> ptr=std::make_unique<int>(0);
      constexpr size_t UpperLimit=1e6;
      tbb::parallel_for(size_t(0), size_t(UpperLimit), [ptr](size_t i){ //please note the capture by copy
        *ptr+=i; // ... killing some more polar bears sorry :-(
      });

      if( UpperLimit*(UpperLimit+1)/2 != *ptr)
      BOOST_TEST_MESSAGE("unique_ptr SUM IS WRONG ");
      else
      {
         BOOST_TEST_MESSAGE("unique_ptr SUM IS OK ");
      } 
   }
#endif
   
   {
      struct MYInt
      {
         MYInt():m_val(0){}
         std::atomic<size_t> m_val={0}; //now using atomic
      };
       std::shared_ptr<MYInt> ptr=std::make_shared<MYInt>();
      constexpr size_t UpperLimit=1e6;
      tbb::parallel_for(size_t(0), size_t(UpperLimit), [ptr](size_t i){ //please note the capture by copy
        ptr->m_val+=i; // ... killing some more polar bears sorry :-(
      });

      if( ((UpperLimit*(UpperLimit-1))/2) != ptr->m_val)
      BOOST_TEST_MESSAGE("shared_ptr+atomic SUM IS WRONG "<<ptr->m_val);
      else
      {
         BOOST_TEST_MESSAGE("shared_ptr+atomic SUM IS OK ");
      } 
   }

}



BOOST_AUTO_TEST_SUITE_END()



// No more polar bears should die now
