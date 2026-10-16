// MyDCE: a simple dead code elimination pass.
// Removes instructions whose result is never used and that have no side
// effects.

#include "llvm/ADT/SmallVector.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {

struct MyDCEPass : PassInfoMixin<MyDCEPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Changed = false;
    SmallVector<Instruction *, 16> Dead;

    // Repeat until a full round finds nothing to delete (a fixed point).
    // Erasing an instruction can make the instructions it used dead too.
    do {
      Dead.clear();

      // Collect instructions whose result is unused and that have no side
      // effects.
      for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
          if (I.use_empty() && !I.isTerminator() && !I.mayHaveSideEffects())
            Dead.push_back(&I);
        }
      }

      // Erase after iterating, so we never invalidate the loop's iterator.
      for (Instruction *I : Dead)
        I->eraseFromParent();

      if (!Dead.empty())
        Changed = true;
    } while (!Dead.empty());

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

} // namespace

// Plugin entry point: lets you run the pass with  opt -passes=my-dce
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MyDCE", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "my-dce") {
                    FPM.addPass(MyDCEPass());
                    return true;
                  }
                  return false;
                });
          }};
}