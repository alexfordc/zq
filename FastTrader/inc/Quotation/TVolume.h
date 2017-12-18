#pragma once
#include "stdafx.h"
#include "wx/defs.h"

#include "wx/window.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif


#include <time.h>
#include "wx\msw\font.h"
#include "../inc/Quotation/TCtrl.h"
#include "../inc/Quotation/FData.h"



class   CTVolume : public CTCtrl
{
public:
	 CTVolume(void);
	 ~CTVolume(void);


private:
	wxColour			m_clBackGround; //����ɫ
	wxColour			m_clBorder;               //�߿�ɫ
	wxColour			m_cldashLine;
	wxColour			m_clPrice;
	wxColour			m_clAveragePrice;
	wxColour			m_clCurSelLine;
	wxColour			m_clTipsBk;

	wxColour			m_clName; 
	wxColour			m_clTime; //ʱ��
	wxColour			m_zhang;  //��
	wxColour			m_die;    //��
	wxColour			m_hengpan;//����
	wxColour			m_clLineName;//�ߵ�����

private:
	void		CalcMaxMin();
public:
	bool InitCfg(TiXmlElement *root);

public://configר��
	void SetBackGround(unsigned long lValue);
	void SetBorder(unsigned long lValue);
	void SetdashLine(unsigned long lValue);
	void SetPriceColor(unsigned long lValue);
	void SetAveragePriceColor(unsigned long lValue);
	void SetCurSelLineColor(unsigned long lValue);
	void SetTipsBkColor(unsigned long lValue);
	void SetNameColor(unsigned long lValue);
	void SetTimeColor(unsigned long lValue);
	void SetzhangColor(unsigned long lValue);
	void SetdieColor(unsigned long lValue);
	void SethengpanColor(unsigned long lValue);
	void SetLineNameColor(unsigned long lValue);
	void SetSpanLine(int nLineType, int	nLineSize);
	void SetHourLineLine(int nLineType, int	nLineSize);
	void SetHalfHourLine(int nLineType, int	nLineSize);
private:
	DECLARE_DYNAMIC_CLASS (CTLine)
	DECLARE_EVENT_TABLE()
	void DrawDC(wxDC& dc);
	void OnPaint (wxPaintEvent & PaintEvent);

	void DrawBorder(wxDC& dc, wxRect& rt);
	void DrawScales(wxDC& dc);
	void DrawHead(wxDC& dc);
	void DrawVolumeLine(wxDC& dc);
	void SetTextColor(wxDC& dc, double dbClosePrice);
	void DrawCurSelLine(wxDC& dc);
	void DrawTips(wxDC& dc);

	void DrawTips_Time(wxDC& dc, wxRect rtClient, int nX, bool bIsMouse);


private:
	int         m_nSpanLineType;		//��ʱ���������� 0���㻮�ߣ�1��ʵ����
	int			m_nSpanLineSize;		//��ʱ�����߿���
	int         m_nHourLineType;		//Сʱ���������� 0���㻮�ߣ�1��ʵ����
	int			m_nHourLineSize;		//Сʱ�����߿���
	int         m_nHalfHourLineLineType;//��Сʱ���������� 0���㻮�ߣ�1��ʵ����
	int			m_nHalfHourLineSize;	//��Сʱ�����߿���

};