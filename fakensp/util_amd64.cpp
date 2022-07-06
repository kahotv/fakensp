#include "stdafx.h"
#include "util_amd64.h"
extern "C" void* amd64_getReg(UtilAMD64RegID id)
{
	switch (id)
	{
	case UtilAMD64RegID::RAX:
		return _amd64_get_rax();
	case UtilAMD64RegID::RBX:
		return _amd64_get_rbx();
	case UtilAMD64RegID::RCX:
		return _amd64_get_rcx();
	case UtilAMD64RegID::RDX:
		return _amd64_get_rdx();
	case UtilAMD64RegID::RSI:
		return _amd64_get_rsi();
	case UtilAMD64RegID::RDI:
		return _amd64_get_rdi();
	case UtilAMD64RegID::RBP:
		return _amd64_get_rbp();
	case UtilAMD64RegID::RSP:
		return _amd64_get_rsp();
	case UtilAMD64RegID::R8:
		return _amd64_get_r8();
	case UtilAMD64RegID::R9:
		return _amd64_get_r9();
	case UtilAMD64RegID::R10:
		return _amd64_get_r10();
	case UtilAMD64RegID::R11:
		return _amd64_get_r11();
	case UtilAMD64RegID::R12:
		return _amd64_get_r12();
	case UtilAMD64RegID::R13:
		return _amd64_get_r13();
	case UtilAMD64RegID::R14:
		return _amd64_get_r14();
	case UtilAMD64RegID::R15:
		return _amd64_get_r15();
	default:
		return 0;
	}
}