//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

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
		ON_WM_MBUTTONDOWN()
		ON_WM_RBUTTONDOWN()
		ON_WM_LBUTTONUP()

		ON_COMMAND(ID_PIN_RENDER, &DisplayView::OnRenderPin)
        ON_COMMAND(ID_PIN_REMOVE, &DisplayView::OnRemovePin)
		ON_COMMAND(ID_PIN_NULL_STREAM, &DisplayView::OnRenderNullStream)
		ON_COMMAND(ID_PIN_DUMP_STREAM, &DisplayView::OnDumpStream)
		ON_COMMAND(ID_PIN_TIME_MEASURE_STREAM, &DisplayView::OnTimeMeasureStream)
		ON_COMMAND(ID_PIN_ANALYZE_STREAM, &DisplayView::OnAnalyzeStream)
        ON_COMMAND(ID_PIN_ANALYZE_WRITER_STREAM, &DisplayView::OnAnalyzeWriterStream)
        ON_COMMAND(ID_PIN_TEE_STREAM, &DisplayView::OnTeeStream)
		ON_COMMAND(ID_PIN_FILE_WRITER, &DisplayView::OnFileWriterStream)
		ON_COMMAND(ID_PROPERTYPAGE, &DisplayView::OnPropertyPage)
		ON_COMMAND(ID_DELETE_FILTER, &DisplayView::OnDeleteFilter)
        ON_COMMAND(ID_MPEG2DEMUX_CREATE_PSI_PIN, &DisplayView::OnMpeg2DemuxCreatePsiPin)
        ON_COMMAND(ID_CHOOSE_SOURCE_FILE, &DisplayView::OnChooseSourceFile)
        ON_COMMAND(ID_CHOOSE_DESTINATION_FILE, &DisplayView::OnChooseDestinationFile)
        ON_COMMAND(ID_FILTER_FAVORITE, &DisplayView::OnFilterFavorite)
        ON_COMMAND(ID_FILTER_BLACKLIST, &DisplayView::OnFilterBlacklist)

		ON_COMMAND_RANGE(ID_STREAM_SELECT, ID_STREAM_SELECT+100, &DisplayView::OnSelectStream)
		ON_COMMAND_RANGE(ID_COMPATIBLE_FILTER, ID_COMPATIBLE_FILTER+999, &DisplayView::OnCompatibleFilterClick)
        //ON_COMMAND_RANGE(ID_FAVORITE_FILTER, ID_FAVORITE_FILTER+500, &DisplayView::OnFavoriteFilterClick)

		ON_COMMAND(ID_FILE_SETLOGFILE, &DisplayView::OnFileSetlogfile)
		ON_UPDATE_COMMAND_UI(ID_FILE_SETLOGFILE, &DisplayView::OnUpdateFileSetlogfile)
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

        current_pin = NULL;
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
		if (!menu.CreatePopupMenu()) return ;

		FILTER_STATE	state = State_Running;
		if (graph.GetState(state, 0) != 0) {
			state = State_Running;
		}

		// find out if there is any filter being selected
		current_filter = graph.FindFilterByPos(point);

		// check for a pin - will have different menu
		current_pin = current_filter ? current_filter->FindPinByPos(point, false) : NULL;

		if (!current_pin)
			current_pin = GetPinFromFilterClick(current_filter, nFlags, /* findConnectedPin = */ false);

		// make rendering inactive for connected pins
		UINT	renderFlags = 0;
		if (current_pin) {
			if (current_pin->connected) renderFlags |= MF_GRAYED;
			if (current_pin->dir != PINDIR_OUTPUT) renderFlags |= MF_GRAYED;
		}
		if (state != State_Stopped) renderFlags |= MF_GRAYED;

		/*
			If the pin is not connected we might try to
			check it for MEDIATYPE_Stream so we can offer to connect
			the File Writer filter.
		*/

		bool	offer_writer = !current_filter;		// offer writer filters if no filter selected
        bool    offer_remove = false;

		if (current_pin && !current_pin->connected) {
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

            // Check if we can remove
            CComQIPtr<IMpeg2Demultiplexer> pMpeg2Demux = current_pin->filter->filter;
            offer_remove = pMpeg2Demux && current_pin->dir == PINDIR_OUTPUT;
		}

		int p = 0;

		////////////////////////// Filter and Pin operations

		if (current_filter || current_pin) {
			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING, ID_PROPERTYPAGE, _T("&Properties..."));
		}

		/////////////////////// Pin operations

		if (current_pin && !current_pin->connected) {
			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | renderFlags, ID_PIN_RENDER, _T("&Render Pin"));
			if (offer_remove)
				menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | renderFlags, ID_PIN_REMOVE, _T("Remo&ve Pin"));
		}

		/////////////////////// Filter operations

		if (current_filter && !current_pin) {

			const CComQIPtr<IFileSourceFilter> file_source(current_filter->filter);
			if (file_source) {
				menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING, ID_CHOOSE_SOURCE_FILE, _T("Choose &Source File..."));
			}

			const CComQIPtr<IFileSinkFilter> file_sink(current_filter->filter);
			if (file_sink) {
				menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING, ID_CHOOSE_DESTINATION_FILE, _T("Choose &Destination File..."));
			}

			CComQIPtr<IMpeg2Demultiplexer> mp2demux = current_filter->filter;
			if(mp2demux) {
				menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING, ID_MPEG2DEMUX_CREATE_PSI_PIN, _T("&Create PSI Pin"));
			}

			// check for IAMStreamSelect interface
			PrepareStreamSelectMenu(menu, current_filter->filter);
			p = menu.GetMenuItemCount();

			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING, ID_DELETE_FILTER, _T("D&elete Selected"));

			bool favorite = false;
			bool blacklisted = false;

			DSUtil::FilterTemplate filter_template;
			if (CFiltersForm::FilterTemplateFromCLSID(current_filter->clsid, filter_template)) {
				favorite = CFavoritesForm::GetFavoriteFilters()->ContainsMoniker(filter_template.moniker_name);
				blacklisted = CFavoritesForm::GetBlacklistedFilters()->ContainsMoniker(filter_template.moniker_name);
			}

			menu.InsertMenu(p++, (favorite ? MF_CHECKED : MF_UNCHECKED) | MF_DISABLED | MF_BYPOSITION | MF_STRING, ID_FILTER_FAVORITE, _T("&Favorite"));
			menu.InsertMenu(p++, (blacklisted ? MF_CHECKED : MF_UNCHECKED) | MF_DISABLED | MF_BYPOSITION | MF_STRING, ID_FILTER_BLACKLIST, _T("&Blacklist"));
		}

		/////////////////////// Inserting new filters

		// Offer to insert new filters unless a filter is selected without pin
		if (!current_filter || current_pin) {

			if (p > 0)
				menu.InsertMenu(p++, MF_BYPOSITION | MF_SEPARATOR);

			// add Favorite filters
			PrepareFavoriteFiltersMenu(menu);
			p = menu.GetMenuItemCount();

			if (current_pin) {
				// check for compatible filters
				PrepareCompatibleFiltersMenu(menu, current_pin);
				// check for IAMStreamSelect interface
				PrepareStreamSelectMenu(menu, current_pin->pin);
				p = menu.GetMenuItemCount();
				menu.InsertMenu(p++, MF_BYPOSITION | MF_SEPARATOR);
			}

			if (!current_pin || !current_pin->connected) {
				menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | renderFlags, ID_PIN_NULL_STREAM, _T("Insert &Null Renderer"));
				menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | renderFlags, ID_PIN_DUMP_STREAM, _T("Insert D&ump Filter"));
				menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | renderFlags, ID_PIN_ANALYZE_WRITER_STREAM, _T("Insert Analyzer &Writer Filter"));
			}

			if (offer_writer) {
				menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | renderFlags, ID_PIN_FILE_WRITER, _T("Insert &File Writer"));
			}

			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | renderFlags, ID_PIN_TEE_STREAM, _T("Insert &Tee Filter"));
			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | renderFlags, ID_PIN_TIME_MEASURE_STREAM, _T("Insert Time &Measure Filter"));
			menu.InsertMenu(p++, MF_BYPOSITION | MF_STRING | renderFlags, ID_PIN_ANALYZE_STREAM, _T("Insert &Analyzer Filter"));
		}

		CPoint	pt;
		GetCursorPos(&pt);

		// display menu
		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, AfxGetMainWnd());
	}

	// Select the single thing we've clicked on - pin/filter/connection - and delete it
	void DisplayView::OnMButtonDown(UINT nFlags, CPoint point)
	{
		point += GetScrollPosition();

		current_pin = NULL;
		current_filter = NULL;

		// deselect all filters but select any connections we've clicked on
		for (int i=0; i<graph.filters.GetCount(); i++) {
			if (graph.filters[i]->selected) {
				graph.filters[i]->Select(false);
			}
			graph.filters[i]->SelectConnection(nFlags, point);
		}

		// find out if there is any filter being selected
		Filter * const filter = graph.FindFilterByPos(point);

		// check for a pin - will have different menu
		Pin * pin = filter ? filter->FindPinByPos(point, false) : NULL;

		if (filter && !pin) {
			pin = GetPinFromFilterClick(filter, nFlags, /* findConnectedPin = */ true);
			if (!pin && (nFlags & (MK_SHIFT | MK_CONTROL) )) {
				// Used pressed shift or control but no pin found so return to prevent unexpected destructive side effects
				return;			
			}
		}

		if (pin) {
			if (pin->connected) {
				pin->Select(true);
			}
		} else if (filter) {
			filter->Select(true);
		}

		OnDeleteSelection();
	}

	Pin * DisplayView::GetPinFromFilterClick(Filter* filter, int clickFlags, bool findConnectedPin)
	{
		if (!filter)
			return NULL;

		Pin* hitpin = NULL;
		const UINT keyFlags = clickFlags & (MK_SHIFT | MK_CONTROL);
		CArray<Pin*> * pins = NULL;

		// Choose while Pin list we're using
		switch (keyFlags) {
		case MK_SHIFT:
			pins = &filter->output_pins;
			break;

			break;
		case MK_SHIFT | MK_CONTROL: {
			pins = &filter->input_pins;
			break;
			}
		}

		// If we have a list, find the first matching Pin or if that fails just the first pin
		if (pins) {
			for (int i=0; i<pins->GetCount(); i++) {
				Pin * const pin = (*pins)[i];
				if (pin->connected == findConnectedPin) {
					hitpin = pin;
					break;
				}
			}
		}
		return hitpin;
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
		Pin *hitpin = current->FindPinByPos(point, false);	// allow connected pins

		if (!hitpin)
			hitpin = GetPinFromFilterClick(current, nFlags, /* findConnectedPin = */ false);

		if (hitpin) {
			// deselect all filters
			for (int i=0; i<graph.filters.GetCount(); i++) {
				graph.filters[i]->Select(false);
			}

			// remember the start point
			hitpin->GetCenterPoint(&new_connection_start);
			new_connection_end = new_connection_start;
			new_connection_start_connected = hitpin->connected;
            new_connection_start_type = hitpin->connectionType;
			new_connection_end_connected = false;
			drag_mode = DisplayView::DRAG_CONNECTION;

		} else {

			int icon = current->CheckIcons(point);
			if (icon >= 0) {
				drag_mode = DisplayView::DRAG_OVERLAY_ICON;
				return ;
			}

			if (current->selected) {
				if (nFlags & MK_CONTROL) {
					current->Select(false);
					graph.Dirty();
					Invalidate();
				} else {
					// nothing here...
				}
			} else {
				if (nFlags & MK_CONTROL) {
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

		const bool chooseMediaType = (nFlags&MK_SHIFT) != 0;		// only applies in direct connection mode

		if (drag_mode == DisplayView::DRAG_CONNECTION) {
			Filter * const f1 = graph.FindFilterByPos(new_connection_start);
			Pin * const p1 = f1 ? f1->FindPinByPos(new_connection_start, false) : NULL;

			Filter * const f2 = graph.FindFilterByPos(new_connection_end);
			Pin * p2 = f2 ? f2->FindPinByPos(new_connection_end, false) : NULL;

            if(p1 != NULL)
            {
                if(p2 != NULL)
                {
                    if(p1 == p2)
						;			// clean up below
                    else if(p1->filter == p2->filter)
                    {
                        DSUtil::ShowInfo(_T("Can't connect pins on the same filter."));
                    }
                    else if(p1->dir == p2->dir)
                    {
                        DSUtil::ShowInfo(_T("Can't connect pins with the same direction."));
                    }
                    else
                    {
						if (p1->connected)
							p1->Disconnect();
						if (p2->connected)
							p2->Disconnect();

						if (!p1->connected && !p2->connected)	// check in case disconnection above failed
							HRESULT hr = graph.ConnectPins(p1, p2, chooseMediaType);
                    }
                }
                else
                {
                    if(f2 != NULL)
                    {
                        bool nopins = true;
                        bool nofreepins = true;
                        if(p1->dir == PINDIR_OUTPUT)
                        {
                            for (int i=0; i<f2->input_pins.GetCount(); i++)
                            {
                                nopins = false;
                                if(!f2->input_pins[i]->connected)
                                {
                                    nofreepins = false;
                                    p2 = f2->input_pins[i];
                                    break;
                                }
                            }
                        }
                        else
                        {
                            for (int i=0; i<f2->output_pins.GetCount(); i++)
                            {
                                nopins=false;
                                if(!f2->output_pins[i]->connected)
                                {
                                    nofreepins = false;
                                    p2 = f2->output_pins[i];
                                    break;
                                }
                            }
                        }

                        if(nopins)
                        {
                            if(p1->dir == PINDIR_INPUT)
                                DSUtil::ShowInfo(_T("No output pins on the filter to connect to."));
                            else
                                DSUtil::ShowInfo(_T("No input pins on the filter to connect to."));
                        }
                        else if(nofreepins)
                        {
                            DSUtil::ShowInfo(_T("No free pins on the filter to connect to."));
                        }
                        else
                        {
                            HRESULT hr = graph.ConnectPins(p1, p2, chooseMediaType);
		                }
                    }
                }
            }
		}
		new_connection_start = CPoint(-100,-100);
		new_connection_end = CPoint(-101, -101);
		new_connection_start_connected = false;
        new_connection_start_type = Pin::PIN_CONNECTION_TYPE_OTHER;
		new_connection_end_connected = false;
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
							const int px = DisplayGraph::NextGridPos(filter->start_drag_pos.x + deltax);
							const int py = DisplayGraph::NextGridPos(filter->start_drag_pos.y + deltay);

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
					new_connection_end_connected = false;

					Filter	* const current = graph.FindFilterByPos(point);
					if (current) {
						Pin * drop_end = current->FindPinByPos(point, false);	// allow connected pins
						if (!drop_end) {
							Filter * const filter_start = graph.FindFilterByPos(new_connection_start);
                            Pin * const drop_start = filter_start ? 
									filter_start->FindPinByPos(new_connection_start, false)	// allow connected pins
									: NULL;
                            if(drop_start->dir == PINDIR_OUTPUT) {
                                for(int i=0; i<current->input_pins.GetSize(); i++)
                                    if(!current->input_pins[i]->IsConnected()) {
                                        drop_end = current->input_pins[i];
                                        break;
                                    }
                            } else {
                                for(int i=0; i<current->output_pins.GetSize(); i++)
                                    if(!current->output_pins[i]->IsConnected()) {
                                        drop_end = current->output_pins[i];
                                        break;
                                    }
                            }
                        }

                        if (drop_end) {
                            Filter * const start_filter = graph.FindFilterByPos(new_connection_start);
							if (start_filter != current) {			// don't allow a filter to connect with itself
								drop_end->GetCenterPoint(&new_connection_end);
								new_connection_end_connected = drop_end->connected;
							}
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
            current_pin = NULL;

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
			CBrush backBrush(!render_params.is_remote ? render_params.color_back : render_params.color_back_remote);

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
		CBrush backBrush(!render_params.is_remote ? render_params.color_back : render_params.color_back_remote);
		CBrush *prev_brush = pDC->SelectObject(&backBrush);
		pDC->PatBlt(back_width, 0, r.Width(), r.Height(), PATCOPY);
		pDC->PatBlt(0, back_height, back_width, r.Height(), PATCOPY);

		pDC->SelectObject(prev_brush);

		// draw arrow
		if (drag_mode == DisplayView::DRAG_CONNECTION) {
			// Draw new connections that require disconnections in red rather than connection color as a visual cue
			DWORD color = RenderParameters::color_connection_type[new_connection_start_type]; 
            if (new_connection_start_connected || new_connection_end_connected)
                color = RenderParameters::color_connection_break;

            const int nPenStyle = PS_DOT;
			graph.DrawArrow(pDC, new_connection_start, new_connection_end, color, nPenStyle);
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
		CComPtr<IBaseFilter>	instance;
		HRESULT hr = CoCreateInstance(CLSID_FileWriter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr))
            DSUtil::ShowError(hr,_T("Can't create File Writer"));
		else
			hr = InsertNewFilter(instance, _T("File Writer"));
	}

	void DisplayView::OnTimeMeasureStream()
	{
		CComPtr<IBaseFilter>	instance;
		HRESULT hr = S_OK;
		CMonoTimeMeasure * const filter = new CMonoTimeMeasure(NULL, &hr);
		hr = filter->NonDelegatingQueryInterface(IID_IBaseFilter, (void**)&instance);
		if (SUCCEEDED(hr))
			hr = InsertNewFilter(instance, _T("Time Measure"));
	}

	void DisplayView::OnAnalyzeStream()
	{
		CComPtr<IBaseFilter>	instance;
		HRESULT hr = S_OK;
		CAnalyzerFilter * const filter = new CAnalyzerFilter(NULL, &hr);
		hr = filter->NonDelegatingQueryInterface(IID_IBaseFilter, (void**)&instance);
		if (SUCCEEDED(hr))
			hr = InsertNewFilter(filter, _T("Analyzer"));
	}

    void DisplayView::OnAnalyzeWriterStream()
	{
		CComPtr<IBaseFilter>	instance;
		HRESULT hr = S_OK;
		CAnalyzerWriterFilter * const filter = new CAnalyzerWriterFilter(NULL, &hr);
		hr = filter->NonDelegatingQueryInterface(IID_IBaseFilter, (void**)&instance);
		if (SUCCEEDED(hr))
			hr = InsertNewFilter(filter, _T("Analyzer"));
	}

	void DisplayView::OnDumpStream()
	{
		CComPtr<IBaseFilter>	instance;
		HRESULT hr = CoCreateInstance(DSUtil::CLSID_Dump, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr)) {
			// try our internal Dump Filter as an alternative
			CMonoDump * const dump = new CMonoDump(NULL, &hr);
			hr = dump->NonDelegatingQueryInterface(IID_IBaseFilter, (void**)&instance);
		} 
		
		if (SUCCEEDED(hr))
			hr = InsertNewFilter(instance, _T("Dump"));
	}

    void DisplayView::OnTeeStream()
	{
		CComPtr<IBaseFilter>	instance;
        HRESULT hr = CoCreateInstance(CLSID_InfTee, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr)) {
            DSUtil::ShowError(hr,_T("Can't create Tee Filter"));
		} else {
			hr = InsertNewFilter(instance, _T("Tee Filter"));
		}
	}

	void DisplayView::OnRenderNullStream()
	{
		CComPtr<IBaseFilter>	instance;
		HRESULT hr = CoCreateInstance(DSUtil::CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr)) {
            DSUtil::ShowError(hr,_T("Can't create Null Renderer"));
		} else {
			hr = InsertNewFilter(instance, _T("Null Renderer"));
		}
	}

	HRESULT DisplayView::InsertNewFilter(IBaseFilter* newFilter, const CString& filterName, bool connectToCurrentPin /*= true */ )
	{
		HRESULT hr = S_OK;

		if (!newFilter)
			return S_OK;			// Nothing to do so finish now

		// now check for a few interfaces
		const int ret = ConfigureInsertedFilter(newFilter, filterName);
		if (ret < 0) {
			hr = E_FAIL;
			if (newFilter) {
				newFilter = NULL;
			}
		}

		// has selected pin, then get IPin interface now, because graph.AddFilter will release current_pin
		CComPtr<IPin> outpin(connectToCurrentPin && current_pin ? current_pin->pin : NULL);
		hr = graph.AddFilter(newFilter, filterName);
		if (FAILED(hr)) {
			DSUtil::ShowError(hr, CString(_T("Can't add ")) + filterName);
		} else {
			if (outpin) {
				// Get previously connected IPin and media type and disconnect it
				CComPtr<IPin> previousPin;
				CMediaType previousMediaType;
				hr = outpin->ConnectedTo(&previousPin);
				if (previousPin) {
					hr = outpin->ConnectionMediaType(&previousMediaType);
					hr = graph.gb->Disconnect(outpin);
					hr = graph.gb->Disconnect(previousPin);
				}

				// connect new filter to currently selected pin
				hr = DSUtil::ConnectPinToFilter(graph.gb, outpin, newFilter, 
						graph.params->connect_mode != 0, graph.params->connect_mode == 2);	
				if (FAILED(hr))
					DSUtil::ShowError(hr, CString(_T("Can't connect ")) + filterName);
				else {
					if (previousPin) {
						hr = DSUtil::ConnectPinToFilter(graph.gb, previousPin, newFilter, 
								true, false, &previousMediaType);
					}
				}
			}
			graph.RefreshFilters();
			graph.SmartPlacement();
			graph.Dirty();
			Invalidate();
		}
		outpin = NULL;
		current_pin = NULL;

		return hr;
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
		} else {
            DSUtil::ShowError(hr,_T("Can't render pin"));
		}
		current_pin = NULL;
	}

    void DisplayView::OnRemovePin()
	{
		if (!current_pin) return;

        CComQIPtr<IMPEG2PIDMap> pPIDMap = current_pin->pin;
        CComQIPtr<IMpeg2Demultiplexer> pMpeg2Demux = current_pin->filter->filter;

        if (pPIDMap && pMpeg2Demux)
        {
            CT2W pinName(current_pin->name);
            HRESULT hr = pMpeg2Demux->DeleteOutputPin(pinName);

		    if (SUCCEEDED(hr)) {
			    graph.RefreshFilters();
			    graph.SmartPlacement();
			    graph.Dirty();
			    Invalidate();
			} else {
				DSUtil::ShowError(hr,_T("Can't remove pin"));
		    }
        }
		current_pin = NULL;
	}

	void DisplayView::OnDeleteFilter()
	{
		// ask the derived class to do it ...
		OnDeleteSelection();
	}

    void DisplayView::OnMpeg2DemuxCreatePsiPin()
    {
        // to be overriden
    }

	void DisplayView::OnChooseSourceFile()
	{
		if (current_filter && current_filter->filter) {
			CComQIPtr<IFileSourceFilter> source(current_filter->filter);
			if (source) {
				CFileSrcForm form(current_filter->display_name);
				HRESULT hr = form.ChooseSourceFile(source);
				graph.RefreshFilters();
				graph.SmartPlacement();
				graph.Dirty();
				Invalidate();
			}
		}
	}

	void DisplayView::OnChooseDestinationFile()
	{
		if (current_filter && current_filter->filter) {
			CComQIPtr<IFileSinkFilter> sink(current_filter->filter);
			if (sink) {
				CFileSinkForm form(current_filter->display_name);
				HRESULT hr = form.ChooseSinkFile(sink);
				graph.RefreshFilters();
				graph.SmartPlacement();
				graph.Dirty();
				Invalidate();
			}
		}
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

	void DisplayView::MakeScreenshot(const CString& base_filename)
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

		// Round outwards and add an extra grid of border
		minx = DisplayGraph::PrevGridPos(minx) - DisplayGraph::GRID_SIZE;
		minx = max(minx, 0);

		miny = DisplayGraph::PrevGridPos(miny) - DisplayGraph::GRID_SIZE;
		miny = max(miny, 0);

		maxx = DisplayGraph::NextGridPos(maxx) + DisplayGraph::GRID_SIZE;
		maxy = DisplayGraph::NextGridPos(maxy) + DisplayGraph::GRID_SIZE;

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

		CPath input_path(base_filename);
		input_path.RemoveExtension();
		CString input_filename = CString(input_path);
	
		dlg.m_ofn.lpstrFile = input_filename.GetBufferSetLength(MAX_PATH + 1);
		dlg.m_ofn.nMaxFile = MAX_PATH + 1;

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
		if (id >= compatible_filters.filters.GetCount()) 
			return ;

		// create an instance of the filter and insert it into the graph
		CComPtr<IBaseFilter>		instance;
		DSUtil::FilterTemplate	&templ = compatible_filters.filters[id];
		HRESULT hr = templ.CreateInstance(&instance);
		if (SUCCEEDED(hr)) {
			hr = InsertNewFilter(instance, templ.name);
		}
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

				const CString name = _T("&") + filt.name + _T("\t(0x") + merit;
				active_menu.InsertMenu(idx, MF_BYPOSITION | MF_STRING, ID_COMPATIBLE_FILTER + i, name);
			}			

			// do insert the menu
			int		count = menu.GetMenuItemCount();
			menu.InsertMenu(count, MF_BYPOSITION | MF_STRING, 0, _T("&Compatible filters"));
			menu.ModifyMenu(count, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)submenu.m_hMenu, _T("&Compatible filters"));
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
					active_menu.InsertMenu(idx, mflags | MF_STRING, ID_STREAM_SELECT+i, CString(_T("&")) + name);
				}

				// get rid of the buffers
				if (mt) {
					DeleteMediaType(mt);
				}
			}

		// do insert the menu
		int		count = menu.GetMenuItemCount();
		menu.InsertMenu(count, MF_BYPOSITION | MF_STRING, 0, _T("&Stream selection"));
		menu.ModifyMenu(count, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)submenu.m_hMenu, _T("&Stream selection"));
		submenu.Detach();

		stream_select = NULL;
	}

	void DisplayView::UpdateFavoritesMenu()
	{
		// first erase all added items
		CMenu * const main_menu		= GetParentFrame()->GetMenu();
		CMenu * const filters_menu	= main_menu ? main_menu->GetSubMenu(4) : NULL;
		ASSERT(filters_menu);

		if (filters_menu) {
			// remove all items from menu apart from two at end
			while (filters_menu->GetMenuItemCount() > 2) {
				filters_menu->DeleteMenu(0, MF_BYPOSITION);
			}

			GraphStudio::BookmarkedFilters	* const favorites = CFavoritesForm::GetFavoriteFilters();
			favorites->Sort();

			const int	cnt = favorites->filters.GetCount() + favorites->groups.GetCount();

			if (cnt > 0) {
				const int	offset = filters_menu->GetMenuItemCount() - 2;
				const int c = CFavoritesForm::FillMenu(filters_menu, favorites, offset);

				// separator at the end
				filters_menu->InsertMenu(offset + c, MF_BYPOSITION | MF_SEPARATOR, 0);
			}
		}
	}

	void DisplayView::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
	{
		if (pPopupMenu) {
			const int menu_count = pPopupMenu->GetMenuItemCount();
			if (menu_count > 0) {
				if (pPopupMenu->GetMenuItemID(menu_count - 1) == ID_FILTERS_MANAGEBLACKLIST) {	// if this is this the favorites menu
					UpdateFavoritesMenu();
				}
			}
		}
		// do NOT call base class here as this was called from CMainFrame rather than called through message map
	}

	void DisplayView::PrepareFavoriteFiltersMenu(CMenu &menu)
    {
		CMenu		submenu;
		submenu.CreatePopupMenu();
        GraphStudio::BookmarkedFilters	*favorites = CFavoritesForm::GetFavoriteFilters();
        
		UpdateFavoritesMenu();		// update favorites menu as the command handlers use it directly
        CFavoritesForm::FillMenu(&submenu, favorites);

		// do insert the menu
		const int count = menu.GetMenuItemCount();
		menu.InsertMenu(count, MF_BYPOSITION | MF_STRING, 0, _T("Fa&vorite filters"));
		menu.ModifyMenu(count, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)submenu.m_hMenu, _T("Fa&vorite filters"));
		submenu.Detach();
	}

    ULONG DisplayView::GetGestureStatus(CPoint ptTouch)
    {
        return 0;
    }

	// Toggle the bookmark status of this Filter class
	static void ToggleFilterClassBookmark(BookmarkedFilters* bookmarks, const Filter* filter)
	{
		if (bookmarks && filter) {
			DSUtil::FilterTemplate filter_template;
			if (CFiltersForm::FilterTemplateFromCLSID(filter->clsid, filter_template)) {
				if (bookmarks->ContainsMoniker(filter_template.moniker_name)) {
					bookmarks->RemoveBookmark(filter_template);
				} else {
					bookmarks->AddBookmark(filter_template);
				}
				bookmarks->Save();
			}
		}
	}

	void DisplayView::OnFilterFavorite()
	{
		ToggleFilterClassBookmark(CFavoritesForm::GetFavoriteFilters(), current_filter);
	}

	void DisplayView::OnFilterBlacklist()
	{
		ToggleFilterClassBookmark(CFavoritesForm::GetBlacklistedFilters(), current_filter);
	}

	void DisplayView::OnFileSetlogfile()
	{
		HRESULT hr = S_OK;
		if (graph.IsLogFileOpen()) {
			hr = graph.CloseLogFile();
		} else {
			CFileDialog dlg(FALSE, _T("txt"), _T("GraphStudioNextLog.txt"), OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,
					_T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*|"), this);

			if (IDOK == dlg.DoModal()) {
				hr = graph.OpenLogFile(dlg.GetPathName());
			}
		}
		DSUtil::ShowError(hr, _T("Set Log File"));
	}

	void DisplayView::OnUpdateFileSetlogfile(CCmdUI *pCmdUI)
	{
		pCmdUI->Enable(!graph.is_remote);
		pCmdUI->SetCheck(!graph.is_remote && graph.IsLogFileOpen());
	}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation
