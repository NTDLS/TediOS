////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CCPU.H"

#include "../Lib/stdlib.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCPU gCPU;
CCPU *CPU = (CCPU *)&gCPU;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCPU::Initialize(void)
{
	if(this->Identify())
	{
		printf("        (%s, (F/M/S:%u/%u/%u)\n",
			this->Info.sVendor, this->Info.uFamily,
			this->Info.uModel, this->Info.uStepping);
	}

	this->EnableFPU();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCPU::Destroy(void)
{
	this->DisableFPU();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCPU::DisableFPU(void)
{
	__asm__ volatile (
		"mov %%cr0,%%eax;"
		"or $8,%%eax;"
		"mov %%eax,%%cr0;"
		:
		:
		: "%eax"
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCPU::EnableFPU(void)
{
	__asm__ volatile (
		"mov %%cr0,%%eax;"
		"and $0xffFFffF7,%%eax;"
		"mov %%eax,%%cr0;"
		:
		:
		: "%eax"
	);	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32 CCPU::HasID(void)
{
	uint32 val, ret;
	
	asm volatile (
		"pushf\n"               /* read flags */
		"popl %0\n"
		"movl %0, %1\n"
		
		"btcl $21, %1\n"        /* swap the ID bit */
		
		"pushl %1\n"            /* propagate the change into flags */
		"popf\n"
		"pushf\n"
		"popl %1\n"
		
		"andl $(1 << 21), %0\n" /* interrested only in ID bit */
		"andl $(1 << 21), %1\n"
		"xorl %1, %0\n"
		: "=r" (ret), "=r" (val)
	);
	
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCPU::ID(uint32 uCmd, cpu_info_t *pInfo)
{
	asm volatile (
		"cpuid\n"
		: "=a" (pInfo->cpuid_eax), "=b" (pInfo->cpuid_ebx), "=c" (pInfo->cpuid_ecx), "=d" (pInfo->cpuid_edx)
		: "a" (uCmd)
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCPU::Identify(void)
{
	cpu_info_t info;

	if(this->HasID())
	{
		this->ID(0, &info);

		//Check for AMD processor.
		if((info.cpuid_ebx == AMD_CPUID_EBX)
		    && (info.cpuid_ecx == AMD_CPUID_ECX)
			&& (info.cpuid_edx == AMD_CPUID_EDX))
		{
			strcpy(this->Info.sVendor, "AMD");
		}
		//Check for Intel processor.
		else if((info.cpuid_ebx == INTEL_CPUID_EBX)
		    && (info.cpuid_ecx == INTEL_CPUID_ECX)
			&& (info.cpuid_edx == INTEL_CPUID_EDX))
		{
			strcpy(this->Info.sVendor, "Intel");
		}
		else{
			strcpy(this->Info.sVendor, "Unknown");
		}

		this->ID(1, &info);
		this->Info.uFamily = (info.cpuid_eax >> 8) & 0x0f;
		this->Info.uModel = (info.cpuid_eax >> 4) & 0x0f;
		this->Info.uStepping = (info.cpuid_eax >> 0) & 0x0f;

		return true;
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
