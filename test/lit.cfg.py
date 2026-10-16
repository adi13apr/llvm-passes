# lit configuration: tells lit how to find and run the tests in this folder.
import os
import lit.formats

config.name = "llvm-passes"

# Run each test's "RUN:" lines as shell commands (bash)
config.test_format = lit.formats.ShTest(execute_external=False)
# Only .ll files are tests; skip the IR generated from example.c
config.suffixes = [".ll"]
config.excludes = ["example.ll"]

config.test_source_root = os.path.dirname(os.path.abspath(__file__))

# Put LLVM 18's tools (opt, FileCheck) first on PATH
config.environment["PATH"] = "/usr/lib/llvm-18/bin:" + os.environ["PATH"]

# %plugin in a RUN line is replaced with the path to our built pass
config.substitutions.append(
    ("%plugin", os.path.join(config.test_source_root, "..", "build", "libInstCounter.so"))
)
config.substitutions.append(
    ("%dceplugin", os.path.join(config.test_source_root, "..", "build", "libMyDCE.so"))
)
config.substitutions.append(
    ("%unrollplugin", os.path.join(config.test_source_root, "..", "build", "libUnrollAdvisor.so"))
)