#include "StdAfx.h"
#include "TCPDataService.h"
#include "Tools_Ansi.h"
#include "OfferMainInterface.h"
#include "FileOpr.h "
using namespace std;
extern COfferMainInterface*		g_pOfferMain;
#define TIMER_SERVER_ADDR_CHECK	1
#define TIMER_HEARTBEAT			2
UINT_PTR g_lTimeQuto;
#if 0
#define LOG_INFO(fmt, ...)  ;
#else
#define LOG_INFO(fmt, ...)  CFileOpr::getObj().writelocallogDaily("log","PlatformCTPQuot", "[%s][%d]:"fmt, __FILE__, __LINE__, __VA_ARGS__);
#endif

CTCPDataService* CTCPDataService::m_pInstance = NULL;
int g_QuotServerSeqID = 0;
//定时检查licence服务器地址是否就绪，就绪时，发起连接
VOID CALLBACK CTCPDataService::OnTimer1(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if( m_pInstance == NULL)
		return;

	if ( m_pInstance->m_step == REQ_STEP_INIT && m_pInstance->m_nNetStatus == enmNetConnectSuccess )
	{
		//请求服务地址
		char szBuffer[256];
		strcpy(szBuffer, "LicenceVarify.dll");
		m_pInstance->SendData(CMDID_SERVER_ADDR_REQ, szBuffer, sizeof(szBuffer), 0);
		m_pInstance->m_step = REQ_STEP_REQ_SERVER_ADDR;
		return;
	}

	if ( m_pInstance->m_step == REQ_STEP_SERVER_ADDR_RECVED )
	{
		m_pInstance->Connect( CTools_Ansi::ConvertdwIP(m_pInstance->m_addr.TCPIP), m_pInstance->m_addr.TCPPort);
		m_pInstance->m_step = REQ_STEP_CONNECT_TO_SERVER;
		return;
	}

	if ( m_pInstance->m_step == REQ_STEP_CONNECT_TO_SERVER && m_pInstance->m_nNetStatus == enmNetConnectSuccess)
	{
		m_pInstance->SendData(CMDID_LICENCE_LIST_REQ, NULL, 0, 0);
		m_pInstance->m_step = REQ_STEP_REQ_LICENCE_LIST;
		return;
	}

	if ( m_pInstance->m_step == REQ_STEP_LICENCE_LIST_RECVED)
	{
		KillTimer(NULL, TIMER_SERVER_ADDR_CHECK);
	}
}
void CTCPDataService::CallbackOntimeFunc(int UserData)
{
	if( m_pInstance == NULL)
		return;

	if( m_pInstance->GetNetStatus() == enmNetConnectSuccess) 
	{
		m_pInstance->SendData(CMDID_HEARTBEAT, NULL, 0);
	}	
}/*
VOID CALLBACK CTCPDataService::OnTimer2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if( m_pInstance == NULL)
		return;

	m_pInstance->OnTimer_HeartBeat(uMsg, idEvent, dwTime);
}*/

//从tcp得到数据后，调用此回调函数进行处理
void CTCPDataService::CallbackReceiveData(char *pData,int DataLen,Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData)
{
	if( m_pInstance == NULL || NULL == m_pInstance->m_pStreamCache || NULL == pData )
		return;

	//Stru_UniPkgHead * lpHead = (Stru_UniPkgHead*)pData;
	//if(lpHead->len == 0 && lpHead->cmdid == Cmd_QuotEvent_Push)
	//{
	//	OFFER_INFO("A zero len package come");
	//	return;
	//}
	////严格判断
	//if(DataLen != sizeof(Stru_UniPkgHead) +lpHead->len + sizeof(TAIL) && lpHead->cmdid == Cmd_QuotEvent_Push )
	//{
	//	OFFER_INFO("DataLen not match %d ,headlen %d ",DataLen,lpHead->len);
	//	return;
	//}
	m_pInstance->m_pStreamCache->SaveDataTo(pData, DataLen);

	int nCacheLen = 0;
	while( (nCacheLen = m_pInstance->m_pStreamCache->GetLen()) >= sizeof(Stru_UniPkgHead) + sizeof(TAIL))
	{
		char szHeadBuf[sizeof(Stru_UniPkgHead)];
		memset(szHeadBuf, 0, sizeof(Stru_UniPkgHead));
		m_pInstance->m_pStreamCache->PeekFrom(szHeadBuf, sizeof(Stru_UniPkgHead));
		int nPackageHead = *(DWORD*)szHeadBuf;

		// 检查并抛弃包头错误的部分
		if( nPackageHead != HeadID_UniPkg)
		{
			int i = 0;
			for( ; i < sizeof(Stru_UniPkgHead) - sizeof(DWORD) + 1; i++)
			{
				if(*(DWORD*)(szHeadBuf + i) == HeadID_UniPkg )
				{
					break;
				}
			}

			m_pInstance->m_pStreamCache->DeleteData(i);
			continue;
		}

		Stru_UniPkgHead pkgHead;
		m_pInstance->m_pStreamCache->PeekFrom(&pkgHead, sizeof(Stru_UniPkgHead));
		//等待一个包收齐
		if ( nCacheLen < (int)(sizeof(Stru_UniPkgHead) + pkgHead.len + sizeof(TAIL)) )
		{
			return;
		}

		//检查、处理心跳包
		if( pkgHead.cmdid == CMDID_HEARTBEATRSP 
			&& pkgHead.len == 0 )
		{
			Stru_Heartbeat heartBeatPkg;
			memset(&heartBeatPkg, 0, sizeof(heartBeatPkg));
			m_pInstance->m_pStreamCache->GetFrom(&heartBeatPkg, sizeof(Stru_Heartbeat));

			TAIL tail;
			m_pInstance->m_pStreamCache->GetFrom(&tail, sizeof(tail));

			OFFER_INFO("CMDID_HEARTBEATRSP \n");
			continue;
		}


		// 将合法请求信息加入消息队列，待Work线程处理
		if ( VerifyRequest(pkgHead))
		{
			Stru_UniPkgHead head;
			m_pInstance->m_pStreamCache->GetFrom(&head, sizeof(Stru_UniPkgHead));
			//处理业务返回数据
		/*	if ( head.cmdid == CMDID_SERVER_ADDR_RSP)
			{
				//收到licence服务IP
				m_pInstance->m_step = REQ_STEP_SERVER_ADDR_RECVED;
				memset(&m_pInstance->m_addr, 0, sizeof(m_pInstance->m_addr));
				m_pInstance->m_pStreamCache->GetFrom(&m_pInstance->m_addr, sizeof(Stru_ServerAddr));
			}
			else if ( head.cmdid == CMDID_LICENCE_LIST_RSP )
			{
				m_pInstance->m_licenceMap.clear();
				m_pInstance->m_step = REQ_STEP_LICENCE_LIST_RECVED;
				for ( size_t i = 0; i < head.len / sizeof(Stru_Licence); i++)
				{
					Stru_Licence licence;
					memset(&licence, 0, sizeof(licence));
					m_pInstance->m_pStreamCache->GetFrom(&licence, sizeof(Stru_Licence));
					m_pInstance->m_licenceMap[licence.szAppID] = licence.szLicence;
				}
			}*/
			if( pkgHead.cmdid == Cmd_QuotEvent_Push && pkgHead.len ==sizeof(PlatformStru_DepthMarketData))
			{//处理行情

				PlatformStru_DepthMarketData depthData;
				ZeroMemory(&depthData, sizeof(depthData));
			//	memcpy(&depthData, pData+sizeof(Stru_UniPkgHead), sizeof(depthData));

				m_pInstance->m_pStreamCache->GetFrom(&depthData, sizeof(PlatformStru_DepthMarketData));

				 if ((depthData.SettlementPrice<0.00001 || depthData.SettlementPrice>10000) && depthData.SettlementPrice != util::GetDoubleInvalidValue())
				 {
					 OFFER_INFO("报盘接收到服务器行情XXXXXXXXXXX,Instrument===%s, LastPrice===%f, SettlementPrice===%f, OpenPrice===%f\n",
					depthData.InstrumentID, depthData.LastPrice,depthData.SettlementPrice,depthData.OpenPrice);
					
					 //PUSH_LOG_ERROR(FirstLevelError,true,true,"报盘接收到服务器行情XXXXXXXXXXX,Instrument===%s, LastPrice===%f, SettlementPrice===%f, OpenPrice===%f\n",
					//	 depthData.InstrumentID, depthData.LastPrice,depthData.SettlementPrice,depthData.OpenPrice);
				 }
				
				 

#ifdef SIMU//模拟交易 给发送成交和报单到交易数据管理
				std::vector<CThostFtdcOrderField> vecOrder;
				std::vector<CThostFtdcTradeField> vecTrader;
				CInterface_SvrSimulateTrade::getObj().Quot(depthData, vecOrder, vecTrader);

				for(int p = 0; p < (int)vecOrder.size(); p++)
				{
					CThostFtdcOrderField& order = vecOrder[p];
					std::string strBrokerID = order.BrokerID;
					std::string strAccountID = order.InvestorID;
					BrokerAccountKey BAKey;
					strcpy(BAKey.BrokerID, strBrokerID.c_str());
					strcpy(BAKey.AccountID, strAccountID.c_str());
					CCTPAccount* pCtpAccount = NULL;
					if(!g_pOfferMain->GetCTPAccount(BAKey,  pCtpAccount))
						continue;
					if(pCtpAccount)
					{
						pCtpAccount->ProcessOrder(order);						
					}					
				}

				for(int p = 0; p < (int)vecTrader.size(); p++)
				{
					CThostFtdcTradeField& trade = vecTrader[p];

					std::string strBrokerID = trade.BrokerID;
					std::string strAccountID = trade.InvestorID;
					BrokerAccountKey BAKey;
					strcpy(BAKey.BrokerID, strBrokerID.c_str());
					strcpy(BAKey.AccountID, strAccountID.c_str());
					CCTPAccount* pCtpAccount = NULL;
					if(!g_pOfferMain->GetCTPAccount(BAKey,  pCtpAccount))
						continue;
					if(pCtpAccount)
					{
						pCtpAccount->ProcessTrade(trade);						
					}			
				}
#endif

				CInterface_SvrTradeData::getObj().SetQuotInfo(depthData);//SetQuotInfo改变了里面的数据
			}
			else if( pkgHead.cmdid == Cmd_SubscribeQuotEvent_Rsp)
			{
				if(pkgHead.userdata1 == CF_ERROR_SUCCESS && pkgHead.len ==sizeof(CThostFtdcSpecificInstrumentField))
				{						
					CThostFtdcSpecificInstrumentField InstrumentField;
					ZeroMemory(&InstrumentField, sizeof(InstrumentField));
					m_pInstance->m_pStreamCache->GetFrom(&InstrumentField, sizeof(CThostFtdcSpecificInstrumentField));

					std::string strInstrument = InstrumentField.InstrumentID;
					m_pInstance->DelQutoInstruments(strInstrument);
				}					
			}
			else if( pkgHead.cmdid == Cmd_ReInitQuotServerEvent_Rsp)
			{			
				int hSocket = 0;
				bool bFind = m_pInstance->GetInstance()->GetmapSeqSubID2Socket(pkgHead.seq, hSocket);
				if(bFind)
				{
					if(pkgHead.userdata1 != CF_ERROR_SUCCESS)
					{						
						int nError = 0;
						m_pInstance->m_pStreamCache->GetFrom(&nError, sizeof(int));
						const char * lErrorString = FormatErrorCode(nError);

						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_ReInitQuotServerEvent_Rsp,
							(void*)lErrorString, strlen(lErrorString)+1,pkgHead.seq,0,nError);
					}
					else
					{
						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_ReInitQuotServerEvent_Rsp,
							NULL,0,pkgHead.seq,0,CF_ERROR_SUCCESS);
					}					
				}
			}
			else if( pkgHead.cmdid == Cmd_QuotServerTimeResetEvent_Rsp)
			{
				int hSocket = 0;
				bool bFind = m_pInstance->GetInstance()->GetmapSeqSubID2Socket(pkgHead.seq, hSocket);
				if(bFind)
				{
					if(pkgHead.userdata1 != CF_ERROR_SUCCESS)
					{						
						int nError = 0;
						m_pInstance->m_pStreamCache->GetFrom(&nError, sizeof(int));
						const char * lErrorString = FormatErrorCode(nError);

						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_QuotServerTimeResetEvent_Rsp,
							(void*)lErrorString, strlen(lErrorString)+1,pkgHead.seq,0,nError);
					}
					else
					{
						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_QuotServerTimeResetEvent_Rsp,
							NULL,0,pkgHead.seq,0,CF_ERROR_SUCCESS);
					}				
				}
			}
			else if(pkgHead.cmdid == Cmd_QryQuotServerTimeEvent_Rsp)
			{
				int hSocket = 0;
				bool bFind = m_pInstance->GetInstance()->GetmapSeqSubID2Socket(pkgHead.seq, hSocket);
				if(bFind)
				{
					if(pkgHead.userdata1 != CF_ERROR_SUCCESS)
					{						
						int nError = 0;
						m_pInstance->m_pStreamCache->GetFrom(&nError, sizeof(int));
						const char * lErrorString = FormatErrorCode(nError);

						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_QryQuotServerTimeEvent_Rsp,
							(void*)lErrorString, strlen(lErrorString)+1,pkgHead.seq,0,nError);
					}
					else
					{
						sReinitTime depthData;
						ZeroMemory(&depthData, sizeof(depthData));
						m_pInstance->m_pStreamCache->GetFrom(&depthData, sizeof(sReinitTime));
						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_QryQuotServerTimeEvent_Rsp,
							&depthData,sizeof(sReinitTime),pkgHead.seq,0,CF_ERROR_SUCCESS);
					}					
				}

			}
			else if(pkgHead.cmdid == Cmd_QrySimulateDay_Rsp)
			{
				int hSocket = 0;
				bool bFind = m_pInstance->GetInstance()->GetmapSeqSubID2Socket(pkgHead.seq, hSocket);
				if(bFind)
				{
					if(pkgHead.userdata1 != CF_ERROR_SUCCESS)
					{						
						int nError = 0;
						m_pInstance->m_pStreamCache->GetFrom(&nError, sizeof(int));
						const char * lErrorString = FormatErrorCode(nError);

						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_QrySimulateDay_Rsp,
							(void*)lErrorString, strlen(lErrorString)+1,pkgHead.seq,0,nError);
					}
					else
					{
						sSimulateDay depthData;
						ZeroMemory(&depthData, sizeof(depthData));
						m_pInstance->m_pStreamCache->GetFrom(&depthData, sizeof(sSimulateDay));
						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_QrySimulateDay_Rsp,
							&depthData,sizeof(sSimulateDay),pkgHead.seq,0,CF_ERROR_SUCCESS);
					}					
				}
			}
			else if(pkgHead.cmdid == Cmd_QrySimulatePlayMode_Rsp)
			{
				int hSocket = 0;
				bool bFind = m_pInstance->GetInstance()->GetmapSeqSubID2Socket(pkgHead.seq, hSocket);
				if(bFind)
				{
					if(pkgHead.userdata1 != CF_ERROR_SUCCESS)
					{						
						int nError = 0;
						m_pInstance->m_pStreamCache->GetFrom(&nError, sizeof(int));
						const char * lErrorString = FormatErrorCode(nError);

						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_QrySimulatePlayMode_Rsp,
							(void*)lErrorString, strlen(lErrorString)+1,pkgHead.seq,0,nError);
					}
					else
					{
						int depthData;
						ZeroMemory(&depthData, sizeof(depthData));
						m_pInstance->m_pStreamCache->GetFrom(&depthData, sizeof(int));
						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_QrySimulatePlayMode_Rsp,
							&depthData,sizeof(int),pkgHead.seq,0,CF_ERROR_SUCCESS);
					}					
				}
			}
			else if(pkgHead.cmdid == Cmd_SimulatePlay_Rsp)
			{
				int hSocket = 0;
				bool bFind = m_pInstance->GetInstance()->GetmapSeqSubID2Socket(pkgHead.seq, hSocket);
				if(bFind)
				{
					if(pkgHead.userdata1 != CF_ERROR_SUCCESS)
					{						
						int nError = 0;
						m_pInstance->m_pStreamCache->GetFrom(&nError, sizeof(int));
						const char * lErrorString = FormatErrorCode(nError);

						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_SimulatePlay_Rsp,
							(void*)lErrorString, strlen(lErrorString)+1,pkgHead.seq,0,nError);
					}
					else
					{
						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_SimulatePlay_Rsp,
							NULL,0,pkgHead.seq,0,CF_ERROR_SUCCESS);
					}					
				}
			}
			else if(pkgHead.cmdid == Cmd_SimulatePause_Rsp)
			{
				int hSocket = 0;
				bool bFind = m_pInstance->GetInstance()->GetmapSeqSubID2Socket(pkgHead.seq, hSocket);
				if(bFind)
				{
					if(pkgHead.userdata1 != CF_ERROR_SUCCESS)
					{						
						int nError = 0;
						m_pInstance->m_pStreamCache->GetFrom(&nError, sizeof(int));
						const char * lErrorString = FormatErrorCode(nError);

						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_SimulatePause_Rsp,
							(void*)lErrorString, strlen(lErrorString)+1,pkgHead.seq,0,nError);
					}
					else
					{
						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_SimulatePause_Rsp,
							NULL,0,pkgHead.seq,0,CF_ERROR_SUCCESS);
					}					
				}
			}
			else if(pkgHead.cmdid == Cmd_SimulateStop_Rsp)
			{
				int hSocket = 0;
				bool bFind = m_pInstance->GetInstance()->GetmapSeqSubID2Socket(pkgHead.seq, hSocket);
				if(bFind)
				{
					if(pkgHead.userdata1 != CF_ERROR_SUCCESS)
					{						
						int nError = 0;
						m_pInstance->m_pStreamCache->GetFrom(&nError, sizeof(int));
						const char * lErrorString = FormatErrorCode(nError);

						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_SimulateStop_Rsp,
							(void*)lErrorString, strlen(lErrorString)+1,pkgHead.seq,0,nError);
					}
					else
					{
						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_SimulateStop_Rsp,
							NULL,0,pkgHead.seq,0,CF_ERROR_SUCCESS);
					}					
				}
			}
			else if(pkgHead.cmdid == Cmd_SimulateApply_Rsp)
			{
				int hSocket = 0;
				bool bFind = m_pInstance->GetInstance()->GetmapSeqSubID2Socket(pkgHead.seq, hSocket);
				if(bFind)
				{
					if(pkgHead.userdata1 != CF_ERROR_SUCCESS)
					{						
						int nError = 0;
						m_pInstance->m_pStreamCache->GetFrom(&nError, sizeof(int));
						const char * lErrorString = FormatErrorCode(nError);

						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_SimulateApply_Rsp,
							(void*)lErrorString, strlen(lErrorString)+1,pkgHead.seq,0,nError);
					}
					else
					{
						CInterface_SvrTcp::getObj().SendPkgData(hSocket,Cmd_SimulateApply_Rsp,
							NULL,0,pkgHead.seq,0,CF_ERROR_SUCCESS);
					}					
				}
			}
			TAIL tail;
			m_pInstance->m_pStreamCache->GetFrom(&tail, sizeof(tail));
			continue;
		}

		
		//检查并抛弃错误包，如果是一个完整包，但在前面得不到处理，则认为是一个错误包，抛弃
		if( nCacheLen >= (int)(sizeof(Stru_UniPkgHead) + pkgHead.len + sizeof(TAIL)) )
		{
			m_pInstance->m_pStreamCache->DeleteData( sizeof(Stru_UniPkgHead) + pkgHead.len + sizeof(TAIL) );
			continue;
		}

		break;
	}
}

//开始调用connect()后，调用此函数通知应用层
void CTCPDataService::CallbackStartConnect(Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData)
{
	if( m_pInstance !=NULL ) 
	{
		m_pInstance->SetNetStatus(enmNetConnectWaiting);
	}
}

//断开tcp连接后，调用此回调函数进行处理
void CTCPDataService::CallbackDisconnect(Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData)
{
	if( m_pInstance !=NULL ) 
	{
		std::string strIP = CTools_Ansi::ConvertdwIP(SvrAddr.IP);
		OFFER_INFO("断开行情服务器：IP[%s], Port[%d]\n", strIP.c_str(),  SvrAddr.Port);
		PUSH_LOG_ERROR(FirstLevelError,true,true,"断开行情服务器：IP[%s], Port[%d]\n", strIP.c_str(),  SvrAddr.Port);
		m_pInstance->SetNetStatus(enmNetDisconnect);
	}
}

//连接成功后，调用此回调函数进行处理
void CTCPDataService::CallbackConnectSuccess(Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData)
{
	if( m_pInstance !=NULL ) 
	{
		std::string strIP = CTools_Ansi::ConvertdwIP(SvrAddr.IP);
		OFFER_INFO("连接行情服务器成功：IP[%s], Port[%d]\n", strIP.c_str(),  SvrAddr.Port);
		PUSH_LOG_ERROR(Logger,true,true,"连接行情服务器成功：IP[%s], Port[%d]\n", strIP.c_str(),  SvrAddr.Port);
		m_pInstance->SetNetStatus(enmNetConnectSuccess);
		m_pInstance->m_pStreamCache->Empty();//清空缓存区，里面有脏数据
		if(g_pOfferMain)
			g_pOfferMain->QueryQuot();
	}
}

//连接失败后，调用此回调函数进行处理
void CTCPDataService::CallbackConnectFail(Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData)
{
	if( m_pInstance !=NULL ) 
	{
		std::string strIP = CTools_Ansi::ConvertdwIP(SvrAddr.IP);
		OFFER_INFO("连接行情服务器失败：IP[%s], Port[%d]\n", strIP.c_str(),  SvrAddr.Port);
		PUSH_LOG_ERROR(FirstLevelError,true,true,"连接行情服务器失败：IP[%s], Port[%d]\n", strIP.c_str(),  SvrAddr.Port);
		m_pInstance->SetNetStatus(enmNetDisconnect);
	}
}

CTCPDataService* CTCPDataService::GetInstance()
{
	if( m_pInstance == NULL ) 
	{
		g_QuotServerSeqID = 0;
		m_pInstance = new CTCPDataService();
		g_lTimeQuto = SetTimer(NULL,NULL,300000,(TIMERPROC)(NULL)); //每5分钟订阅一次行情	
	    //g_lTimeQuto=	 SetTimer(NULL, TIMER_SERVER_ADDR_CHECK, 100, OnTimer1);
		// SetTimer(NULL, TIMER_SERVER_ADDR_CHECK, 100, OnTimer1);
	
	//	SetTimer(NULL, TIMER_HEARTBEAT, 10000, OnTimer2);
	}

	return m_pInstance;
}

void CTCPDataService::DestroyInstance()
{
	if( m_pInstance != NULL) 
	{
		KillTimer(NULL, TIMER_HEARTBEAT);
		Sleep(5);
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

CTCPDataService::CTCPDataService(void) 
: m_pTcpClient(NULL)
, m_pStreamCache(NULL)
, m_nNetStatus(enmNetNothing)
, m_step(REQ_STEP_INIT)
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_licenceMap.clear();
	m_pStreamCache = new CMemoryCache(5*1024*1024);
}

CTCPDataService::~CTCPDataService(void)
{
	if( m_pTcpClient != NULL ) 
	{
		delete m_pTcpClient;
		m_pTcpClient = NULL;
	}

	if ( NULL != m_pStreamCache )
	{
		delete m_pStreamCache;
		m_pStreamCache = NULL;
	}

	m_nNetStatus = enmNetNothing;
}

bool CTCPDataService::Connect(std::string strIP, int nPort)
{
	if( m_pTcpClient != NULL ) 
	{
		delete m_pTcpClient;
		m_pTcpClient = NULL;
	}

	if ( NULL != m_pStreamCache )
	{
		m_pStreamCache->Empty();
	}

	Stru_IPAddress tIPAddr, tServAddr;
	ZeroMemory(&tIPAddr, sizeof(tIPAddr));
	ZeroMemory(&tServAddr, sizeof(tServAddr));
	
	tServAddr.IP = CTools_Ansi::ConvertStrIP((char*)strIP.c_str());
	tServAddr.Port = nPort;
	m_pTcpClient = new CTcpClientWorker(tIPAddr, tServAddr, 
			CallbackReceiveData, CallbackDisconnect, CallbackStartConnect, 
			CallbackConnectSuccess, CallbackConnectFail, CallbackOntimeFunc);
	SetNetStatus(enmNetConnectWaiting);

	return true;
}

bool CTCPDataService::SendData(UINT nCMDID, void* pData, int nLen, UINT seq)
{
	if( m_pTcpClient != NULL && m_nNetStatus == enmNetConnectSuccess) 
	{
		if ( pData == NULL && nLen != 0)
		{
			return false;
		}

		Stru_UniPkgHead head;
		head.moduleid = 0;
		head.cmdid = nCMDID;
		head.seq = seq;
		head.len = nLen;

		int nTotalLen = sizeof(Stru_UniPkgHead)+head.len+sizeof(TAIL);
		char* pBuf = new char[nTotalLen];
		memset(pBuf, 0, sizeof(nTotalLen));
		auto_ptr<char> ptrBuf( pBuf );
		if ( NULL == pBuf )
		{
			return false;
		}

		memcpy(pBuf, &head, sizeof(Stru_UniPkgHead));
		memcpy(pBuf+sizeof(Stru_UniPkgHead), pData, nLen);
		TAIL tail = TailID_UniPkg;
		memcpy(pBuf+sizeof(Stru_UniPkgHead)+head.len, &tail, sizeof(TAIL));

		int nLen = m_pTcpClient->SendData(pBuf, nTotalLen);
		if(nLen >0 && pData!= NULL)
		{
		//	OFFER_INFO("报盘发送订阅行情命令： %s\n", pBuf+sizeof(Stru_UniPkgHead));
			//PUSH_LOG_ERROR(Logger,false,true,"发送订阅行情命令： %s\n", pBuf+sizeof(Stru_UniPkgHead));

		}
		return true;
	}
	else 
	{
		return false;
	}
}
/*
void CTCPDataService::OnTimer_HeartBeat(UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if( m_nNetStatus == enmNetConnectSuccess) 
	{
		SendData(CMDID_HEARTBEAT, NULL, 0);
	}
}
*/
int CTCPDataService::GetPackageHeadTailLen()
{
	return (int)sizeof(Stru_UniPkgHead)+(int)sizeof(DWORD);
}

int CTCPDataService::GetPackageHeadLen()
{
	return (int)sizeof(Stru_UniPkgHead);
}

bool CTCPDataService::VerifyRequest( Stru_UniPkgHead& head )
{
/*	switch(head.cmdid)
	{
	case CMDID_LICENCE_LIST_RSP:
		return head.len == sizeof(Stru_Licence);
	case CMDID_SERVER_ADDR_RSP:
		return head.len == sizeof(Stru_ServerAddr);
	case Cmd_QuotEvent_Push:
		return head.len == sizeof(PlatformStru_DepthMarketData);
    case Cmd_ReInitQuotServerEvent_Rsp:
        return true;
    case Cmd_QuotServerTimeResetEvent_Rsp:
        return true;
    case Cmd_QryQuotServerTimeEvent_Rsp:
		return true;  
	case Cmd_SubscribeQuotEvent_Rsp:
		return true;
	default:
		return false;
	}*/
	return true;
}

bool CTCPDataService::LicenceRecved()
{
	return m_step >= REQ_STEP_LICENCE_LIST_RECVED;
}

bool CTCPDataService::GetLicence( std::string strAppID, std::string& strLicence )
{
	std::map<std::string, std::string>::iterator it = m_licenceMap.find(strAppID);
	if ( it != m_licenceMap.end() )
	{
		strLicence = it->second;
		return true;
	}
	else
	{
		return false;
	}
}

int CTCPDataService::InsertmapSeqSubID2Socket(int hSocket)
{	
	g_QuotServerSeqID++;
	m_Mutex_RequestID.write_lock();
	m_mapSeqSubID2Socket.insert(make_pair(g_QuotServerSeqID, hSocket));
	m_Mutex_RequestID.write_unlock();

	return g_QuotServerSeqID;
}
void CTCPDataService::DeletemapSeqSubID2Socket(int nQuotServerSeqID)
{
	m_Mutex_RequestID.write_lock();
	m_mapSeqSubID2Socket.erase(nQuotServerSeqID);
	m_Mutex_RequestID.write_unlock();
}
bool CTCPDataService::GetmapSeqSubID2Socket(int nQuotServerSeqID, int& hSocket)
{
	bool bGet= false;
	m_Mutex_RequestID.write_lock();
	std::map<int, int>::iterator it = m_mapSeqSubID2Socket.find(nQuotServerSeqID);
	if(it != m_mapSeqSubID2Socket.end())
	{
		hSocket = it->second;
		bGet = true;
	}		
	m_Mutex_RequestID.write_unlock();

	return bGet;
}
void CTCPDataService::AddQutoInstruments(std::map<std::string, int>& mapInstrumentID)
{	
	m_Mutex_mapInstrumentID.write_lock();
	if(m_mapInstrumentID.size() == 0)	
	{
		m_mapInstrumentID = mapInstrumentID;
		//callback 函数里面settimer失败 
	//	g_lTimeQuto = SetTimer(NULL,NULL,300000,(TIMERPROC)(NULL)); //每5分钟订阅一次行情	
	   
	}
	m_Mutex_mapInstrumentID.write_unlock();
}
void CTCPDataService::DelQutoInstruments(std::string strInstrumentID)
{
	m_Mutex_mapInstrumentID.write_lock();	
	m_mapInstrumentID.erase(strInstrumentID);
	OFFER_INFO("订阅合约成功：[%s]\n", strInstrumentID.c_str());
	m_Mutex_mapInstrumentID.write_unlock();
}
void CTCPDataService::GetQutoInstruments(std::map<std::string, int>& mapInstrumentID)
{
	m_Mutex_mapInstrumentID.write_lock();	
	mapInstrumentID = m_mapInstrumentID;
	m_Mutex_mapInstrumentID.write_unlock();
}