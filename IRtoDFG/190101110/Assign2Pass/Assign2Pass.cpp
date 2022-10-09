#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h" 
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "vector"
#include "algorithm"
#include "fstream"
#include "map"
#include "string"


using namespace std;
using namespace llvm;

// parses string to get " " or "," separated words
vector<string> wordsFromString(string &str){
	vector<string> words;
	words.push_back("");
	for(char x: str){
		if(x==',' || x==' '){
			if(words.back().size()) words.push_back("");
		}else{
			words.back().push_back(x);
		}
	}
	while(words.size() && !words.back().size()) words.pop_back();
	return words;
}

// converts Instructions structure to string
string insToStr(Instruction& I){
	string str;
	llvm::raw_string_ostream(str) << I;
	return str;
}


// extracts all the variables present in an instruction using "%"
vector<string> varsFromInstr(string& IStr){
	vector<string> words = wordsFromString(IStr);
	vector<string> vars = {};
	for(string x: words) if(x.size() && x[0]=='%') vars.push_back(x);
	return vars;
}


// gives variables which are written by the current instruction
string changedVar(string& IStr, string& opCodeName, vector<string> &vars){
	if(opCodeName == "store"){
		return vars.back();
	}else{
		for(char x: IStr) if(x=='=') return vars[0];
	}
	return "";
}


// gives variables on which the current instruction depends
vector<string> dependentVars(string& IStr, string& opCodeName, vector<string> &vars){
	if(opCodeName == "store"){
		return vars;
	}else{
		for(char x: IStr) {
			if(x=='=') {
				vector<string> newVars = vars;
				reverse(newVars.begin(), newVars.end());
				newVars.pop_back();
				return newVars;
			}
		}
	}
	return {};
}


BasicBlock* biggestBasicBlock;
int maxBlockSize = -1;


namespace {

	struct Assign2Pass: public ModulePass {

		static char ID;
		Assign2Pass() : ModulePass(ID) {}

		virtual bool runOnModule(Module &M){


			// identify the biggest block
			for(auto &F: M){
				for(auto &B: F){
					int curr = 0;
					for(auto &I: B) curr++;

					if(curr > maxBlockSize){
						biggestBasicBlock = &B;
						maxBlockSize = curr;
					}
				}
			}

			errs() << "Function to which biggest basic block belongs: " << biggestBasicBlock->getParent()->getName() << "\n";
			errs() << "Block name: " << biggestBasicBlock->getName() << "\n";
			errs() << "No. of instructions: " << maxBlockSize << "\n";

			for(auto &I: *biggestBasicBlock) {
				errs() << "First instruction: " << I << "\n";
				break;
			}


			map<string, int> prevDeclaration = {};
			vector<Instruction*> allInstructions = {};
			for(auto &I : *biggestBasicBlock) allInstructions.push_back(&I);
			int n = allInstructions.size();
			vector<vector<int>> adj(n, vector<int> (0));


			// create adjacency list
			int ind = 0;
			for(auto &I : *biggestBasicBlock){
				string IStr = insToStr(I);
				string opCodeName;
				llvm::raw_string_ostream(opCodeName) << I.getOpcodeName();

				vector<string> vars = varsFromInstr(IStr);
				for(string str: dependentVars(IStr,opCodeName,vars)){
					if(prevDeclaration.find(str)!=prevDeclaration.end()) 
						adj[ind].push_back(prevDeclaration[str]);
				}
				string var = changedVar(IStr,opCodeName,vars);
				if(var.length()) prevDeclaration[var] = ind;
				ind++;
			}

			// print ssa form 
			ofstream ssa;
			ssa.open("ssa.txt");

			int i = 0;
			for(auto &I: *biggestBasicBlock){
				ssa << "#"<< ++i << ": " << insToStr(I) << "\n";
			}


			//generate the .dot file from adjacency liust
			ofstream graph;
			graph.open("graph.dot");

			graph << "digraph DateFlowGraph{\n";

			for(i = 0; i<n; i++){
				for(auto u: adj[i]){
					graph << i << " -> " << u << ";\n";
					// graph << '"' << insToStr(*allInstructions[i]) << '"' << " -> " << '"' << insToStr(*allInstructions[u]) << '"' << "\n";
				}
			}

			graph << "}";

			errs() << "\n\n";
			errs() << "Refer to ssa.txt, graph.png (generated from graph.dot) for the outputs\n\n";

			return false;
		}
	};
}


char Assign2Pass::ID = 'x';

static RegisterPass<Assign2Pass> X("assign2pass", "Assign2 pass", false, false);


static RegisterStandardPasses Y(
    PassManagerBuilder::EP_EarlyAsPossible,
    [](const PassManagerBuilder &Builder,
       legacy::PassManagerBase &PM) { PM.add(new Assign2Pass()); });