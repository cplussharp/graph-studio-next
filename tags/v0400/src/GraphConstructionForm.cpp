//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

using namespace GraphStudio;

//-----------------------------------------------------------------------------
//
//	CGraphConstructionForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CGraphConstructionForm, CDialog)

BEGIN_MESSAGE_MAP(CGraphConstructionForm, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CGraphConstructionForm class
//
//-----------------------------------------------------------------------------

CGraphConstructionForm::CGraphConstructionForm(CWnd *pParent) : 
	CDialog(CGraphConstructionForm::IDD, pParent)
{
}

CGraphConstructionForm::~CGraphConstructionForm()
{
}

void CGraphConstructionForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_EXPLORER1, browser_window);
}

BOOL CGraphConstructionForm::DoCreateDialog()
{
	BOOL ret = Create(IDD);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	CComVariant		v;

	// navigate to empty page to make the document valid
	browser_window.Navigate(_T("about:blank"), &v, &v, &v, &v);

	SetWindowPos(NULL, 0, 0, 400, 250, SWP_NOMOVE | SWP_NOACTIVATE);
	return TRUE;
}


void CGraphConstructionForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(title)) {
		title.GetClientRect(&rc2);

		browser_window.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW);

		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW);
		title.Invalidate();
	}
}

bool is_item_allowed(CString name)
{
	if (name == _T("Registered Pins")
		) return false;

	return true;
}

int get_html_count(PropItem *item)
{
	if (!item) return 0;
	if (!is_item_allowed(item->name)) return 0;

	// does it have subitems ?
	if (item->GetCount() == 0) {	
		if (is_item_allowed(item->name)) return 1;
		return 0;
	} else {
		int		cnt = 0;
		for (int i=0; i<item->GetCount(); i++) {
			cnt += get_html_count(item->GetItem(i));
		}
		return cnt;
	}
}

void render_html_list(PropItem *item, CString &html)
{
	if (!item) return ;
	if (item->name == _T("Registered Pins")) return ;

	// does it have subitems ?
	if (item->GetCount() == 0) {
		if (!is_item_allowed(item->name)) return ;

		CString		t;
		if (html == _T("")) {
			t.Format(_T("<td>%s</td><td width=70%%>%s</td></tr>\n"), item->name.GetBuffer(), item->value.GetBuffer());
			html += t;
		} else {
			t.Format(_T("<tr><td>%s</td><td>%s</td></tr>\n"), item->name.GetBuffer(), item->value.GetBuffer());
			html += t;
		}
	} else {
		for (int i=0; i<item->GetCount(); i++) {
			render_html_list(item->GetItem(i), html);
		}
	}
}


void CGraphConstructionForm::GenerateHTML(CString &text, GraphStudio::RenderParameters *params)
{
	text = _T("<html><table>\n");

	// now loop through the filters
	for (UINT i=0; i<params->render_actions.size(); i++) {

		RenderAction		&action = params->render_actions[i];
		CString				item, t;

		if (action.type == RenderAction::ACTION_SELECT) {

			DSUtil::FilterTemplate		templ;
			templ.LoadFromMoniker(action.displ_name);
			item = _T("<tr bgcolor=\"#8D8DF0\">");

				t.Format(_T("%5.3f sec"), action.time_ms / 1000.0);
				item += _T("<td width=100>") + t + _T("</td>");
				item += _T("<td width=90% colspan=2>Selected : ") + templ.name + _T("</td>");

			item += _T("</tr>\n");
			text += item;

		} else
		if (action.type == RenderAction::ACTION_CREATE) {

			PropItem			info(_T("info"));
			GetFilterDetails(action.clsid, &info);

			// find for the name
			CString				name = _T("Unknown Filter");
			int					i;
			for (i=0; i<info.GetCount(); i++) {				
				PropItem		*item = info.GetItem(i);
				if (item->name == _T("Object Name")) name = item->value;
			}

			item = _T("<tr bgcolor=\"#D0D0D0\">");

				t.Format(_T("%5.3f sec"), action.time_ms / 1000.0);
				item += _T("<td width=100>") + t + _T("</td>");
				item += _T("<td width=90% colspan=2>Created : ") + name + _T("</td>");

			item += _T("</tr>\n");
			text += item;

			// find out what value to set for rowspan
			int rowspan = get_html_count(&info);

			CString		html_list, it;
			it.Format(_T("<tr><td rowspan=%d></td>"), rowspan);
			render_html_list(&info, html_list);
			it += html_list;

			text += it;
		}

	}

	text += _T("</table></html>\n");
	int len = text.GetLength();
	if (len < 0) {
		text = _T("");
	}
}

void CGraphConstructionForm::Reload(GraphStudio::RenderParameters *params)
{
	CString		text;
	GenerateHTML(text, params);

	LPDISPATCH		doc = browser_window.get_Document();
	if (doc) {
		CComPtr<IHTMLDocument2>	html;
		doc->QueryInterface(IID_IHTMLDocument2, (void**)&html);
		if (html) {

			CComBSTR    bstrURL;
			CComVariant varDummy;

			html->open(bstrURL, CComVariant("_self"), varDummy, varDummy, NULL);

			// Create a safearray to store the HTML text
			SAFEARRAY      *pSA = NULL;
			SAFEARRAYBOUND  saBound = {1, 0};
			pSA = SafeArrayCreate(VT_VARIANT, 1, &saBound);
			if (pSA) {

				// Copy the HTML into the one and only element
				VARIANT   *pVar;	
				SafeArrayAccessData(pSA, (void**)&pVar);    // Access safearray data

				V_BSTR(pVar)	= SysAllocString(text);
				V_VT(pVar)		= VT_BSTR;

				SafeArrayUnaccessData(pSA);                 // Release access
				// Write the HTML as the document's new text
				html->write(pSA);                           // Overwrite HTML
				SafeArrayDestroy(pSA);                      // Finished with the safearray
			}

			html->close();  
		}
		doc->Release();
	}	
}

