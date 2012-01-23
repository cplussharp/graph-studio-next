//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

namespace GraphStudio
{

	//-------------------------------------------------------------------------
	//
	//	DisplayView class
	//
	//-------------------------------------------------------------------------

	IMPLEMENT_DYNCREATE(DisplayView, CScrollView)

	BEGIN_MESSAGE_MAP(DisplayView, CScrollView)
		// Standard printing commands
		ON_WM_ERASEBKGND()
		ON_WM_SIZE()
		ON_WM_MOUSEMOVE()
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_RBUTTONDOWN()
		ON_WM_LBUTTONUP()

		ON_COMMAND(ID_PIN_RENDER, &DisplayView::OnRenderPin)
		ON_COMMAND(ID_PIN_NULL_STREAM, &DisplayView::OnRenderNullStream)
		ON_COMMAND(ID_PIN_DUMP_STREAM, &DisplayView::OnDumpStream)
        ON_COMMAND(ID_PIN_TEE_STREAM, &DisplayView::OnTeeStream)
		ON_COMMAND(ID_PIN_FILE_WRITER, &DisplayView::OnFileWriterStream)
		ON_COMMAND(ID_PROPERTYPAGE, &DisplayView::OnPropertyPage)
		ON_COMMAND(ID_DELETE_FILTER, &DisplayView::OnDeleteFilter)

		ON_COMMAND_RANGE(ID_STREAM_SELECT, ID_STREAM_SELECT+100, &DisplayView::OnSelectStream)
		ON_COMMAND_RANGE(ID_COMPATIBLE_FILTER, ID_COMPATIBLE_FILTER+999, &DisplayView::OnCompatibleFilterClick)

	END_MESSAGE_MAP()

	DisplayView::DisplayView()
	{
		back_width = 0;
		back_height = 0;
		overlay_filter = NULL;

		// nastavime DC
		graph.params = &render_params;
		graph.callback = this;
		graph.dc = &memDC;
	}

	DisplayView::~DisplayView()
	{
		if (memDC.GetSafeHdc()) {
			memDC.DeleteDC();
			backbuffer.DeleteObject();
		}
	}

	BOOL DisplayView::OnEraseBkgnd(CDC* pDC)
	{
		return TRUE;
	}

	void DisplayView::OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		point += GetScrollPosition();

		// find out if there is any filter being selected
		current_filter = graph.FindFilterByPos(point);
		if (!current_filter) return ;

		// check for a pin - will have different menu
		current_pin = current_filter->FindPinByPos(point, false);
		OnPropertyPage();
	}

	void DisplayView::OnRButtonDown(UINT nFlags, CPoint point)
	{
		point += GetScrollPosition();

		CMenu	menu;
		// find out if there is any filter being selected
		current_filter = graph.FindFilterByPos(point);
		if (!current_filter) return ;

		if (!menu.CreatePopupMenu()) return ;

		FILTER_STATE	state = State_Running;
		if (graph.GetState(state, 0) != 0) {
			state = State_Running;
		}

		// check for a pin - will have different menu
		current_pin = current_filter->FindPinByPos(point, false);
		if (current_pin) {

			// make rendering inactive for connected pins
			UINT	flags = 0;
			if (current_pin->connected) flags |= MF_GRAYED;
			if (current_pin->dir != PINDIR_OUTPUT) flags |= MF_GRAYED;
			if (state != State_Stopped) flags |= MF_GRAYED;


			/*
				If the pin is not connected we might try to
				check it for MEDIATYPE_Stream so we can offer to connect
				the File Writer filter.
			*/

			bool	offer_writer = false;
			if (current_pin->connected == false) {
			
				DSUtil::MediaTypes			mtypes;
				HRESULT						hr;

				// we will ignore the async file source filter
				if (current_filter->clsid != CLSID_AsyncReader) {
					hr = DSUtil::EnumMediaTypes(current_pin->pin, mtypes);
					if (SUCCEEDED(hr)) {
						for (int i=0; i<mtypes.GetCount(); i++) {
							if (mtypes[i].majortype == MEDIATYPE_Stream) {
								offer_writer = true;
								break;
							}
						}
					}
				}
			}

			int		p = 0;
			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | flags, ID_PIN_RENDER, _T("Render Pin"));
			menu.InsertMenu(p++, MF_BYPOSITION | MF_SEPARATOR);
			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | flags, ID_PIN_NULL_STREAM, _T("Insert Null Renderer"));
			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | flags, ID_PIN_DUMP_STREAM, _T("Insert Dump Filter"));
            menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | flags, ID_PIN_TEE_STREAM, _T("Insert Tee Filter"));

			if (offer_writer) {
				menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | flags, ID_PIN_FILE_WRITER, _T("Insert File Writer"));
			}

			// check for compatible filters
			PrepareCompatibleFiltersMenu(menu, current_pin);

			p = menu.GetMenuItemCount();
			menu.InsertMenu(p++, MF_BYPOSITION | MF_SEPARATOR);
			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING, ID_PROPERTYPAGE, _T("Properties"));

			// check for IAMStreamSelect interface
			PrepareStreamSelectMenu(menu, current_pin->pin);

		} else {
			menu.InsertMenu(0, MF_STRING, ID_PROPERTYPAGE, _T("Properties"));

			int p = menu.GetMenuItemCount();
			menu.InsertMenu(p++, MF_BYPOSITION | MF_SEPARATOR);
			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING, ID_DELETE_FILTER, _T("Delete Selection"));


			// check for IAMStreamSelect interface
			PrepareStreamSelectMenu(menu, current_filter->filter);
		}

		CPoint	pt;
		GetCursorPos(&pt);

		// display menu
		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}

	void DisplayView::OnLButtonDown(UINT nFlags, CPoint point)
	{
		point += GetScrollPosition();

		SetCapture();	
		start_drag_point = point;
		end_drag_point = point;

		Filter	*current = graph.FindFilterByPos(point);
		if (!current) {
			// deselect all filters
			bool need_invalidate = false;
			for (int i=0; i<graph.filters.GetCount(); i++) {
				if (graph.filters[i]->selected) {
					graph.filters[i]->Select(false);
				}
				graph.filters[i]->SelectConnection(nFlags, point);
				need_invalidate = true;
			}

			// store the selection point
			drag_mode = DisplayView::DRAG_SELECTION;

			if (need_invalidate) {
				graph.Dirty();
				Invalidate();
			}
			return ;
		}

		// check if we hit a pin
		Pin *hitpin = current->FindPinByPos(point);
		if (hitpin) {
			// deselect all filters
			for (int i=0; i<graph.filters.GetCount(); i++) {
				graph.filters[i]->Select(false);
			}

			// remember the start point
			hitpin->GetCenterPoint(&new_connection_start);
			new_connection_end = new_connection_start;
			drag_mode = DisplayView::DRAG_CONNECTION;

		} else {

			int icon = current->CheckIcons(point);
			if (icon >= 0) {
				drag_mode = DisplayView::DRAG_OVERLAY_ICON;
				return ;
			}

			if (current->selected) {
				if (nFlags & MK_SHIFT) {
					current->Select(false);
					graph.Dirty();
					Invalidate();
				} else {
					// nothing here...
				}
			} else {
				if (nFlags & MK_SHIFT) {
					current->Select(true);
					graph.Dirty();
					Invalidate();
				} else {
					// deselect all filters but this
					for (int i=0; i<graph.filters.GetCount(); i++) {
						graph.filters[i]->Select(false);
					}
					current->Select(true);
					graph.Dirty();
					Invalidate();
				}
			}

			drag_mode = DisplayView::DRAG_GROUP;

			// start dragging operation on all selected filters
			start_drag_point = point;
			for (int i=0; i<graph.filters.GetCount(); i++) {
				graph.filters[i]->BeginDrag();
			}
		}
	}

	void DisplayView::OnLButtonUp(UINT nFlags, CPoint point)
	{
		point += GetScrollPosition();

		// check for an overlay icon
		if (drag_mode == DisplayView::DRAG_OVERLAY_ICON) {
			Filter	*current = graph.FindFilterByPos(point);
			if (current && current == overlay_filter) {
				int icon = current->CheckIcons(point);
				if (icon >= 0) {
					OnOverlayIconClick(current->overlay_icons[icon], point); 
				}
			}
		}

		if (drag_mode == DisplayView::DRAG_CONNECTION) {
			Pin *p1 = graph.FindPinByPos(new_connection_start);
			Pin *p2 = graph.FindPinByPos(new_connection_end);

			int ret = graph.ConnectPins(p1, p2);
			if (ret < -1) {
				DSUtil::ShowError(ret);
			}
		}
		new_connection_start = CPoint(-100,-100);
		new_connection_end = CPoint(-101, -101);
		drag_mode = DisplayView::DRAG_GROUP;
		ReleaseCapture();
		graph.Dirty();
		Invalidate();
	}

	void DisplayView::OnMouseMove(UINT nFlags, CPoint point)
	{
		point += GetScrollPosition();
		bool need_invalidate = false;

		// loop through the filters...
		if (nFlags & MK_LBUTTON) {


			switch (drag_mode) {
			case DisplayView::DRAG_GROUP:
				{
					// we are dragging now
					int	deltax = point.x - start_drag_point.x;
					int deltay = point.y - start_drag_point.y;

					// verify the deltas
					int i, selected_count = 0;
					for (i=0; i<graph.filters.GetCount(); i++) {
						Filter *filter = graph.filters[i];
						if (filter->selected) {
							selected_count ++;
							filter->VerifyDrag(&deltax, &deltay);
						}
					}

					// exit if there's no selected filter
					if (selected_count == 0) return ;

					// update their position
					for (i=0; i<graph.filters.GetCount(); i++) {
						Filter *filter = graph.filters[i];
						if (filter->selected) {
							int px = filter->start_drag_pos.x + deltax;
							int py = filter->start_drag_pos.y + deltay;

							// snap to grid
							px = (px+7)&~0x07;
							py = (py+7)&~0x07;

							if (px != filter->posx || py != filter->posy) {
								filter->posx = px;
								filter->posy = py;
								need_invalidate = true;
							}
						}
					}
				}
				break;

			case DisplayView::DRAG_CONNECTION:
				{
					new_connection_end = point;

					Filter	*current = graph.FindFilterByPos(point);
					if (current) {
						Pin *drop_end = current->FindPinByPos(point);
						if (drop_end) {
							drop_end->GetCenterPoint(&new_connection_end);
						}
					}

					need_invalidate = true;
				}
				break;
			case DisplayView::DRAG_SELECTION:
				{
					int	minx = start_drag_point.x;
					int miny = start_drag_point.y;
					int maxx = point.x;
					int maxy = point.y;

					if (minx > maxx) {
						minx = point.x;
						maxx = start_drag_point.x;
					}
					if (miny > maxy) {
						miny = point.y;
						maxy = start_drag_point.y;
					}

					end_drag_point = point;
					CRect	rc(minx, miny, maxx, maxy);

					for (int i=0; i<graph.filters.GetCount(); i++) {
						Filter *filter = graph.filters[i];

						CRect	rc2(filter->posx, filter->posy, 
									filter->posx+filter->width, 
									filter->posy+filter->height);
						CRect	rc3;						

						rc3.IntersectRect(&rc, &rc2);
						bool sel = (rc3.IsRectEmpty() ? false : true);

						if (sel != filter->selected) {
							filter->Select(sel);
							need_invalidate = true;
						}
					}

					if (!need_invalidate) {
						Invalidate();
					}
				}
				break;
			}

			if (need_invalidate) {
				graph.Dirty();
				Invalidate();
			}
		} else {

			/*
				No buttons are pressed. We only check for overlay icons
			*/

			Filter	*current = graph.FindFilterByPos(point);

			// if there was a filter active before
			if (overlay_filter) {
				// which was not ours
				if (overlay_filter != current) {
					// make it's overlay icon disappear
					overlay_filter->overlay_icon_active = -1;
					need_invalidate = true;
				}
			}

			overlay_filter = current;

			if (current) {		
				int	cur_icon = current->overlay_icon_active;

				int ret = current->CheckIcons(point);
				if (ret != cur_icon) {
					need_invalidate = true;
				}
			}

			if (need_invalidate) {
				graph.Dirty();
				Invalidate();
			}
		}
	}

	void DisplayView::OnSize(UINT nType, int cx, int cy)
	{
		UpdateScrolling();

		/*
		CRect	r;
		CDC		*dc = GetDC();
		GetClientRect(&r);

		if ((back_width != r.Width()) || (back_height != r.Height())) {

			if (memDC.GetSafeHdc()) {
				memDC.DeleteDC();
				backbuffer.DeleteObject();
			}

			memDC.CreateCompatibleDC(dc);
			backbuffer.CreateCompatibleBitmap(dc, r.Width(), r.Height());
			memDC.SelectObject(&backbuffer);
			back_width = r.Width();
			back_height = r.Height();

		}

		ReleaseDC(dc);
		*/
	}

	void DisplayView::RepaintBackbuffer()
	{
		CSize	size = graph.GetGraphSize();

		if (size.cx != back_width || size.cy != back_height) {

			CDC		*dc = GetDC();

			// we initialize a new backbuffer with the size of the graph
			if (memDC.GetSafeHdc()) {
				memDC.DeleteDC();
				backbuffer.DeleteObject();
			}

			memDC.CreateCompatibleDC(dc);
			backbuffer.CreateCompatibleBitmap(dc, size.cx, size.cy);
			memDC.SelectObject(&backbuffer);
			back_width = size.cx;
			back_height = size.cy;

			ReleaseDC(dc);

			graph.dirty = true;
		}

		if (graph.dirty) {
			CRect	rect;

			// now we repaint the buffer
			CBrush backBrush(render_params.color_back);

			memDC.SelectObject(&backBrush);
			memDC.GetClipBox(&rect);
			memDC.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);

			graph.Draw(&memDC);

			// not dirty anymore
			graph.dirty = false;
			UpdateScrolling();
		}
	}

	void DisplayView::OnDraw(CDC *pDC)
	{
		CRect	r;
		CRect	rect;
		GetClientRect(&r);

		// Set brush to desired background color
		RepaintBackbuffer();
		pDC->BitBlt(0, 0, back_width, back_height, &memDC, 0, 0, SRCCOPY);

		// paint the rest of client area with background brush
		CBrush backBrush(render_params.color_back);
		CBrush *prev_brush = pDC->SelectObject(&backBrush);
		pDC->PatBlt(back_width, 0, r.Width(), r.Height(), PATCOPY);
		pDC->PatBlt(0, back_height, back_width, r.Height(), PATCOPY);

		pDC->SelectObject(prev_brush);

		// draw arrow
		if (drag_mode == DisplayView::DRAG_CONNECTION) {
			graph.DrawArrow(pDC, new_connection_start, new_connection_end);
		} else
		if (drag_mode == DisplayView::DRAG_SELECTION) {

			// select a null (hollow) brush
			CBrush	*pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
			CPen	pen(PS_DOT, 1, RGB(0,0,0));
			CPen	*pOldPen = pDC->SelectObject(&pen);

			// set pen pixels as the inverse of the screen color.
			int nOldROP2 = pDC->SetROP2(R2_XORPEN);

			// draw new or erase old selection rectangle
			pDC->Rectangle(start_drag_point.x, start_drag_point.y,
						   end_drag_point.x, end_drag_point.y);
			
			pDC->SetROP2(nOldROP2);
			pDC->SelectObject(pOldBrush);
			pDC->SelectObject(pOldPen);
		}
	}

	void DisplayView::OnFileWriterStream()
	{
		if (!current_pin) return ;

		// now create an instance of this filter
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr;

		hr = CoCreateInstance(CLSID_FileWriter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr)) {
            DSUtil::ShowError(hr,_T("Can't create File Writer"));
			return ;
		} 
		
		if (SUCCEEDED(hr)){
			
			// now check for a few interfaces
			int ret = ConfigureInsertedFilter(instance, _T("File Writer"));
			if (ret < 0) {
				instance = NULL;
			}

			if (instance) {

				IPin		*outpin = current_pin->pin;
				outpin->AddRef();

				// add the filter to graph
				hr = graph.AddFilter(instance, _T("File Writer"));
				if (FAILED(hr)) {
					// display error message
				} else {
					// connect the pin to the renderer
					hr = DSUtil::ConnectPin(graph.gb, outpin, instance);

					graph.RefreshFilters();
					graph.SmartPlacement();
					graph.Dirty();
					Invalidate();
				}

				outpin->Release();
			}
		}
		instance = NULL;
		current_pin = NULL;
	}


	void DisplayView::OnDumpStream()
	{
		if (!current_pin) return ;

		// now create an instance of this filter
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr;

		hr = CoCreateInstance(DSUtil::CLSID_Dump, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr)) {
			// try our internal Dump Filter as an alternative
			CMonoDump	*dump = new CMonoDump(NULL, &hr);
			hr = dump->NonDelegatingQueryInterface(IID_IBaseFilter, (void**)&instance);
		} 
		
		if (SUCCEEDED(hr)){
			
			// now check for a few interfaces
			int ret = ConfigureInsertedFilter(instance, _T("Dump"));
			if (ret < 0) {
				instance = NULL;
			}

			if (instance) {

				IPin		*outpin = current_pin->pin;
				outpin->AddRef();

				// add the filter to graph
				hr = graph.AddFilter(instance, _T("Dump"));
				if (FAILED(hr)) {
					// display error message
                    DSUtil::ShowError(hr,_T("Can't add Dump Filter"));
				} else {
					// connect the pin to the renderer
					hr = DSUtil::ConnectPin(graph.gb, outpin, instance);

					graph.RefreshFilters();
					graph.SmartPlacement();
					graph.Dirty();
					Invalidate();
				}

				outpin->Release();
			}
		}
		instance = NULL;
		current_pin = NULL;
	}

    void DisplayView::OnTeeStream()
	{
		if (!current_pin) return ;

		// now create an instance of this filter
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr;

        hr = CoCreateInstance(CLSID_InfTee, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr)) {
			// display error message
            DSUtil::ShowError(hr,_T("Can't create Tee Filter"));
		} else {

			IPin		*outpin = current_pin->pin;
			outpin->AddRef();

			// add the filter to graph
			hr = graph.AddFilter(instance, _T("Tee Filter"));
			if (FAILED(hr)) {
				// display error message
                DSUtil::ShowError(hr,_T("Can't add Tee Filter"));
			} else {
				// connect the pin to the renderer
				hr = DSUtil::ConnectPin(graph.gb, outpin, instance);

				graph.RefreshFilters();
				graph.SmartPlacement();
				graph.Dirty();
				Invalidate();
			}

			outpin->Release();
		}
		instance = NULL;
		current_pin = NULL;
	}

	void DisplayView::OnRenderNullStream()
	{
		if (!current_pin) return ;

		// now create an instance of this filter
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr;

		hr = CoCreateInstance(DSUtil::CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr)) {
			// display error message
            DSUtil::ShowError(hr,_T("Can't create Null Renderer"));
		} else {
			
			// now check for a few interfaces
			int ret = ConfigureInsertedFilter(instance, _T("Null Renderer"));
			if (ret < 0) {
				instance = NULL;
			}

			if (instance) {

				IPin		*outpin = current_pin->pin;
				outpin->AddRef();

				// add the filter to graph
				hr = graph.AddFilter(instance, _T("Null Renderer"));
				if (FAILED(hr)) {
					// display error message
                    DSUtil::ShowError(hr,_T("Can't add Null Renderer"));
				} else {
					// connect the pin to the renderer
					hr = DSUtil::ConnectPin(graph.gb, outpin, instance);

					graph.RefreshFilters();
					graph.SmartPlacement();
					graph.Dirty();
					Invalidate();
				}

				outpin->Release();
			}
		}
		instance = NULL;
		current_pin = NULL;
	}

	void DisplayView::OnRenderPin()
	{
		if (!current_pin) return ;

		render_params.MarkRender(true);	
		HRESULT	hr = graph.gb->Render(current_pin->pin);
		render_params.MarkRender(false);
		OnRenderFinished();

		if (SUCCEEDED(hr)) {
			graph.RefreshFilters();
			graph.SmartPlacement();
			graph.Dirty();
			Invalidate();
		}
		current_pin = NULL;
	}

	void DisplayView::OnDeleteFilter()
	{
		// ask the derived class to do it ...
		OnDeleteSelection();
	}

	void DisplayView::OnPropertyPage()
	{
		CString	title;
		if (current_pin) {
			title = current_pin->name + _T(" Properties");
			OnDisplayPropertyPage(current_pin->pin, current_filter->filter, title);
			return ;
		}
		if (current_filter) {
			title = current_filter->name + _T(" Properties");
			OnDisplayPropertyPage(current_filter->filter, current_filter->filter, title);
			return ;
		}
	}

	void DisplayView::OnDeleteSelection()
	{
		// to be overriden
	}

	void DisplayView::OnFilterRemoved(DisplayGraph *sender, Filter *filter)
	{
		// to be overriden
	}

	void DisplayView::OnDisplayPropertyPage(IUnknown *object, IUnknown *filter, CString title)
	{
	}

	void DisplayView::OnOverlayIconClick(OverlayIcon *icon, CPoint point)
	{
	}

	void DisplayView::OnRenderFinished()
	{
	}

	// scrolling aid
	void DisplayView::UpdateScrolling()
	{
		CSize	size = graph.GetGraphSize();
		
		SetScrollSizes(MM_TEXT, size);

	}

	void DisplayView::MakeScreenshot()
	{
		// find out the rectangle
		int	minx = 10000000;
		int	miny = 10000000;
		int maxx = 0;
		int maxy = 0;

		for (int i=0; i<graph.filters.GetCount(); i++) {
			Filter	*filter = graph.filters[i];
			if (filter->posx < minx) minx = filter->posx;
			if (filter->posy < miny) miny = filter->posy;
			if (filter->posx + filter->width > maxx) maxx = filter->posx+filter->width;
			if (filter->posy + filter->height > maxy) maxy = filter->posy+filter->height;
		}

		minx = minx &~ 0x07; minx -= 8;	if (minx < 0) minx = 0;
		miny = miny &~ 0x07; miny -= 8;	if (miny < 0) miny = 0;
		maxx = (maxx+7) &~ 0x07; maxx += 8;
		maxy = (maxy+7) &~ 0x07; maxy += 8;

		// now copy the bitmap
		int	cx = (maxx-minx);
		int cy = (maxy-miny);

		if (cx == 0 || cy == 0) {
			OpenClipboard();
			EmptyClipboard();
			CloseClipboard();
			return ;
		}

		CRect		imgrect(minx, miny, maxx, maxy);
		CRect		bufrect(0, 0, back_width, back_height);
		CDC			tempdc;
		CBitmap		tempbitmap;

		CRect		area=imgrect;
		area.IntersectRect(&imgrect, &bufrect);

		tempdc.CreateCompatibleDC(&memDC);
		tempbitmap.CreateBitmap(area.Width(), area.Height(), 1, 32, NULL);
		CBitmap *old = tempdc.SelectObject(&tempbitmap);
		tempdc.BitBlt(0, 0, area.Width(), area.Height(), &memDC, area.left, area.top, SRCCOPY);

		OpenClipboard();
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, tempbitmap.GetSafeHandle());
		CloseClipboard();

        // ask for file
        CString	filter;
	    CString	filename;
	    filter = _T("PNG (*.png)|*.png|JPEG (*.jpg,*.jpeg)|*.jpg;*.jpeg|GIF (*.gif)|*.gif|TIFF (*.tiff,*.tif)|*.tiff;*.tif|Bitmap (*.bmp)|*.bmp|All Files (*.*)|*.*|");

	    CFileDialog dlg(FALSE,_T("png"),NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,filter);
        int ret = dlg.DoModal();

	    filename = dlg.GetPathName();
	    if (ret == IDOK)
        {
            GUID format = Gdiplus::ImageFormatPNG;
		    CPath path(filename);
		    if (path.GetExtension() == _T(""))
            {
			    path.AddExtension(_T(".png"));
			    filename = CString(path);
            }
            else
            {
                CString ext = path.GetExtension();
                ext.MakeLower();
                if(ext == _T(".jpg") || ext == _T(".jpeg"))
                    format = Gdiplus::ImageFormatJPEG;
                else if(ext == _T(".gif"))
                    format = Gdiplus::ImageFormatGIF;
                else if(ext == _T(".bmp"))
                    format = Gdiplus::ImageFormatBMP;
                else if(ext == _T(".tiff") || ext == _T(".tif"))
                    format = Gdiplus::ImageFormatTIFF;
            }

            CImage img;
            img.Attach(tempbitmap);
            img.Save(filename, format);
        }

        // free resources
		tempdc.SelectObject(old);
		tempbitmap.DeleteObject();
		tempdc.DeleteDC();

	}

	void DisplayView::OnSelectStream(UINT id)
	{
		CComPtr<IAMStreamSelect>	select;
		select = NULL;

		if (!current_filter) return ;
		if (current_pin && current_pin->pin) current_pin->pin->QueryInterface(IID_IAMStreamSelect, (void**)&select);
		if (!select) current_filter->filter->QueryInterface(IID_IAMStreamSelect, (void**)&select);
		if (!select) return ;

		id -= ID_STREAM_SELECT;

		// enable the stream
		select->Enable(id, AMSTREAMSELECTENABLE_ENABLE);

		select = NULL;
	}

	void DisplayView::OnCompatibleFilterClick(UINT id)
	{		
		id -= ID_COMPATIBLE_FILTER;

		// create an instance of the filter and insert it into the graph
		CComPtr<IBaseFilter>		instance;
		CComPtr<IPin>				outpin;
		HRESULT						hr;

		if (id >= compatible_filters.filters.GetCount()) return ;
		if (!current_pin) return ;
		if (!current_pin->pin) return ;

		outpin = current_pin->pin;

		DSUtil::FilterTemplate	&templ = compatible_filters.filters[id];
		hr = templ.CreateInstance(&instance);
		if (SUCCEEDED(hr)) {

			// now check for a few interfaces
            int ret = ConfigureInsertedFilter(instance, templ.name);
			if (ret < 0) {
				instance = NULL;
			}

			if (instance) {
				// add the filter to graph
				hr = graph.AddFilter(instance, templ.name);
				if (FAILED(hr)) {
					// display error message
				} else {

					// now try to connect the filter
					hr = DSUtil::ConnectPin(graph.gb, outpin, instance);

					graph.RefreshFilters();
					graph.SmartPlacement();
					Invalidate();
				}
			}
		}

		outpin = NULL;
		instance = NULL;
	}

	void DisplayView::PrepareCompatibleFiltersMenu(CMenu &menu, Pin *pin)
	{
		/*
			Enumerate output mediatypes and look for compatible
			filters.
		*/

		compatible_filters.filters.RemoveAll();

		// ignore invalid and connected pins
		if (!pin || !(pin->pin) || (pin->dir == PINDIR_INPUT)) return ;
		if (pin->connected) return ;

		// enumerate media types
		DSUtil::MediaTypes			mtypes;

		DSUtil::EnumMediaTypes(pin->pin, mtypes);
		if (mtypes.GetCount() <= 0) return ;

		// now try to enumerate compatible filters
		int ret = compatible_filters.EnumerateCompatible(mtypes, MERIT_UNLIKELY, false, render_params.exact_match_mode);
		if ((ret == 0) && (compatible_filters.filters.GetCount() > 0)) {

			CMenu		submenu;
			submenu.CreatePopupMenu();
			CMenu		&active_menu = submenu;

			for (int i=0; i<compatible_filters.filters.GetCount(); i++) {
				DSUtil::FilterTemplate	&filt = compatible_filters.filters[i];

				CString		merit;
				merit.Format(_T("%08x)"), filt.merit);
				merit = merit.MakeUpper();

				int idx = active_menu.GetMenuItemCount();

				CString		name = filt.name + _T("\t(0x") + merit;
				active_menu.InsertMenu(idx, MF_BYPOSITION | MF_STRING, ID_COMPATIBLE_FILTER + i, name);
			}			

			// do insert the menu
			int		count = menu.GetMenuItemCount();
			menu.InsertMenu(count++, MF_BYPOSITION | MF_SEPARATOR);
			menu.InsertMenu(count, MF_BYPOSITION | MF_STRING, 0, _T("Compatible filters"));
			menu.ModifyMenu(count, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)submenu.m_hMenu, _T("Compatible filters"));
			submenu.Detach();
		}
	}

	void DisplayView::PrepareStreamSelectMenu(CMenu &menu, IUnknown *obj)
	{
		// support for IAMStreamSelect
		CComPtr<IAMStreamSelect>	stream_select;

		HRESULT		hr = obj->QueryInterface(IID_IAMStreamSelect, (void**)&stream_select);
		if (FAILED(hr) || !stream_select) return ;
			
		// now scan through the streams
		DWORD		last_group = (DWORD)-1;
		DWORD		stream_count = 0;
		hr = stream_select->Count(&stream_count);
		if (FAILED(hr)) {
			stream_select = NULL;
			return ;
		}

		// nothing to add - we're done here
		if (stream_count <= 0) {
			stream_select = NULL;
			return ;
		}

		CMenu	submenu;
		submenu.CreatePopupMenu();
		CMenu	&active_menu = submenu;


			menu.InsertMenu(menu.GetMenuItemCount(), MF_BYPOSITION | MF_SEPARATOR);
			for (DWORD i=0; i<stream_count; i++) {

				AM_MEDIA_TYPE		*mt = NULL;
				DWORD				flags;
				LCID				language;
				DWORD				group;
				LPWSTR				name;

				hr = stream_select->Info(i, &mt, &flags, &language, &group, &name, NULL, NULL);
				if (SUCCEEDED(hr)) {
				
					// add separators between stream groups
					if (i == 0) {
						last_group = group;
					} else {
						if (group != last_group) {
							active_menu.InsertMenu(active_menu.GetMenuItemCount(), MF_BYPOSITION | MF_SEPARATOR);
							last_group = group;
						}
					}

					int idx = active_menu.GetMenuItemCount();
					UINT	mflags = MF_BYPOSITION | MF_STRING;
					if (flags > 0) mflags |= MF_CHECKED;
					active_menu.InsertMenu(idx, mflags | MF_STRING, ID_STREAM_SELECT+i, name);
				}

				// get rid of the buffers
				if (mt) {
					DeleteMediaType(mt);
				}
			}

		// do insert the menu
		int		count = menu.GetMenuItemCount();
		menu.InsertMenu(count, MF_BYPOSITION | MF_STRING, 0, _T("Stream selection"));
		menu.ModifyMenu(count, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)submenu.m_hMenu, _T("Stream selection"));
		submenu.Detach();

		stream_select = NULL;
	}



};





