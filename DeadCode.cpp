#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

namespace {
	struct DeadCode : public FunctionPass {
		static char ID;
		bool flag;

		DeadCode() : FunctionPass(ID) {}

		bool runOnFunction(Function &F) {
			flag = false;
			//errs() << "Inside Function: " << F.getName() << '\n';

			//if (skipFunction(F))
				//return false;
	
			SmallPtrSet<Instruction*, 32> Alive;
			SmallVector<Instruction*, 32> Worklist;
			unsigned int totalIns = 0, liveIns = 0;
			for (BasicBlock &B : F) {
  				for (Instruction &I : B) {
					
					// Collect the set of "root" instructions that are known live.
					totalIns++;
					errs() << "I- " << totalIns << ": " << I << " ";
					if (I.isTerminator() || I.mayHaveSideEffects() || isa<DbgInfoIntrinsic>(I) || isa<LandingPadInst>(I)) {
						errs() << " -Live";
						Alive.insert(&I);
						Worklist.push_back(&I);
						liveIns++;
					}
					errs() << "\n";
				}
				
			}
		
			errs() << "Total: " << totalIns << " Live: " << liveIns << "\n";
			//int size = Worklist.capacity();
			//errs() << "Printing from WorkList-" << size << " items :\n";
			//while (!Worklist.empty()) {
			//	Instruction *current = Worklist.pop_back_val();
			//	errs() << *current << "\n";
			//}

			//errs() << "Printing from AliveList:\n";
			//while (!Alive.empty()) {
				//Instruction *current = Alive.pop_back_val();
				//errs() << *current << "\n";
			//}
			// Pop the items and check for operand declaration instruction
			while (!Worklist.empty()) {
				Instruction *current = Worklist.pop_back_val();
				errs() << *current << "\n";
				unsigned int opCount = 0;
				for (Use &I : current->operands()) {
					errs() << "Opcount: " << ++opCount << "\n";
					if (Instruction *instruction = dyn_cast<Instruction>(I))
						if (Alive.insert(instruction).second)
							Worklist.push_back(instruction);
				}
			}
			
			errs() << "\nPrinting the dead instruction\n"; 
			unsigned int counter = 0;
			SmallVector<Instruction*, 32> Deadlist;
			for (BasicBlock &B : F) {
  				for (Instruction &I : B) {
					//errs() << ++counter << ": " << I;
					if (!Alive.count(&I)) {
					counter++;
						Deadlist.push_back(&I);
						I.dropAllReferences();
						errs() << I << " -dead";
					}
					//errs() << "\n";
				}
			}
			errs() << "\nCounter: " << counter <<"\n";
			
			/*for (Instruction *&I : Deadlist) {
				I->eraseFromParent();
			}*/

 		//return !Deadlist.empty();
		return false;
		}
	};
}
char DeadCode::ID = 0;
static RegisterPass<DeadCode> X("deadcode", "Dead Code Pass");

/*static void registerMyPass(const PassManagerBuilder &,
	legacy::PassManagerBase &PM) {
	PM.add(new DeadCode());
}
static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerMyPass);*/


