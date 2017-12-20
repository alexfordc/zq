#pragma once

#include "Interface_SvrCommon.h"

#ifndef SVRTRADESUPPORT_API
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SVRTRADESUPPORT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TESTTCPSVR_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SVRTRADESUPPORT_EXPORTS
#define SVRTRADESUPPORT_API __declspec(dllexport)
#else
#define SVRTRADESUPPORT_API __declspec(dllimport)
#endif
#endif


class SVRTRADESUPPORT_API CInterface_SvrTradeSupport : public CInterface_SvrCommon
{
public:
    static CInterface_SvrTradeSupport& getObj(void);


	//---------------------------------------------------------------------------------
	//	���ݽӿ�
	//---------------------------------------------------------------------------------
	int getExampleValue(void);

private:
    CInterface_SvrTradeSupport();
    virtual ~ CInterface_SvrTradeSupport();
    static CInterface_SvrTradeSupport* m_pObj;
};







