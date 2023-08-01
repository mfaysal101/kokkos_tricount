#include <Kokkos_Core.hpp>
#include <cstdio>
#include "csrmat.h"


int main(int argc, char* argv[]) 
{
    Kokkos::initialize(argc, argv);

    CSRMat A(argv[1], true);

    printf("point 1\n");

    #ifdef KOKKOS_ENABLE_CUDA
    #define MemSpace Kokkos::CudaSpace
    #endif
    #ifdef KOKKOS_ENABLE_HIP
    #define MemSpace Kokkos::Experimental::HIPSpace
    #endif
    #ifdef KOKKOS_ENABLE_OPENMPTARGET
    #define MemSpace Kokkos::OpenMPTargetSpace
    #endif

    #ifndef MemSpace
    #define MemSpace Kokkos::HostSpace
    #endif

    using ExecSpace = MemSpace::execution_space;

    using range_policy = Kokkos::RangePolicy< Kokkos::Schedule<Kokkos::Dynamic>, ExecSpace> ;

    typedef Kokkos::View<int*, MemSpace> VectorType_View;

    int rowsize = A.rowptr.size();
    int colsize = A.colids.size();

    VectorType_View IA("Row Index IA", rowsize);
    VectorType_View JA("Column indices JA", colsize);

    for(int i = 0; i < A.rowptr.size(); i++)
    {
		IA(i) = A.rowptr[i];
    }

    for(int j = 0; j < A.colids.size(); j++)
    {
		JA(j) = A.colids[j];
    }

    double tricount_timer;

    size_t numTriC = 0;

    Kokkos::Timer timer;

    Kokkos::parallel_reduce("tricount linear algebra", range_policy(0, rowsize - 1), KOKKOS_LAMBDA(int m, size_t& triC)
    {
		VectorType_View spa("spa view", A.numcols);
		int startRowA = IA(m);
		int endRowA = IA(m+1);
		for(int k = startRowA; k < endRowA; k++)
		{
			int startRowB = IA(JA(k));
			int endRowB = IA(JA(k) + 1);
			for(int n = startRowB; n < endRowB; n++)
			{
				spa(JA(n)) += 1; 
			}
		}

		for(int cid = startRowA; cid < endRowA; cid++)
		{
			triC += spa(JA(cid));
		}
    }, numTriC);

    tricount_timer = timer.seconds();

    cout << "Triangle Count: "<< numTriC << std::endl;
    cout << "Tricount Time: "<< tricount_timer << std::endl;
	
	Kokkos::finalize();

  return 0;
}