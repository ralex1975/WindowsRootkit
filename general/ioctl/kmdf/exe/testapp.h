#ifndef TESTAPP_H
#define TESTAPP_H

#ifdef TESTAPP_C
#define EXTERN __declspec( dllexport )
#else
#define EXTERN __declspec( dllimport )
#endif

EXTERN void OpenDev(void);
EXTERN void CloseDev(void);
EXTERN VOID InitDev();
EXTERN VOID EnterU();
EXTERN void EnterL(unsigned long long);
EXTERN ULONG64 ReadFSBase();
EXTERN ULONG64 WriteFSBase(ULONG64);
EXTERN ULONG64 ReadGSBase();
EXTERN ULONG64 WriteGSBase(ULONG64);


#endif
