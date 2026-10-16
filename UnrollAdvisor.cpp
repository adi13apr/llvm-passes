// UnrollAdvisor: an analysis pass that looks at every loop in a function and
// prints whether a simple heuristic would fully unroll it, partially unroll it,
// or leave it alone. It does not change the IR.

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

// Heuristic thresholds (see README for the reasoning).
constexpr unsigned FullUnrollMaxSize = 64;    // trip count x body size
constexpr unsigned PartialUnrollMaxBody = 16; // instructions in the loop body
constexpr unsigned PartialUnrollFactor = 4;

struct UnrollAdvisorPass : PassInfoMixin<UnrollAdvisorPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    // Ask LLVM's analysis manager for the loops and for loop math.
    LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
    ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

    // Visit every loop, outer loops before the loops nested inside them.
    for (Loop *L : LI.getLoopsInPreorder()) {
      // Body size: instructions in all blocks that belong to the loop.
      unsigned Size = 0;
      for (BasicBlock *BB : L->blocks())
        Size += BB->size();

      // Exact number of iterations if it's a known constant, otherwise 0.
      unsigned TripCount = SE.getSmallConstantTripCount(L);

      errs() << "loop %" << L->getHeader()->getName() << " in " << F.getName()
             << ": trip count ";
      if (TripCount == 0)
        errs() << "unknown";
      else
        errs() << TripCount;
      errs() << ", size " << Size << " -> ";

      // The heuristic.
      if (TripCount != 0 && TripCount * Size <= FullUnrollMaxSize)
        errs() << "full unroll\n";
      else if (Size <= PartialUnrollMaxBody)
        errs() << "partial unroll by " << PartialUnrollFactor << "\n";
      else
        errs() << "no unroll\n";
    }

    // We only read the IR, so every analysis is still valid.
    return PreservedAnalyses::all();
  }
};

} // namespace

// Plugin entry point: lets you run the pass with  opt -passes=unroll-advisor
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "UnrollAdvisor", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "unroll-advisor") {
                    FPM.addPass(UnrollAdvisorPass());
                    return true;
                  }
                  return false;
                });
          }};
}