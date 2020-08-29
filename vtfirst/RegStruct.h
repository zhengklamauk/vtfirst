#ifndef __REGSTRUCT_H__
#define __REGSTRUCT_H__

typedef union {
	struct {
		unsigned int VME : 1;
		unsigned int PVI : 1;
		unsigned int TSD : 1;
		unsigned int DE : 1;
		unsigned int PSE : 1;
		unsigned int PAE : 1;
		unsigned int MCE : 1;
		unsigned int PGE : 1;			//bit 7
		unsigned int PCE : 1;
		unsigned int OSFXSR : 1;
		unsigned int PSXMMEXCPT : 1;
		unsigned int UNKONOWN_1 : 1;	//zero
		unsigned int UNKONOWN_2 : 1;	//zero
		unsigned int VMXE : 1;			//bit 13
		unsigned int Reserved : 18;
		//unsigned int Reserved_64 : 32;
	};
}_CR4;

typedef union {
	struct {
		unsigned int CF : 1;
		unsigned int Unknown_1 : 1;
		unsigned int PF : 1;
		unsigned int Unknown_2 : 1;
		unsigned int AF : 1;
		unsigned int Unknown_3 : 1;
		unsigned int ZF : 1;
		unsigned int SF : 1;			//bit 7
		unsigned int TF : 1;
		unsigned int IF : 1;
		unsigned int DF : 1;
		unsigned int OF : 1;
		unsigned int IOPL : 2;
		unsigned int NT : 1;
		unsigned int Unknown_4 : 1;		//bit 15
		unsigned int RF : 1;
		unsigned int VM : 1;
		unsigned int AC : 1;
		unsigned int VIF : 1;
		unsigned int VIP : 1;
		unsigned int ID : 1;			//bit 21
		unsigned int Reserved : 10;
		//unsigned Reserved_64 : 32;
	};
}_EFLAGS;

#endif // !__REGSTRUCT_H__
