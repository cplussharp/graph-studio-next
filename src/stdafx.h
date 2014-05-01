//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

//#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS // Remove MFC Controls from Static MFC lib

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0601		// Change this to the appropriate value to target other versions of Windows.
#endif  // 0x0501

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0601	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#pragma warning( disable : 4995 4996 )		// TODO fix deprecated function references

// Define this here so that we can add a counter definition in cpp.hint
// to allow Class Wizard to work with classes in GraphStudio namespace
// cf https://connect.microsoft.com/VisualStudio/feedback/details/543019/class-wizard-and-class-view-does-not-detect-namespaces-changes-in-configurations

#define GRAPHSTUDIO_NAMESPACE_START		namespace GraphStudio { 
#define GRAPHSTUDIO_NAMESPACE_END		}


// Find memmory leaks
//#define _CRTDBG_MAP_ALLOC

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <atlbase.h>
#include <atlwin.h>
#include <atlpath.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>
#include <afxlinkctrl.h>
#include <afxtooltipctrl.h>
#include <afxglobals.h>
#include <afxadv.h> // for jumplist
//#include <afxlistctrl.h>    // CMFCListCtrl

#include <streams.h>
#include <initguid.h>
#include <qnetwork.h>
#include <dvdmedia.h>
#include <dmodshow.h>
#include <medparam.h>
#include <mmreg.h>
#include <ks.h>
#include <ksproxy.h>
#include <ksmedia.h>
#include <mfidl.h>
#include <d3d9.h>
#include <evr.h>
#include <evr9.h>
#include <mfapi.h>      // API Media Foundation
#include <Mferror.h>
#include <dmo.h>
#include <Sbe.h>
#include <wmcodecdsp.h>
#pragma comment(lib, "wmcodecdspuuid.lib")
#include <propsys.h>
#include "comdef.h"
#include <Bdaiface.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <tuner.h>
#include <dxva.h>
#include <dxva2api.h>   // DXVA2
#include <Vidcap.h>
#include "moreuuids.h"

#include "HiResTimer.h"

#include "..\resource.h"

#include <list>
#include <vector>
using namespace std;


#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")
#pragma comment(lib, "Version.lib")
#include "xml_parser.h"

#include "..\lib\sqlite\sqlite3.h"
#include "Crc32.h"
#include "dsutil.h"

#include "bits.h"
#include "H264StructReader.h"

#include "MediaInfoDLL.h"

#include "..\interfaces\monofilters.h"

#include "..\dump_h.h"    // Interface
#include "filters\dump\filter_dump.h"

#include "..\dxva_null_h.h"    // Interface
#include "filters\dxva_null\filter_dxva_null.h"

#include "..\time_measure_h.h"    // Interface
#include "filters\time_measure\filter_time.h"

#include "filters\fake_m2ts\filter_fake_m2ts_device.h"

#include "..\psiconfig_h.h"    // CLSID
#include "filters\psi_config\psiconfig.h"

#include "..\analyzer_h.h"    // Interface
#include "filters\analyzer\analyzer.h"
#include "filters\analyzer\filter_analyzer.h"
#include "filters\analyzer\filter_analyzer_writer.h"
#include "filters\analyzer\analyzer_proppage_config.h"

#include "filters\h264_analyzer\h264_analyzer.h"
#include "filters\h264_analyzer\filter_h264_analyzer.h"

#include "filters\video_analyzer\video_analyzer.h"
#include "filters\video_analyzer\filter_video_analyzer.h"

#include "filters\audio_analyzer\audio_analyzer.h"
#include "filters\audio_analyzer\filter_audio_analyzer.h"

#include "mtypes_ext.h"

#include "CustomToolTipCtrl.h"

#include "GraphStudioModelessDialog.h"
#include "title_bar.h"
#include "url_label.h"
#include "seeking_bar.h"
#include "EVR_VideoWindow.h"
#include "RenderParameters.h"
#include "display_graph.h"
#include "display_view.h"
#include "filter_list.h"
#include "prop_tree.h"
#include "schedule_list.h"

#include "object_details.h"
#include "filename_list.h"

#include "graphCli.h"
#include "graphstudio.h"
#include "mru_list.h"
#include "MainFrm.h"
#include "graphDoc.h"
#include "FiltersForm.h"
#include "FilterFromFile.h"
#include "volumebarform.h"
#include "ConfirmForm.h"
#include "MeritForm.h"

#include "InterfaceScanner.h"
#include "MediaInfo.h"
#include "CustomPage.h"
#include "FilterDetailsPage.h"
#include "FilterVCMPage.h"
#include "FilterACMPage.h"
#include "BufferNegotiationPage.h"
#include "WMADecPage.h"
#include "WMResizerPage.h"
#include "DMOQualCtrlPage.h"
#include "SbeSinkPage.h"
#include "AnalyzerPage.h"

#include "BrowserControl.h"

#include "BlacklistForm.h"
#include "LookupForm.h"
#include "DecPerformanceForm.h"
#include "FavoritesForm.h"
#include "GraphConstructionForm.h"
#include "EventsForm.h"
#include "FileTypesForm.h"
#include "ScheduleForm.h"
#include "SeekForm.h"
#include "StatisticForm.h"
#include "RemoteGraphForm.h"
#include "TextInfoForm.h"
#include "FileSrcForm.h"
#include "FileSinkForm.h"
#include "ProgressForm.h"
#include "RenderUrlForm.h"
#include "PropertyForm.h"
#include "NewGroupForm.h"
#include "SbeConfigForm.h"
#include "CliOptionsForm.h"
#include "graphView.h"

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
