#include "StdAfx.h"

#include "../../../inc/Quotation/GridEx.h"

#include "../../../inc/Quotation/coolscroll.h"
#define MAX(a,b) (a)>(b)?(a):(b)
#define MIN(a,b) (a)<(b)?(a):(b)
int nVScrollMax, nVScrollPos, nVScrollPage;
int nHScrollMax, nHScrollPos, nHScrollPage;

int nVMaxLines;
int nHMaxLines;




IMPLEMENT_DYNAMIC_CLASS(wxGridEx, wxGrid)
DEFINE_EVENT_TYPE(wxEVT_GridEx_KeyDown)
wxGridEx::wxGridEx(void)
{
	m_Leftpanelzhang		= wxColour(192, 0, 0);	
	m_TextColor				= wxColour(192, 192, 192);
	m_nHeadFontSize         = 8;

	m_bSize = false;
}
wxGridEx::~wxGridEx(void)
{
	
}
wxGridEx::wxGridEx( wxWindow *parent,
			   wxWindowID id,
			   const wxPoint& pos,
			   const wxSize& size,
			   long style,
			   const wxString& name ):
wxGrid(parent,id,pos,size,style)			 
{	
	m_Leftpanelzhang		= wxColour(192, 0, 0);	
	m_TextColor				= wxColour(192, 192, 192);
	bool created = m_created;

	InitializeCoolSB((HWND)GetHWND());
	::ShowScrollBar((HWND)GetHWND(),SB_BOTH,FALSE);
}

BEGIN_EVENT_TABLE(wxGridEx, wxGrid)
EVT_KEY_DOWN( wxGridEx::OnKeyDown )
EVT_SIZE(wxGridEx::OnSize)
END_EVENT_TABLE()

void wxGridEx::DrawColLabel( wxDC& dc, int col )
{
	if ( GetColWidth(col) <= 0 || m_colLabelHeight <= 0 )
		return;

	int colLeft = GetColLeft(col);

	wxRect rect;

#if 0
	def __WXGTK20__
		rect.SetX( colLeft + 1 );
	rect.SetY( 1 );
	rect.SetWidth( GetColWidth(col) - 2 );
	rect.SetHeight( m_colLabelHeight - 2 );

	wxWindowDC *win_dc = (wxWindowDC*) &dc;

	wxRendererNative::Get().DrawHeaderButton( win_dc->m_owner, dc, rect, 0 );
#else
	int colRight = GetColRight(col) - 1;
	
	if(col == m_numCols-1)
		colRight = colRight+26;//股东条占16个像素
		//nColRight = m_extraWidth;

	dc.SetPen( wxPen(m_Leftpanelzhang, 1, wxSOLID) );
	if(col == m_numCols-1)
		dc.DrawLine( colRight, 0, colRight, m_colLabelHeight - 1 );
	
	dc.DrawLine( colLeft, 0, colRight+1, 0 );
	dc.DrawLine( colLeft, m_colLabelHeight - 1,
		colRight + 1, m_colLabelHeight - 1 );

	dc.SetPen( m_Leftpanelzhang );
	dc.DrawLine( colLeft, 1, colLeft, m_colLabelHeight - 1 );
	//dc.DrawLine( colLeft, 1, colRight, 1 );
#endif


	dc.SetBackgroundMode( wxTRANSPARENT );
	dc.SetTextForeground( GetLabelTextColour() );

	wxFont wxfont(m_nHeadFontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT, false); 	
	dc.SetFont(wxfont);


	int hAlign, vAlign, orient;
	GetColLabelAlignment( &hAlign, &vAlign );
	orient = GetColLabelTextOrientation();

	rect.SetX( colLeft + 2 );
	rect.SetY( 2 );
	rect.SetWidth( GetColWidth(col) - 4 );
	rect.SetHeight( m_colLabelHeight - 4 );
	DrawTextRectangle( dc, GetColLabelValue( col ), rect, hAlign, vAlign, orient );
}
void wxGridEx::OnKeyDown( wxKeyEvent& event )
{
	int nkeyCode = event.GetKeyCode();
	switch ( nkeyCode )
	{
	case WXK_PRIOR:
	case WXK_NEXT:
		{
			wxWindow* window= GetParent();
			wxCommandEvent myEvent(wxEVT_GridEx_KeyDown);
			myEvent.SetInt(nkeyCode);
			window->ProcessEvent(myEvent);
		}
		break;
	default:
		event.Skip();
		break;
	}
}

void wxGridEx::SetScrollPos( int orient, int pos, bool refresh)
{
	if(orient == wxHORIZONTAL)
		return;
	SCROLLINFO si;
	si.cbSize	= sizeof(si);
	si.fMask = SIF_POS;
	si.nPos		= pos;
	si.nPage = 0;
	si.nMin = 0;
	if ( HasFlag(wxALWAYS_SHOW_SB) )
	{
		// disable scrollbar instead of removing it then
		si.fMask |= SIF_DISABLENOSCROLL;
	}
	CoolSB_SetScrollInfo ((HWND)GetHWND(), orient == wxHORIZONTAL ? SB_HORZ : SB_VERT, &si, refresh);

	nVScrollPos = pos;
	if(nVScrollPos < 0) 
		nVScrollPos = 0;

	if(nVScrollPos > nVScrollMax - nVScrollPage + 1) 
		nVScrollPos = nVScrollMax - nVScrollPage + 1;
}
void wxGridEx::SetScrollbar(int orient, int pos, int pageSize, int range, bool refresh)
{
	if(orient == wxHORIZONTAL)
		return;

	SCROLLINFO info;
	info.nPage = pageSize;
	info.nMin = 0;              // range is nMax - nMin + 1
	info.nMax = range - 1;      //  as both nMax and nMax are inclusive
	info.nPos = pos;
	info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	if ( HasFlag(wxALWAYS_SHOW_SB) )
	{
		// disable scrollbar instead of removing it then
		info.fMask |= SIF_DISABLENOSCROLL;
	}

	HWND hWnd = (HWND)GetHWND();
	if ( hWnd )
	{
		// We have to set the variables here to make them valid in events
		// triggered by ::SetScrollInfo()
		*(orient == wxHORIZONTAL ? &m_xThumbSize : &m_yThumbSize) = pageSize;

		CoolSB_SetScrollInfo (hWnd,orient == wxHORIZONTAL ? SB_HORZ : SB_VERT,
			&info, refresh);
		nVScrollMax = range;
		nVScrollPos = pos;
		if(nVScrollPos < 0) 
			nVScrollPos = 0;

		if(nVScrollPos > nVScrollMax - nVScrollPage + 1) 
			nVScrollPos = nVScrollMax - nVScrollPage + 1;
		nVScrollPage = pageSize;
	}
}
WXLRESULT wxGridEx::MSWWindowProc(WXUINT   message,   WXWPARAM   wParam,   WXLPARAM   lParam) 
{
	if(WM_SIZE==message)
	{
		SCROLLINFO si;
		RECT rect;

		::GetClientRect((HWND)GetHWND(), &rect);
		// VERT
		nVMaxLines	=SetOrCalcRowSizes(true)/GetScrollLineY()+1;
		nVScrollPage = MIN(nVMaxLines + 1, (rect.bottom - rect.top) / GetScrollLineY());
		nVScrollMax  = MAX(0, nVMaxLines);
		nVScrollPos  = MIN(nVScrollPos, nVScrollMax - nVScrollPage + 1);

		si.cbSize	= sizeof(si);
		si.fMask	= SIF_ALL;//SIF_PAGE | SIF_POS | SIF_RANGE|SIF_TRACKPOS;
		si.nMin		= 0;
		si.nMax		= nVScrollMax;
		si.nPos		= nVScrollPos;
		si.nPage	= MIN(nVScrollPage, nVScrollMax + 1);

		CoolSB_SetScrollInfo ((HWND)GetHWND(), SB_VERT, &si, TRUE);
/*
		// HORZ
		nHMaxLines	=SetOrCalcColumnSizes(true)/GetScrollLineX()+1;
		nHScrollPage = MIN(nHMaxLines + 1, (rect.right - rect.left) / GetScrollLineX());
		nHScrollMax  = MAX(0, nHMaxLines);
		nHScrollPos  = MIN(nHScrollPos, nHScrollMax - nHScrollPage + 1);

		si.cbSize	= sizeof(si);
		si.fMask	= SIF_ALL;// SIF_PAGE | SIF_POS | SIF_RANGE|SIF_TRACKPOS ;
		si.nMin		= 0;
		si.nMax		= nHScrollMax;
		si.nPos		= nHScrollPos;
		si.nPage	= MIN(nHScrollPage, nHScrollMax + 1);

		CoolSB_SetScrollInfo ((HWND)GetHWND(), SB_HORZ, &si, TRUE);*/
		HandleSize( LOWORD(lParam), HIWORD(lParam),wParam);
		return 0;
	}
	else if(WM_VSCROLL==message)
	{	
		wxString str;
		str.Printf(wxT("%d\n"), LOWORD(wParam));
		OutputDebugString(str.c_str());
		static int countV;
		char ach[80];
		switch(LOWORD(wParam))
		{
		case SB_LINEUP:		nVScrollPos --;					break;
		case SB_LINEDOWN:	nVScrollPos ++;					break;
		case SB_PAGEUP:		nVScrollPos -= nVScrollPage;	break;
		case SB_PAGEDOWN:	nVScrollPos += nVScrollPage;	break;
		case SB_TOP:		nVScrollPos = 0;				break;
		case SB_BOTTOM:		nVScrollPos = nVScrollMax -1;	break;
		case SB_THUMBTRACK: nVScrollPos = HIWORD(wParam);	break;
		}

		wsprintf(ach, "WM_VSCROLL %d\n", countV++);
		OutputDebugString(ach);

		if(nVScrollPos < 0) 
			nVScrollPos = 0;

		if(nVScrollPos > nVScrollMax - nVScrollPage + 1) 
			nVScrollPos = nVScrollMax - nVScrollPage + 1;
		Scroll(-1,nVScrollPos);
		CoolSB_SetScrollPos((HWND)GetHWND(), SB_VERT, nVScrollPos, TRUE);

		return 0;	
	}
	else if(WM_HSCROLL==message)
	{/*
		static int countH;
		char ach[80];
		switch(LOWORD(wParam))
		{
		case SB_LINEUP:		nHScrollPos --;					break;
		case SB_LINEDOWN:	nHScrollPos ++;					break;
		case SB_PAGEUP:		nHScrollPos -= nHScrollPage;	break;
		case SB_PAGEDOWN:	nHScrollPos += nHScrollPage;	break;
		case SB_TOP:		nHScrollPos = 0;				break;
		case SB_BOTTOM:		nHScrollPos = nHScrollMax -1;	break;
		case SB_THUMBTRACK: nHScrollPos = HIWORD(wParam);	break;
		}
		wsprintf(ach, "WM_HSCROLL %d\n", countH++);
		OutputDebugString(ach);
		if(nHScrollPos < 0) 
			nHScrollPos = 0;

		if(nHScrollPos > nHScrollMax - nHScrollPage + 1) 
			nHScrollPos = nHScrollMax - nHScrollPage + 1;
		Scroll(nHScrollPos,-1);
		CoolSB_SetScrollPos((HWND)GetHWND(), SB_HORZ, nHScrollPos, TRUE);

		return 0;	*/
	}
	return wxGrid::MSWWindowProc(message,wParam,lParam); 
}

void wxGridEx::AdjustScrollbars()
{
	/*AdjustScrollbars()函数是从基类拷贝过来的，这部分注掉是编译不过的原因，有啥影响尚不知？	
	static wxRecursionGuardFlag s_flagReentrancy;
	wxRecursionGuard guard(s_flagReentrancy);
	if ( guard.IsInside() )
	{
	// don't reenter AdjustScrollbars() while another call to
	// AdjustScrollbars() is in progress because this may lead to calling
	// ScrollWindow() twice and this can really happen under MSW if
	// SetScrollbar() call below adds or removes the scrollbar which
	// changes the window size and hence results in another
	// AdjustScrollbars() call
	return;
	}
	*/
	int w = 0, h = 0;
	int oldw, oldh;

	int oldXScroll = m_xScrollPosition;
	int oldYScroll = m_yScrollPosition;

	// VZ: at least under Windows this loop is useless because when scrollbars
	//     [dis]appear we get a WM_SIZE resulting in another call to
	//     AdjustScrollbars() anyhow. As it doesn't seem to do any harm I leave
	//     it here for now but it would be better to ensure that all ports
	//     generate EVT_SIZE when scrollbars [dis]appear, emulating it if
	//     necessary, and remove it later
	// JACS: Stop potential infinite loop by limiting number of iterations
	int iterationCount = 0;
	const int iterationMax = 5;
	do
	{
		iterationCount ++;

		GetTargetSize(&w, 0);

		// scroll lines per page: if 0, no scrolling is needed
		int linesPerPage;

		if ( m_xScrollPixelsPerLine == 0 )
		{
			// scrolling is disabled
			m_xScrollLines = 0;
			m_xScrollPosition = 0;
			linesPerPage = 0;
		}
		else // might need scrolling
		{
			// Round up integer division to catch any "leftover" client space.
			const int wVirt = m_targetWindow->GetVirtualSize().GetWidth();
			m_xScrollLines = (wVirt + m_xScrollPixelsPerLine - 1) / m_xScrollPixelsPerLine;

			// Calculate page size i.e. number of scroll units you get on the
			// current client window.
			linesPerPage = w / m_xScrollPixelsPerLine;

			// Special case. When client and virtual size are very close but
			// the client is big enough, kill scrollbar.
			if ((linesPerPage < m_xScrollLines) && (w >= wVirt)) ++linesPerPage;

			if (linesPerPage >= m_xScrollLines)
			{
				// we're big enough to not need scrolling
				linesPerPage =
					m_xScrollLines =
					m_xScrollPosition = 0;
			}
			else // we do need a scrollbar
			{
				if ( linesPerPage < 1 )
					linesPerPage = 1;

				// Correct position if greater than extent of canvas minus
				// the visible portion of it or if below zero
				const int posMax = m_xScrollLines - linesPerPage;
				if ( m_xScrollPosition > posMax )
					m_xScrollPosition = posMax;
				else if ( m_xScrollPosition < 0 )
					m_xScrollPosition = 0;
			}
		}

		m_win->SetScrollbar(wxHORIZONTAL, m_xScrollPosition,
			linesPerPage, m_xScrollLines);

		// The amount by which we scroll when paging
		SetScrollPageSize(wxHORIZONTAL, linesPerPage);

		GetTargetSize(0, &h);

		if ( m_yScrollPixelsPerLine == 0 )
		{
			// scrolling is disabled
			m_yScrollLines = 0;
			m_yScrollPosition = 0;
			linesPerPage = 0;
		}
		else // might need scrolling
		{
			// Round up integer division to catch any "leftover" client space.
			const int hVirt = m_targetWindow->GetVirtualSize().GetHeight();
			m_yScrollLines = ( hVirt + m_yScrollPixelsPerLine - 1 ) / m_yScrollPixelsPerLine;

			// Calculate page size i.e. number of scroll units you get on the
			// current client window.
			linesPerPage = h / m_yScrollPixelsPerLine;

			// Special case. When client and virtual size are very close but
			// the client is big enough, kill scrollbar.
			if ((linesPerPage < m_yScrollLines) && (h >= hVirt)) ++linesPerPage;

			if (linesPerPage >= m_yScrollLines)
			{
				// we're big enough to not need scrolling
				linesPerPage =
					m_yScrollLines =
					m_yScrollPosition = 0;
			}
			else // we do need a scrollbar
			{
				if ( linesPerPage < 1 )
					linesPerPage = 1;

				// Correct position if greater than extent of canvas minus
				// the visible portion of it or if below zero
				const int posMax = m_yScrollLines - linesPerPage;
				if(m_bSize)
					m_yScrollPosition = posMax;//模仿大智慧，放大缩小后，都是在最后一行
				if ( m_yScrollPosition > posMax )
					m_yScrollPosition = posMax;
				else if ( m_yScrollPosition < 0 )
					m_yScrollPosition = 0;
			}
		}

		m_win->SetScrollbar(wxVERTICAL, m_yScrollPosition,
			linesPerPage, m_yScrollLines);


		// The amount by which we scroll when paging
		SetScrollPageSize(wxVERTICAL, linesPerPage);


		// If a scrollbar (dis)appeared as a result of this, adjust them again.
		oldw = w;
		oldh = h;

		GetTargetSize( &w, &h );
	} while ( (w != oldw || h != oldh) && (iterationCount < iterationMax) );

#ifdef __WXMOTIF__
	// Sorry, some Motif-specific code to implement a backing pixmap
	// for the wxRETAINED style. Implementing a backing store can't
	// be entirely generic because it relies on the wxWindowDC implementation
	// to duplicate X drawing calls for the backing pixmap.

	if ( m_targetWindow->GetWindowStyle() & wxRETAINED )
	{
		Display* dpy = XtDisplay((Widget)m_targetWindow->GetMainWidget());

		int totalPixelWidth = m_xScrollLines * m_xScrollPixelsPerLine;
		int totalPixelHeight = m_yScrollLines * m_yScrollPixelsPerLine;
		if (m_targetWindow->GetBackingPixmap() &&
			!((m_targetWindow->GetPixmapWidth() == totalPixelWidth) &&
			(m_targetWindow->GetPixmapHeight() == totalPixelHeight)))
		{
			XFreePixmap (dpy, (Pixmap) m_targetWindow->GetBackingPixmap());
			m_targetWindow->SetBackingPixmap((WXPixmap) 0);
		}

		if (!m_targetWindow->GetBackingPixmap() &&
			(m_xScrollLines != 0) && (m_yScrollLines != 0))
		{
			int depth = wxDisplayDepth();
			m_targetWindow->SetPixmapWidth(totalPixelWidth);
			m_targetWindow->SetPixmapHeight(totalPixelHeight);
			m_targetWindow->SetBackingPixmap((WXPixmap) XCreatePixmap (dpy, RootWindow (dpy, DefaultScreen (dpy)),
				m_targetWindow->GetPixmapWidth(), m_targetWindow->GetPixmapHeight(), depth));
		}

	}
#endif // Motif

	if (oldXScroll != m_xScrollPosition)
	{
		if (m_xScrollingEnabled)
			m_targetWindow->ScrollWindow( m_xScrollPixelsPerLine * (oldXScroll - m_xScrollPosition), 0,
			GetScrollRect() );
		else
			m_targetWindow->Refresh(true, GetScrollRect());
	}

	if (oldYScroll != m_yScrollPosition)
	{
		if (m_yScrollingEnabled)
			m_targetWindow->ScrollWindow( 0, m_yScrollPixelsPerLine * (oldYScroll-m_yScrollPosition),
			GetScrollRect() );
		else
			m_targetWindow->Refresh(true, GetScrollRect());
	}
}
void wxGridEx::OnSize( wxSizeEvent& WXUNUSED(event) )
{
	m_bSize = true;
	if (m_targetWindow != this) // check whether initialisation has been done
	{
		// update our children window positions and scrollbars
		CalcDimensions();
	}
	m_bSize = false;
}









//以下BigGridTable类的实现
BigGridTable::BigGridTable(long lRows, long lCols) 
{
	m_rowcount = lRows;		
	m_colcount = lCols;
	//m_pTextData = NULL;
	m_dbLastPrice = 0.0;
	m_clText				= wxColour(192,192,192);
	m_Leftpanelzhang		= wxColour(240, 0, 0);
	m_Leftpaneldie			= wxColour(0, 240, 0);
	m_Leftpanelhengpan		= wxColour(255, 255, 255);
	m_clVolume				= wxColour(255, 255, 82);
	m_pFData = NULL;
	m_nBodyfontSize = 10;
	//m_vecQIntime = NULL;
}

int BigGridTable::GetNumberRows() { return m_rowcount; }
int BigGridTable::GetNumberCols() { return m_colcount; }

wxString BigGridTable::GetValue( int row, int col )
{
	if(m_pFData == NULL)
		return wxEmptyString;

	if(row <0 || row >= (int)(m_pFData->m_vecQIntime).size()
		|| col<0|| col>=6)
		return wxEmptyString;
	wxString strText ="";
	SQIntime * pQIntime = (m_pFData->m_vecQIntime)[row];
	switch(col)
	{
	case 0://分笔
		{
			SQIntime *pItemLast2 = NULL;
			if(row -1>= 0)
				pItemLast2 = (m_pFData->m_vecQIntime)[row-1];
			if(pItemLast2 == NULL)
				strText.Printf(wxT("%.2d:%.2d"), pQIntime->dwTime.tm_hour, pQIntime->dwTime.tm_min);
			else
			{
				if(pQIntime->dwTime.tm_year == pItemLast2->dwTime.tm_year
					&& pQIntime->dwTime.tm_mon == pItemLast2->dwTime.tm_mon 
					&& pQIntime->dwTime.tm_mday == pItemLast2->dwTime.tm_mday
					&& pQIntime->dwTime.tm_hour == pItemLast2->dwTime.tm_hour
					&& pQIntime->dwTime.tm_min == pItemLast2->dwTime.tm_min)
					strText.Printf(wxT("   :%.2d"),  pQIntime->dwTime.tm_sec);			
				else
					strText.Printf(wxT("%.2d:%.2d"), pQIntime->dwTime.tm_hour, pQIntime->dwTime.tm_min);
			}
			break;
		}
	case 1://价格
		{
			wxString strFormat;
			strFormat.Printf(wxT("%%.%df"), m_nNumDigits);
			strText.Printf(strFormat, pQIntime->fLastPrice);					
			break;
		}
	case 2://量
		{
			strText.Printf(wxT("%d"),  pQIntime->dwVolume );
			break;
		}
	case 3://开
		{
			strText.Printf(wxT("%d"),  pQIntime->dwOpenVolume );
			break;
		}
	case 4://平
		{
			strText.Printf(wxT("%d"),  pQIntime->dwCloseVolume );
			break;
		}
	case 5://性质
		{
			switch(pQIntime->nDesc)
			{
			case 0:
				strText = KONGKAI;
				break;
			case 1:
				strText = DUOKAI;
				break;
			case 2:
				strText = DUOPING;
				break;
			case 3:
				strText = KONGPING;
				break;
			case 4:
				strText = DUOHUAN;
				break;
			case 5:
				strText = KONGHUAN;
				break;
			case 6:
				strText = SHUANGPING;
				break;
			case 7:
				strText = SHUANGKAI;
				break;
			}
			//	strText =  pQIntime->strDesc.c_str() ;
			break;
		}
	}
	return strText;
}
/*
性质的区分：
开仓数大于平仓数，则属于开仓。  成交价格属于买一价，则为空开，成交价格为卖一价，则为多开。
开仓数小于平仓数，则属于平仓。	成交价格属于买一价，则为多平，成交价格为卖一价，则为空平。
开仓数等于平仓数，则属于换仓。	成交价格属于买一价，则为多换，成交价格为卖一价，则为空换。
开仓数等于0，则为双平。
平仓数等于0，则为双开。

仓差：开仓量减去平仓量    即对持仓量的即时性增减。

颜色说明：成交价为买一价时，显示为绿色  代表价格下降。
成交价为卖一价时，显示为红色，代表价格上升。//这两点简单处理就是针对上一笔价格的上涨还是下跌判定颜色，上涨为红色，下跌为绿色
成交价为最新价时:
颜色首先根据价格判断，影响量和性质。如果价格不变，则根据性质判断。
多换（红色）       空换（绿色）
双开（红色）       双平（绿色）
空平（红色）       多平（绿色）
多开（红色）       空开（绿色）

*/
wxGridCellAttr* BigGridTable::GetAttr( int row, int col,wxGridCellAttr::wxAttrKind  kind )
{
	wxGridCellAttr *pCellAttr = new wxGridCellAttr(); 
	if(m_pFData == NULL)
		return pCellAttr;
	switch(col)
	{
	case 0://分笔
		{
			bool bBold = false;
			if(row >= 0 && row < (int)(m_pFData->m_vecQIntime).size())
			{
				SQIntime *pItemLast2 = NULL;
				if(row -1>= 0)
					pItemLast2 = (m_pFData->m_vecQIntime)[row-1];
				if(pItemLast2 == NULL)
				{
					wxFont font(m_nBodyfontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false); 	
					pCellAttr->SetFont(font);	
				}
				else
				{
					SQIntime * pQIntime = (m_pFData->m_vecQIntime)[row];
					if(pQIntime->dwTime.tm_year == pItemLast2->dwTime.tm_year
						&& pQIntime->dwTime.tm_mon == pItemLast2->dwTime.tm_mon 
						&& pQIntime->dwTime.tm_mday == pItemLast2->dwTime.tm_mday
						&& pQIntime->dwTime.tm_hour == pItemLast2->dwTime.tm_hour
						&& pQIntime->dwTime.tm_min == pItemLast2->dwTime.tm_min)
					{

					}	
					else
					{
						bBold = true;
						wxFont font(m_nBodyfontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false); 	
						pCellAttr->SetFont(font);	
					}
				}
				
			}
			if(!bBold)	
			{
				wxFont font(m_nBodyfontSize-1, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT, false); 	
				pCellAttr->SetFont(font);	
			}
			pCellAttr->SetTextColour(m_clText);
			pCellAttr->SetAlignment(wxALIGN_RIGHT,-1);	
			break;
		}
	case 1://价格
		{
			if(row >=0 && row < (int)(m_pFData->m_vecQIntime).size())
			{
				double dbPrice = m_dbLastPrice;
				SQIntime *pItemLast = (m_pFData->m_vecQIntime)[row];
				if(pItemLast->fLastPrice > dbPrice)
					pCellAttr->SetTextColour(m_Leftpanelzhang);					
				else if(pItemLast->fLastPrice < dbPrice)
					pCellAttr->SetTextColour(m_Leftpaneldie);					
				else 
					pCellAttr->SetTextColour(m_Leftpanelhengpan);		

				wxFont font(m_nBodyfontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT, false); 	
				pCellAttr->SetFont(font);	
				pCellAttr->SetAlignment(wxALIGN_RIGHT,-1);	

			}				
			break;
		}
	case 2://量
		{
			wxFont font(m_nBodyfontSize-1, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT, false); 	
			pCellAttr->SetFont(font);	
			SetVolumeDescColor(row, pCellAttr);
			pCellAttr->SetAlignment(wxALIGN_RIGHT,-1);	
			break;
		}
	case 3://开
		{
			wxFont font(m_nBodyfontSize-1, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT, false); 	
			pCellAttr->SetFont(font);	
			pCellAttr->SetTextColour(m_clVolume);
			pCellAttr->SetAlignment(wxALIGN_RIGHT,-1);	
			break;
		}
	case 4://平
		{
			wxFont font(m_nBodyfontSize-1, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT, false); 	
			pCellAttr->SetFont(font);	
			pCellAttr->SetTextColour(m_clVolume);
			pCellAttr->SetAlignment(wxALIGN_RIGHT,-1);	
			break;
		}
	case 5://性质
		{
			wxFont font(m_nBodyfontSize-1, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT, false); 	
			pCellAttr->SetFont(font);	
			SetVolumeDescColor(row, pCellAttr);
			pCellAttr->SetAlignment(wxALIGN_RIGHT,-1);	
			break;
		}
	}
	pCellAttr->SetReadOnly();
	return pCellAttr;
}
void BigGridTable::SetVolumeDescColor(int row, wxGridCellAttr *pCellAttr)
{
	if(m_pFData == NULL)
		return;
	if(row >=0 && row < (int)m_pFData->m_vecQIntime.size())
	{//直接引用m_pFData->m_vecQIntime会导致多线程崩溃，要用同步锁，在fdata里面加函数，同步m_vecQIntime。暂时不改以后再说吧
		double dbPrice = m_dbLastPrice;
		if(row -1 >=0 && row < (int)(m_pFData->m_vecQIntime).size())
		{
			SQIntime *pitem = m_pFData->m_vecQIntime[row-1];
			dbPrice = pitem->fLastPrice;
		}
		SQIntime *pItemLast = m_pFData->m_vecQIntime[row];
		if(pItemLast->fLastPrice > dbPrice)
			pCellAttr->SetTextColour(m_Leftpanelzhang);					
		else if(pItemLast->fLastPrice < dbPrice)
			pCellAttr->SetTextColour(m_Leftpaneldie);					
		else 
		{
			switch(pItemLast->nDesc)
			{
			case 0://空开（绿色）0
				pCellAttr->SetTextColour(m_Leftpaneldie);	
				break;
			case 1://多开（红色） 1
				pCellAttr->SetTextColour(m_Leftpanelzhang);	
				break;
			case 2:// 多平（绿色）2
				pCellAttr->SetTextColour(m_Leftpaneldie);	
				break;
			case 3://空平（红色）3
				pCellAttr->SetTextColour(m_Leftpanelzhang);	
				break;
			case 4://多换（红色）4
				pCellAttr->SetTextColour(m_Leftpanelzhang);	
				break;
			case 5://空换（绿色）5
				pCellAttr->SetTextColour(m_Leftpaneldie);	
				break;
			case 6://双平（绿色）6
				pCellAttr->SetTextColour(m_Leftpaneldie);	
				break;
			case 7://双开（红色）7
				pCellAttr->SetTextColour(m_Leftpanelzhang);	
				break;
			}
		}
	}			
}
bool BigGridTable::DeleteRows( size_t pos, size_t numRows )
{
	m_rowcount = m_rowcount - numRows;		
	if ( GetView() )
	{
		wxGridTableMessage msg( this,
			wxGRIDTABLE_NOTIFY_ROWS_DELETED,
			pos,
			numRows );

		GetView()->ProcessTableMessage( msg );
	}

	return true;		
}
void BigGridTable::SetValue( int , int , const wxString&  ) { /* ignore */ }
bool BigGridTable::IsEmptyCell( int , int  ) { return false; }
void BigGridTable::SetNumberRows(int count){ m_rowcount=count; }
void BigGridTable::SetFData(CFData*		pFData)
{
	m_pFData = pFData;
}
void BigGridTable::SetColLabelValue( int col, const wxString& str)
{
	m_mapLabels[col] = str;
}


wxString BigGridTable::GetColLabelValue( int col )
{
	if(col <0 || col>= (int)m_mapLabels.size())
		return "";
	return m_mapLabels[col];
}
bool BigGridTable::AppendRows( size_t numRows /*= 1*/ )
{
	m_rowcount = m_rowcount + numRows;		
	if ( GetView() )
	{
		wxGridTableMessage msg( this,
			wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
			numRows );
		GetView()->ProcessTableMessage( msg );
	}
	return true;
}
void BigGridTable::SetLastPrice(double dbPrice)
{
	m_dbLastPrice = dbPrice;
}
bool BigGridTable::InitCfg(TiXmlElement *root)
{
	TiXmlElement *tList = root->FirstChildElement("biggridtable");
	if(tList == NULL)
		return false;

	TiXmlNode *pColor = tList->FirstChild("color");
	if(pColor == NULL)
		return false;

	TiXmlNode *ListItem = pColor->FirstChild("Text");
	if(ListItem == NULL)
		return false;
	const char *strValue = ListItem->ToElement()->GetText();
	unsigned long value = ColorStr2Long(strValue);
	SetclText(value);

	ListItem = pColor->FirstChild("Leftpanelzhang");
	if(ListItem == NULL)
		return false;
	strValue = ListItem->ToElement()->GetText();	
	value = ColorStr2Long(strValue);
	SetLeftpanelzhang(value);

	ListItem = pColor->FirstChild("Leftpaneldie");
	if(ListItem == NULL)
		return false;
	strValue = ListItem->ToElement()->GetText();	
	value = ColorStr2Long(strValue);
	SetLeftpaneldie(value);

	ListItem = pColor->FirstChild("Leftpanelhengpan");
	if(ListItem == NULL)
		return false;
	strValue = ListItem->ToElement()->GetText();	
	value = ColorStr2Long(strValue);
	SetLeftpanelhengpan(value);

	ListItem = pColor->FirstChild("Volume");
	if(ListItem == NULL)
		return false;
	strValue = ListItem->ToElement()->GetText();	
	value = ColorStr2Long(strValue);
	SetclVolume(value);

	ListItem = pColor->FirstChild("bodyfontsize");
	if(ListItem == NULL)
		return false;
	strValue = ListItem->ToElement()->GetText();	
	value = ColorStr2Long(strValue);
	SetBodyfontsize(value);
	return true;
}
void BigGridTable::SetBodyfontsize(unsigned long lValue)
{
	m_nBodyfontSize = lValue;
}
void BigGridTable::SetclText(unsigned long lValue)
{
	m_clText = lValue;
}
void BigGridTable::SetLeftpanelzhang(unsigned long lValue)
{
	m_Leftpanelzhang = lValue;
}
void BigGridTable::SetLeftpaneldie(unsigned long lValue)
{
	m_Leftpaneldie = lValue;
}
void BigGridTable::SetLeftpanelhengpan(unsigned long lValue)
{
	m_Leftpanelhengpan = lValue;
}	
void BigGridTable::SetclVolume(unsigned long lValue)
{
	m_clVolume = lValue;
}
void BigGridTable::SetNumDigits(int nNumDigits)
{
	m_nNumDigits = nNumDigits;
}
