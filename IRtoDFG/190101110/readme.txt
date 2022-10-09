Anuraag Mahajan - 190101110
Assignment - 2

Instructions: 
1) Copy the Assign2Pass folder to $(LLVM_PATH)/llvm/lib/Transforms/

2) Add the following line to (LLVM_PATH)/llvm/lib/Transforms/CMakeLists.txt:  "add_subdirectory(Assign2Pass)"

3) Go the build directory and run: $make -j8

4) Run the bash script in this folder: $bash bash.sh

Output: 
0) Information about biggest block printed on shell
1) lbm.ll: IR code
2) ssa.txt: Single Static Assignment of the biggest basic block
3) graph.dot, graph.png: Data Flow Graph
