#ifndef _CPU_H
#define _CPU_H
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Lib/Types.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define AMD_CPUID_EBX	0x68747541
#define AMD_CPUID_ECX 	0x444d4163
#define AMD_CPUID_EDX 	0x69746e65

#define INTEL_CPUID_EBX	0x756e6547
#define INTEL_CPUID_ECX 0x6c65746e
#define INTEL_CPUID_EDX 0x49656e69

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	uint32 cpuid_eax;
	uint32 cpuid_ebx;
	uint32 cpuid_ecx;
	uint32 cpuid_edx;
} __attribute__ ((packed)) cpu_info_t;

struct __cpuid_feature_info {
	unsigned 			: 23;
	unsigned mmx  :  1;
	unsigned fxsr :  1;
	unsigned sse  :  1;
	unsigned sse2 :  1;
	unsigned      :  5;
} __attribute__ ((packed));

typedef union cpuid_feature_info {
	struct __cpuid_feature_info bits;
	uint32 word;
} cpuid_feature_info;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CPU_Info {
	char sVendor[32];
	uint32 uFamily;
	uint32 uModel;
	uint32 uStepping;
} CPUINFO, *LPCPUINFO;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCPU {
private:
	CPUINFO Info;

public:
	bool Initialize(void);
	void Destroy(void);

	void DisableFPU(void);
	void EnableFPU(void);
	uint32 HasID(void);
	void ID(uint32 uCmd, cpu_info_t *pInfo);
	bool Identify(void);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CCPU gCPU;
extern CCPU *CPU;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
