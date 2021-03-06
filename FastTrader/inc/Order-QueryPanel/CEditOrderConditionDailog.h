#pragma once
#include "stdafx.h"
#include "CfgMgr/CfgGlobal.h"

#include "../inc/Order-Common/CLocalOrderService.h"
#include "../inc/Module-Orders/COrderConditionPanel.h"

#include "../inc/Module-Orders/COrderInputPanel.h"


enum
{
	ID_EDITCONDITION_OK=1850,
	ID_EDITCONDITION_CANCEL,
	EDITCONDITIONSUMNUM_ID	// 标志ID号的数量
};

class CEditOrderConditionDailog :public wxDialog
{
public:
	CEditOrderConditionDailog(wxWindow *parent, wxWindowID id,
             const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_DIALOG_STYLE,
             const wxString& name = wxDialogNameStr);
	~CEditOrderConditionDailog();

    virtual bool Show( bool show = true );

    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
	void OnCreateWindow(wxWindowCreateEvent& evt);
	void OnDestroyWindow(wxWindowDestroyEvent& evt);
	void OnDialogCharHook(wxKeyEvent& evt);
	DECLARE_EVENT_TABLE()

public:

	void SetConditionOrder(CLocalOrderService::CONDITIONORDER& tConditionOrder) {
		m_tConditionOrder = tConditionOrder;
	};
	CLocalOrderService::CONDITIONORDER& GetConditionOrder() {
		return m_tConditionOrder;
	};

	void SetPriceTick(double fPriceTick) {
		m_fPriceTick = fPriceTick;
	};
	
private:
	void ShowOrderReq();
	void ShowOrderCondition();

public:
	static wxWindow*	sm_poDailog;
	static HHOOK		sm_hDailogKeyBoard;

private:
	//CLocalOrderService*	m_poLocalOrderService;
	COrderInputPanel*	m_poOrderInputPanel;
	COrderConditionPanel*	m_poOrderConditionPanel;
    std::vector<unsigned int>	m_TabOrderVector;
	CLocalOrderService::CONDITIONORDER		m_tConditionOrder;
	//CLocalOrderService::CONDITIONORDER&		m_refConditionOrder;
	double	m_fPriceTick;

};