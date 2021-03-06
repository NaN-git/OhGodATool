#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "ohgodatool.h"

void PrintUsage(char *BinName)
{
	printf("OhGodATool %s\n", OHGODATOOL_VERSION);
	printf("Usage: %s [-i GPUIdx | -f VBIOSFile] [Generic Options] [sysfs options] [--core-state StateIdx] [--mem-state StateIdx] [--volt-state StateIdx] [State modification options]\n", BinName);
	printf("Generic modification options:\n");
	printf("\t--set-fanspeed <percent>\n\t--set-tdp <W>\n\t--set-tdc <W>\n\t--set-max-power <W>\n");
	printf("\t--set-max-core-clock <Mhz>\n\t--set-max-mem-clock <Mhz>\n");
	printf("sysfs options:\n");
	printf("\t--sysfs-device-path <path>\tSets the device path in sysfs (used only together with -i GPUIdx)\n");
	printf("State selection options (must be used before state modification options; -1 indicates last existing state):\n");
	printf("\t--core-state <index>\n\t--mem-state <index>\n\t--volt-state <index>\n");
	printf("State modification options:\n");
	printf("\t--mem-clock <Mhz>\n\t--core-clock <Mhz>\n\t--mem-vddc-idx <index>\n");
	printf("\t--core-vddc-idx <index>\n\t--mvdd <mV>\n\t--vddci <mV>\n\t--core-vddc-off <mV>\n");
	printf("\t--vddc-gfx-off <mV>\n\t--vddc-table-set <mV>\n");
	printf("Display options (shows the selected states, or if none selected, all states):\n");
	printf("\t--show-mem\n\t--show-core\n\t--show-voltage\n");
	printf("\t--show-fanspeed\n\t--show-temp\n");
}

#define NEXT_ARG_CHECK() do { if(i == (argc - 1)) { printf("Argument \"%s\" requires a parameter.\n", argv[i]); return(false); } } while(0)

bool ParseCmdLine(ArgsObj *Args, int argc, char **argv)
{
	memset(Args, 0x00, sizeof(ArgsObj));
	
	if(argc < 3)
	{
		PrintUsage(argv[0]);
		return(false);
	}
	
	for(int i = 1; i < argc; ++i)
	{
		if(!strcmp("-i", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->GPUIdx = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE || Args->GPUIdx > 15)
			{
				printf("Invalid GPU index specified.\n");
				return(false);
			}
			
			Args->GPUIdxProvided = true;
		}
		else if(!strcmp("-f", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->VBIOSFileName = strdup(argv[++i]);
			Args->VBIOSFileProvided = true;
		}
		else if (!strcmp("--sysfs-device-path", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->sysfsDevicePath = strdup(argv[++i]);
			Args->sysfsDevicePathProvided = true;
		}
		else if(!strcmp("--mem-state", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->MemStateIdx = strtol(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid memory state specified.\n");
				return(false);
			}
			
			Args->MemStateIdxProvided = true;
		}
		else if(!strcmp("--mem-clock", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqMemClk = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid memory clock specified.\n");
				return(false);
			}
			
			Args->SetMemClock = true;
		}
		else if(!strcmp("--core-state", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->CoreStateIdx = strtol(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid core state specified.\n");
				return(false);
			}
			
			Args->CoreStateIdxProvided = true;
		}
		else if(!strcmp("--core-clock", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqCoreClk = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid core clock specified.\n");
				return(false);
			}
			
			Args->SetCoreClock = true;
		}
		else if(!strcmp("--mem-vddc-idx", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqMemVoltTblIdx = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid voltage index specified.\n");
				return(false);
			}
			
			Args->SetMemVDDCIdx = true;
		}
		else if(!strcmp("--core-vddc-idx", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqCoreVoltTblIdx = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid voltage index specified.\n");
				return(false);
			}
			
			Args->SetCoreVDDCIdx = true;
		}
		else if(!strcmp("--mvdd", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqMVDD = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid voltage specified.\n");
				return(false);
			}
			
			Args->SetMVDD = true;
		}
		else if(!strcmp("--vddci", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqVDDCI = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid voltage specified.\n");
				return(false);
			}
			
			Args->SetVDDCI = true;
		}
		else if(!strcmp("--core-vddc-off", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqCoreVDDCOff = strtol(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid voltage offset specified.\n");
				return(false);
			}
			
			Args->SetCoreVDDCOff = true;
		}
		else if(!strcmp("--vddc-gfx-off", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqVDDCGFXOff = strtol(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid voltage offset specified.\n");
				return(false);
			}
			
			Args->SetVDDCGFXOff = true;
		}
		else if(!strcmp("--volt-state", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->VoltStateIdx = strtol(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid voltage state specified.\n");
				return(false);
			}
			
			Args->VoltageStateIdxProvided = true;
		}
		else if(!strcmp("--vddc-table-set", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqVoltTblVDDC = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid voltage specified.\n");
				return(false);
			}
			
			Args->SetVTblVDD = true;
		}
		else if(!strcmp("--show-mem", argv[i]))
		{
			Args->ShowMemStates = true;
		}
		else if(!strcmp("--show-core", argv[i]))
		{
			Args->ShowCoreStates = true;
		}
		else if(!strcmp("--show-voltage", argv[i]))
		{
			Args->ShowVoltageStates = true;
		}
		else if(!strcmp("--set-fanspeed", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqFanspeedPercentage = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE || Args->ReqFanspeedPercentage > 100)
			{
				printf("Invalid fanspeed specified (must be a percentage between 0 and 100.)\n");
				return(false);
			}
			
			Args->SetFanspeed = true;
		}
		else if(!strcmp("--show-fanspeed", argv[i]))
		{
			Args->ShowFanspeed = true;
		}
		else if(!strcmp("--show-temp", argv[i]))
		{
			Args->ShowTemp = true;
		}
		else if(!strcmp("--set-tdp", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqTDP = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid TDP specified.\n");
				return(false);
			}
			
			Args->SetTDP = true;
		}
		else if(!strcmp("--set-tdc", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqTDC = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid TDC specified.\n");
				return(false);
			}
			
			Args->SetTDC = true;
		}
		else if(!strcmp("--set-max-power", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqMaxPower = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid max power specified.\n");
				return(false);
			}
			
			Args->SetMaxPower = true;
		}
		else if(!strcmp("--set-max-core-clock", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqMaxCoreClk = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid max core clock specified.\n");
				return(false);
			}
			
			Args->SetMaxCoreClk = true;
		}
		else if(!strcmp("--set-max-mem-clock", argv[i]))
		{
			NEXT_ARG_CHECK();
			Args->ReqMaxMemClk = strtoul(argv[++i], NULL, 10);
			
			if(errno == EINVAL || errno == ERANGE)
			{
				printf("Invalid max memory clock specified.\n");
				return(false);
			}
			
			Args->SetMaxMemClk = true;
		}
		else
		{
			PrintUsage(argv[0]);
			printf("\nUnknown option: \"%s\"\n", argv[i]);
			return(false);
		}
	}
	
	return(true);
}
