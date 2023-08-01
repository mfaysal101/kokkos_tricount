To build linear-algebra-based triangle counting for CPU using the Makefile provided

1. Put the file "tricount_with_view.cpp" and "csrmat.h" in the same directory
2. Issue the 'make' command assuming "KOKKOS_PATH" is set to the directory where kokkos is installed

To run on CPU
1. Issue the command './generated_executable.host   input_graph.mtx'

######################################################################

To build linear-algebra-based triangle counting for GPU

1. Put the file "tricount_with_gpu.cpp" and "csrmat.h" in the same directory, comment out the main function in "tricount_with_view.cpp" if in the same directory
2. Issue the command 'make -j KOKKOS_DEVICES=Cuda'

To run on GPU

1. Issue the command './generated_executable.cuda   input_graph.mtx' 
