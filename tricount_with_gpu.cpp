#include <Kokkos_Core.hpp>
#include <cstdio>
#include "csrmat.h"

using view_type = Kokkos::View<double * [3]>;

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

    using range_policy = Kokkos::RangePolicy<ExecSpace>;

    typedef Kokkos::View<int*, Kokkos::LayoutLeft, MemSpace> VectorType_View;

    int rowsize = A.rowptr.size();
    int colsize = A.colids.size();

    VectorType_View IA("Row Index IA", rowsize);
    VectorType_View JA("Column indices JA", colsize);


    // Create host mirrors of device views.
    VectorType_View::HostMirror h_IA = Kokkos::create_mirror_view(IA);
    VectorType_View::HostMirror h_JA = Kokkos::create_mirror_view(JA);

    for(int i = 0; i < A.rowptr.size(); i++)
    {
		h_IA(i) = A.rowptr[i];
    }

    for(int j = 0; j < A.colids.size(); j++)
    {
		h_JA(j) = A.colids[j];
    }

    
    
    // Deep copy host views to device views.
    Kokkos::deep_copy(IA, h_IA);
    Kokkos::deep_copy(JA, h_JA);

    for(int i = 0; i < A.rowptr.size(); i++)
    {
		cout<<h_IA(i)<<" ";
    }
    cout<<endl;

    for(int j = 0; j < A.colids.size(); j++)
    {
		cout<<h_JA(j)<<" ";
    }
    cout<<endl;

    double tricount_timer;

    int numTriC = 0;

    Kokkos::Timer timer;

    Kokkos::parallel_reduce("tricount linear algebra", range_policy(0, rowsize-1), KOKKOS_LAMBDA(int m, int& triC)
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
