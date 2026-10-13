// InstCounter: a function pass that prints how many instructions
// each function contains. Built as a plugin for LLVM's new pass manager.

#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct InstCounterPass : PassInfoMixin<InstCounterPass> {
  // Called once for every function in the module.
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    // Declarations (e.g. printf) have no body, so skip them.
    if (F.isDeclaration())
      return PreservedAnalyses::all();

    unsigned Count = 0;
    for (BasicBlock &BB : F)
      Count += BB.size(); // number of instructions in this block

    errs() << F.getName() << ": " << Count << " instructions\n";

    // We only read the IR, so every analysis is still valid.
    return PreservedAnalyses::all();
  }
};

} // namespace

// Entry point that `opt` looks for when it loads the plugin.
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "InstCounter", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // Lets you run the pass with: opt -passes=inst-counter
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "inst-counter") {
                    FPM.addPass(InstCounterPass());
                    return true;
                  }
                  return false;
                });
          }};
}