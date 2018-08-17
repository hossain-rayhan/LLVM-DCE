#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/InstIterator.h" // inst_range
#include "llvm/IR/Instructions.h" // TerminatorInst
using namespace llvm;

#include "MyDCE.h"
#include <set>
#include <vector>
#include <stdio.h>

char MyDCE::ID = 0;
static RegisterPass<MyDCE> X("myMagicDCE", "My DCE Pass", false, false);

#define MDEBUG


bool MyDCE::runOnFunction(Function &F) {

	SmallVector<Instruction*, 32> Worklist;	// use as a stack
	SmallPtrSet<Instruction*, 32> Marked;

#ifdef MDEBUG	
	puts("\t\033[36mFind Critical Instruction\033[0m");
#endif

unsigned int totalIns = 0, liveIns = 0;
for (BasicBlock &B : F) {
  for (Instruction &Inst : B) {
	//for (Instruction &Inst : inst_range(F)) {
		totalIns++;
		//if (Inst.mayHaveSideEffects() || Inst.isTerminator()) {
		if (Inst.mayThrow() || Inst.isTerminator()) {
			Worklist.push_back(&Inst);
			Marked.insert(&Inst);
			liveIns++;
#ifdef MDEBUG
			Inst.dump();
			if (Inst.isTerminator())
				puts("\033[1;31m\tTerminatorInst \033[0m\n");
			if (Inst.mayHaveSideEffects())
				puts("\033[1;31m\tmayHaveSideEffects \033[0m\n");
#endif
		}
	}
}
errs() << "Total: " << totalIns << " Live: " << liveIns << "\n";


#ifdef MDEBUG
	puts("\t\033[36mFind Dependency Instruction\033[0m");
#endif

	while (!Worklist.empty()) {
		Instruction &Inst = *Worklist.pop_back_val();
		for (auto op = Inst.op_begin(); op != Inst.op_end(); op++) {
			if (isa<Instruction>(op->get())) {
				Instruction &j = static_cast<Instruction&>(*(op->get()));
				if (Marked.count(&j) == 0) {
					Worklist.push_back(&j);
					Marked.insert(&j);

#ifdef MDEBUG
					j.dump();
#endif

				}
			}
		}
	}
	
	bool Changed = false;
	int EliminateCnt = 0;

#ifdef MDEBUG
	puts("Safe-Remove");
#endif

for (BasicBlock &B : F) {
  for (Instruction &Inst : B) {
	//for (Instruction &Inst : inst_range(F)) {
		if (Marked.count(&Inst) == 0) {
			Inst.dropAllReferences();
			Worklist.push_back(&Inst);
		}
	}
}
#ifdef MDEBUG
	puts("DEAD");
#endif
errs() << "Printing Dead instruction\n";
	for (auto Inst : Worklist) {

#ifdef MDEBUG
		Inst->dump();
#endif

		Inst->eraseFromParent();
		Changed = true;
		EliminateCnt++;
	}

errs() << "Total Dead: " << EliminateCnt << "\n";
#ifdef MDEBUG
	printf("\033[32m\t%s, #EliminateInst %d \033[0m\n", Changed ? "Changed" : "Unchanged", EliminateCnt);
#endif

	return Changed;
}
