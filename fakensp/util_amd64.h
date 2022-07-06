#pragma once

extern "C" void* __stdcall _amd64_get_rax();
extern "C" void* __stdcall _amd64_get_rbx();
extern "C" void* __stdcall _amd64_get_rcx();
extern "C" void* __stdcall _amd64_get_rdx();
extern "C" void* __stdcall _amd64_get_rsi();
extern "C" void* __stdcall _amd64_get_rdi();
extern "C" void* __stdcall _amd64_get_rbp();
extern "C" void* __stdcall _amd64_get_rsp();
extern "C" void* __stdcall _amd64_get_r8();
extern "C" void* __stdcall _amd64_get_r9();
extern "C" void* __stdcall _amd64_get_r10();
extern "C" void* __stdcall _amd64_get_r11();
extern "C" void* __stdcall _amd64_get_r12();
extern "C" void* __stdcall _amd64_get_r13();
extern "C" void* __stdcall _amd64_get_r14();
extern "C" void* __stdcall _amd64_get_r15();

enum class UtilAMD64RegID
{
	Min = 0,
	RAX = 0,
	RBX,
	RCX,
	RDX,
	RSI,
	RDI,
	RBP,
	RSP,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	Max = R15
};

extern "C" void* amd64_getReg(UtilAMD64RegID id);