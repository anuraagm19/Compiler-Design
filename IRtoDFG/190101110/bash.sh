clang -S -emit-llvm -fno-discard-value-names lbm.c -o lbm.ll
~/LLVM/llvm-project-llvmorg-12.0.0/build/bin/opt -enable-new-pm=0 -disable-output -load LLVMAssign2Pass.so lbm.ll --assign2pass
dot -Kfdp -n -Tpng graph.dot -o "graph.png"
