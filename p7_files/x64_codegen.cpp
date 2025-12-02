#include <ostream>
#include "3ac.hpp"

namespace leviathan{

void IRProgram::allocGlobals(){
	for (auto &entry : globals){
        SymOpd *opd = entry.second;
        // We already had a mapping of all our globals so we just need to use their name as lables
        opd->setLabel(opd->getName());
    }
}

void IRProgram::datagenX64(std::ostream& out){
	out << ".data\n";
	out << ".globl main\n";
	// FOr the strings
    int strIdx = 0;
    for (auto &entry : this->strings){
        LitOpd *opd = entry.first;
        std::string s = entry.second;

		//Before I got double quotes so It gave me errors so I needed to strip one set of quotes
		if (!s.empty() && s.front() == '"' && s.back() == '"' && s.size() >= 2){
            s = s.substr(1, s.size() - 2);
        }
        std::string label = ".L_str_" + std::to_string(strIdx++);
		opd->setLabel(label);
		out << label << ":\n";
		out << "    .asciz \"" << s << "\"\n";
	}

	//For the globals
	for (auto &entry : globals){
		//We use the symbol to skip over functions
		const SemSymbol *sym = entry.first;
		const DataType *dt = sym->getDataType();
		if (dt->asFn() != nullptr){
			continue;
		}
		SymOpd *opd = entry.second;
		const std::string &lab = opd->getLabel();
		out << lab << ":\n";
		//All our opd SHOULD BE of 8 if the professor implemented this same as we did, might need to double check that
		if (opd->getWidth() == 8){
			out << "    .quad 0\n";
		} else {
			out << "    .byte 0\n";  
		}
	}

	//Put this directive after you write out strings
	// so that everything is aligned to a quadword value
	// again
	out << ".align 8\n";

}

void IRProgram::toX64(std::ostream& out){
	allocGlobals();
	datagenX64(out);
	// Iterate over each procedure and codegen it
	out << ".text\n";
    for (auto proc : *procs){
        proc->toX64(out);
    }
}

void Procedure::allocLocals(){
	//Allocate space for locals
	// Iterate over each procedure and codegen it

	size_t offset = 0;

	// allocate locals
	for (auto &entry : locals){
		SymOpd *opd = entry.second;
		size_t w = opd->getWidth();
		offset += w;
		opd->setMemoryLoc("-" + std::to_string(offset) + "(%rbp)");
	}

	// allocate temps
	for (AuxOpd *tmp : temps){
		size_t w = tmp->getWidth();
		offset += w;
		tmp->setMemoryLoc("-" + std::to_string(offset) + "(%rbp)");
	}

	// allocate formals
	for (SymOpd *formal : formals){
		size_t w = formal->getWidth();
		offset += w;
		formal->setMemoryLoc("-" + std::to_string(offset) + "(%rbp)");
	}

	// allocate addrOpds (for arrays and such..do we need this?)
	for (AddrOpd *addr : addrOpds){
		size_t w = addr->getWidth();
		offset += w;
		addr->setMemoryLoc("-" + std::to_string(offset) + "(%rbp)");
	}

	// 16 byte align stack
	size_t slack = (16 - (offset % 16)) % 16;
	offset += slack;

	setFrameSize(offset);
}

void Procedure::toX64(std::ostream& out){
	//Allocate all locals
	allocLocals();

	enter->codegenLabels(out);
	enter->codegenX64(out);
	out << "#Fn body " << myName << "\n";
	for (auto quad : *bodyQuads){
		quad->codegenLabels(out);
		out << "#" << quad->toString() << "\n";
		quad->codegenX64(out);
	}
	if (myName.compare("<init>") == 0){
		out << "# do all global init at the main symbol\n";
		out << "# then call the user-defined main as fun_main\n";
		out << "callq fun_main";
	}
	out << "#Fn epilogue " << myName << "\n";
	leave->codegenLabels(out);
	leave->codegenX64(out);
}

void Quad::codegenLabels(std::ostream& out){
	if (labels.empty()){ return; }

	size_t numLabels = labels.size();
	size_t labelIdx = 0;
	for ( Label * label : labels){
		out << label->getName() << ": ";
		if (labelIdx != numLabels - 1){ out << "\n"; }
		labelIdx++;
	}
}

void BinOpQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void UnaryOpQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void AssignQuad::codegenX64(std::ostream& out){
	src->genLoadVal(out, A);
	dst->genStoreVal(out, A);
}

void ReadQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

//FIRST THING WE NEED TO GET WORKING!
void WriteQuad::codegenX64(std::ostream& out){
	if (BasicType::INT() == mySrcType){
        mySrc->genLoadVal(out, DI);
        out << "callq printInt\n";

    } else if (BasicType::BOOL() == mySrcType){
        mySrc->genLoadVal(out, DI);
        out << "callq printBool\n";

    } else if (BasicType::STRING() == mySrcType){
        mySrc->genLoadAddr(out, DI);
        out << "callq printString\n";

    } else {
        out << "# WriteQuad: unsupported type\n";
    }
}

void GotoQuad::codegenX64(std::ostream& out){
	out << "jmp " << tgt->getName() << "\n";
}

void IfzQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void NopQuad::codegenX64(std::ostream& out){
	out << "nop" << "\n";
}

void CallQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void EnterQuad::codegenX64(std::ostream& out){
	out << "pushq %rbp\n";
	out << "movq %rsp, %rbp\n";

	size_t sz = myProc->getFrameSize();
	if (sz > 0){
		out << "subq $" << sz << ", %rsp\n";
	}

}

void LeaveQuad::codegenX64(std::ostream& out){
	//NOT RIGHT, MOMENTARY TO GET BASIC PRINTING WORKING WITH LITERALS/GLOBALS
	 out << "leave\n";
	 out << "ret\n";
}

void SetArgQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void GetArgQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void SetRetQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void GetRetQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void LocQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void SymOpd::genLoadVal(std::ostream& out, Register reg){
	//Globals are the only ones with labels
	if (!label.empty()){
        out << getMovOp() << " " << label << "(%rip), " << getReg(reg) << "\n";
    } else {
		out << getMovOp() << " " << getMemoryLoc() << ", " << getReg(reg) << "\n";
	}
}

void SymOpd::genStoreVal(std::ostream& out, Register reg){
	//Globals are the only ones with labels
	if (!label.empty()){
        out << getMovOp() << " " << getReg(reg) << ", " << label << "(%rip)\n";
    } else {
		out << getMovOp() << " " << getReg(reg) << ", " << getMemoryLoc() << "\n";
	}
}

void SymOpd::genLoadAddr(std::ostream& out, Register reg) {
	//Globals are the only ones with labels
	if (!label.empty()){
    out << "leaq " << label << "(%rip), " << getReg(reg) << "\n";
	}
    
}

void AuxOpd::genLoadVal(std::ostream& out, Register reg){
	TODO(Implement me)
}

void AuxOpd::genStoreVal(std::ostream& out, Register reg){
	TODO(Implement me)
}
void AuxOpd::genLoadAddr(std::ostream& out, Register reg){
	TODO(Implement me)
}


void AddrOpd::genStoreVal(std::ostream& out, Register reg){
	TODO(Implement me)
}

void AddrOpd::genLoadVal(std::ostream& out, Register reg){
	TODO(Implement me)
}

void AddrOpd::genStoreAddr(std::ostream& out, Register reg){
	TODO(Implement me)
}

void AddrOpd::genLoadAddr(std::ostream & out, Register reg){
	TODO(Implement me)
}

void LitOpd::genLoadVal(std::ostream & out, Register reg){
	out << getMovOp() << " $" << val << ", " << getReg(reg) << "\n";
}


}
