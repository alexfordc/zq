#pragma once

#include "TcpClientWorker.h"
#include "CommonPkg.h"
#include "MemoryCache.h"
enum 
{	
	enmNetNothing, 
	enmNetDisconnect, 
	enmNetConnectWaiting, 
	enmNetConnectSuccess, 
	enmNetConnectFail, 
	enmNetDisconnectWaiting 
};
class CTCPDataService
{
public:


	enum REQ_STEP
	{
		REQ_STEP_INIT,
		REQ_STEP_REQ_SERVER_ADDR,
		REQ_STEP_SERVER_ADDR_RECVED,
		REQ_STEP_CONNECT_TO_SERVER,
		REQ_STEP_REQ_LICENCE_LIST,
		REQ_STEP_LICENCE_LIST_RECVED
	};

public:
	static CTCPDataService* GetInstance();
	static void DestroyInstance();

	static void	CallbackReceiveData(char *pData,int DataLen,Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData);	//从tcp得到数据后，调用此回调函数进行处理
	static void CallbackStartConnect(Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData);							//开始调用connect()后，调用此函数通知应用层
	static void	CallbackDisconnect(Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData);							//断开tcp连接后，调用此回调函数进行处理
	static void	CallbackConnectSuccess(Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData);						//连接成功后，调用此回调函数进行处理
	static void	CallbackConnectFail(Stru_IPAddress& LocalAddr,Stru_IPAddress& SvrAddr,SOCKET hSocket,int UserData);							//连接失败后，调用此回调函数进行处理
	static void	CallbackOntimeFunc(int UserData);				
	
	static VOID CALLBACK OnTimer1(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	static VOID CALLBACK OnTimer2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

public:
	int GetNetStatus() { return m_nNetStatus;};
	void SetNetStatus(int nNetStatus) { m_nNetStatus = nNetStatus;};

	bool Connect( std::string strIP, int nPort );
	bool SendData(UINT nCMDID, void* pBuff, int nLen, UINT seq = 0);
//	void OnTimer_HeartBeat(UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	bool LicenceRecved();
	bool GetLicence(std::string strAppID, std::string& strLicence);

private:
	CTCPDataService(void);
	virtual ~CTCPDataService(void);

	int GetPackageHeadTailLen();
	int GetPackageHeadLen();
	static bool VerifyRequest(Stru_UniPkgHead& head);

private:
	static CTCPDataService* m_pInstance;
	CTcpClientWorker*	m_pTcpClient;
	CMemoryCache*		m_pStreamCache;	
	int					m_nNetStatus;

	Stru_ServerAddr		m_addr;
	REQ_STEP			m_step;
	std::map<std::string, std::string> m_licenceMap;

public:	
	std::map<int, int> m_mapSeqSubID2Socket;
	CReadWriteLock				m_Mutex_RequestID;
	int InsertmapSeqSubID2Socket(int hSocket);
	bool GetmapSeqSubID2Socket(int nQuotServerSeqID, int& hSocket);
	void DeletemapSeqSubID2Socket(int nQuotServerSeqID);


	std::map<std::string, int> m_mapInstrumentID;
	CReadWriteLock				m_Mutex_mapInstrumentID;
	void AddQutoInstruments(std::map<std::string, int>& mapInstrumentID);
	void DelQutoInstruments(std::string strInstrumentID);
	void GetQutoInstruments(std::map<std::string, int>& mapInstrumentID);
};
