/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include <fstream>
#include <getopt.h>
#include <iostream>

#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetSubtargetInfo.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "codegen.hpp"
#include "parser.hpp"

using namespace lila::codegen;
using namespace lila::parser;

int main(int argc, char** argv) {

  const char *input = "-";
  const char *output = "a.out.o";

  bool verbose = false;

  // ---------------------------------------------------------------------------
  // parse command line options
  // ---------------------------------------------------------------------------

  char usage [1024];
  snprintf(usage,
           1024,
           "%s\n"
           "\n"
           "usage: lilac [OPTIONS] INPUT\n"
           "\n"
           "options:\n"
           "    -o FILENAME      write output to FILENAME\n"
           "                     if omitted, writes to %s\n"
           "    -v               verbose output\n"
           "    INPUT            read source code from INPUT\n"
           "                     if omitted or %s, reads from STDIN\n"
           "\n",
           PACKAGE_STRING,
           output,
           input
           );

  int c;
  while ((c = getopt (argc, argv, "ho:v")) != -1)
    switch (c) {
    case 'h':
      cout << usage;
      return 0;
    case 'o':
      output = optarg;
      break;
    case 'v':
      verbose = true;
      break;
    default:
      cerr << usage;
      return 1;
    }

  if (optind < argc) {
    input = argv[optind];
  }

  // ---------------------------------------------------------------------------
  // read code and tokenize it
  // ---------------------------------------------------------------------------

  vector<unique_ptr<Token>> tokens;

  if (strcmp(input, "-") == 0) {
    tokenize(&cin, &tokens);
  } else {
    ifstream is(input);

    if (is) {
      tokenize(&is, &tokens);
      is.close();
    } else {
      cerr << "error opening file: " << input << endl;
      return 1;
    }
  }

  if (verbose)
    for (auto it = tokens.begin() ; it != tokens.end(); ++it) {
      Token * token = it->get();
      cerr << "[debug] [token] \"" << token->toString() << "\"" << endl;
    }

  // ---------------------------------------------------------------------------
  // parse the tokens to AST
  // ---------------------------------------------------------------------------

  Parser parser(&tokens);

  unique_ptr<ASTNode> ast;

  try {
    ast = parser.parse();
  } catch (const char * msg) {
    cerr << msg << endl;
    return 1;
  }

  if (verbose)
    cerr << "[debug] [ast] " << ast->toString() << endl;

  // ---------------------------------------------------------------------------
  // generate LLVM IR code
  // ---------------------------------------------------------------------------

  CodeGen codegen("lilamodule", llvm::getGlobalContext());

  codegen.generateCode(move(ast), true);

  // ---------------------------------------------------------------------------
  // write object file
  // ---------------------------------------------------------------------------

  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();

  llvm::PassRegistry *Registry = llvm::PassRegistry::getPassRegistry();
  llvm::initializeCore(*Registry);
  llvm::initializeCodeGen(*Registry);
  llvm::initializeLoopStrengthReducePass(*Registry);
  llvm::initializeLowerIntrinsicsPass(*Registry);
  llvm::initializeUnreachableBlockElimPass(*Registry);

  llvm::Triple TheTriple(llvm::sys::getDefaultTargetTriple());

  std::string Error;
  const llvm::Target *TheTarget =
    llvm::TargetRegistry::lookupTarget(TheTriple.getTriple(), Error);

  if (!TheTarget) {
    cerr << Error << endl;
    return 1;
  }

  // compile for native cpu
  std::string CPUStr = llvm::sys::getHostCPUName();

  // compile for native cpu features
  llvm::SubtargetFeatures Features;
  llvm::StringMap<bool> HostFeatures;
  if (llvm::sys::getHostCPUFeatures(HostFeatures))
    for (auto &F : HostFeatures)
      Features.AddFeature(F.first(), F.second);
  std::string FeaturesStr = Features.getString();

  llvm::CodeGenOpt::Level OLvl = llvm::CodeGenOpt::Default;

  llvm::TargetOptions Options;

  std::unique_ptr<llvm::TargetMachine> Target(
    TheTarget->createTargetMachine(TheTriple.getTriple(),
                                   CPUStr,
                                   FeaturesStr,
                                   Options,
                                   llvm::Reloc::Default,
                                   llvm::CodeModel::Default,
                                   OLvl));

  std::error_code EC;
  auto Out = llvm::make_unique<llvm::tool_output_file>(output, EC, llvm::sys::fs::F_None);

  llvm::legacy::PassManager PM;
  llvm::TargetLibraryInfoImpl TLII(TheTriple);
  PM.add(new llvm::TargetLibraryInfoWrapperPass(TLII));

  if (const llvm::DataLayout *DL = Target->getDataLayout())
    codegen.module->setDataLayout(*DL);

  // add features to functions of module
  for (auto &F : *codegen.module) {
    auto &Ctx = F.getContext();
    llvm::AttributeSet Attrs = F.getAttributes(), NewAttrs;

    if (!CPUStr.empty())
      NewAttrs = NewAttrs.addAttribute(Ctx, llvm::AttributeSet::FunctionIndex,
                                       "target-cpu", CPUStr);

    if (!FeaturesStr.empty())
      NewAttrs = NewAttrs.addAttribute(Ctx, llvm::AttributeSet::FunctionIndex,
                                       "target-features", FeaturesStr);

    NewAttrs = Attrs.addAttributes(Ctx, llvm::AttributeSet::FunctionIndex, NewAttrs);
    F.setAttributes(NewAttrs);
  }

  llvm::raw_pwrite_stream *OS = &Out->os();

  if (Target->addPassesToEmitFile(PM, *OS, llvm::TargetMachine::CGFT_ObjectFile)) {
    cerr << "error: target does not support generation of this file type" << endl;
    return 1;
  }

  PM.run(*codegen.module);

  Out->keep();

  // ---------------------------------------------------------------------------
  // end
  // ---------------------------------------------------------------------------

  return 0;
}
