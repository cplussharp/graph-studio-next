# GraphStudioNext

GraphStudioNext is a DirectShow graph editor. It's an open source alternative to [Microsoft Graph Edit](https://msdn.microsoft.com/en-us/library/windows/desktop/dd407274.aspx) in the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk) with many additional features.

GraphStudioNext is a fork of [RadScorpion's](http://blog.monogram.sk/janos/) [GraphStudio](http://blog.monogram.sk/janos/tools/monogram-graphstudio/).
Because development had stopped on the really useful tool GraphStudio, I started this fork. I use this application every day and I needed some new features to support my workflows and wanted to fix some really annoying bugs.

You're welcome to submit bugs and feature requests or just comment whether you like or hate the application [here](https://github.com/cplussharp/graph-studio-next/issues).

Thanks to the following for supporting the development of GraphStudioNext:
 * [root6](http://www.root6.com/) Creators of workflow automation software.

## Latest build:
[![Build status](https://ci.appveyor.com/api/projects/status/github/cplussharp/graph-studio-next?svg=true)](https://ci.appveyor.com/project/cplussharp/graph-studio-next)

### win32:
Release: [graphstudionext.exe](https://ci.appveyor.com/api/projects/cplussharp/graph-studio-next/artifacts/bin/graphstudionext.exe?job=Environment%3A%20CONFIGURATION%3DRelease%2C%20PLATFORM%3DWin32)
([.pdb](https://ci.appveyor.com/api/projects/cplussharp/graph-studio-next/artifacts/bin/graphstudionext.pdb?job=Environment%3A%20CONFIGURATION%3DRelease%2C%20PLATFORM%3DWin32))

Debug: [graphstudionext.exe](https://ci.appveyor.com/api/projects/cplussharp/graph-studio-next/artifacts/bin/graphstudionext.exe?job=Environment%3A%20CONFIGURATION%3DDebug%2C%20PLATFORM%3DWin32)
([.pdb](https://ci.appveyor.com/api/projects/cplussharp/graph-studio-next/artifacts/bin/graphstudionext.pdb?job=Environment%3A%20CONFIGURATION%3DDebug%2C%20PLATFORM%3DWin32))

### x64:

Release: [graphstudionext64.exe](https://ci.appveyor.com/api/projects/cplussharp/graph-studio-next/artifacts/bin/graphstudionext64.exe?job=Environment%3A%20CONFIGURATION%3DRelease%2C%20PLATFORM%3Dx64)
([.pdb](https://ci.appveyor.com/api/projects/cplussharp/graph-studio-next/artifacts/bin/graphstudionext64.pdb?job=Environment%3A%20CONFIGURATION%3DRelease%2C%20PLATFORM%3Dx64))

Debug: [graphstudionext64.exe](https://ci.appveyor.com/api/projects/cplussharp/graph-studio-next/artifacts/bin/graphstudionext64.exe?job=Environment%3A%20CONFIGURATION%3DDebug%2C%20PLATFORM%3Dx64)
([.pdb](https://ci.appveyor.com/api/projects/cplussharp/graph-studio-next/artifacts/bin/graphstudionext64.pdb?job=Environment%3A%20CONFIGURATION%3DDebug%2C%20PLATFORM%3Dx64))

## Frequently Asked Questions
See [docs/FAQ.md](docs/FAQ.md)

## Releases

### Recent changes
See also [commit log comments](https://github.com/cplussharp/graph-studio-next/commits/master)

### Release 0.7.3.1 (2022-01-15)

*New Features:*
 * Relative paths to reference media files in GRFX [issue #319](https://github.com/cplussharp/graph-studio-next/issues/319)
 * Better support for editing and persisting filters created directly from DLL class factory [issue #318](https://github.com/cplussharp/graph-studio-next/issues/318) [issue #317](https://github.com/cplussharp/graph-studio-next/issues/317) [issue #227](https://github.com/cplussharp/graph-studio-next/issues/227)
 * Show internal filters in Filters dialog [issue #312](https://github.com/cplussharp/graph-studio-next/issues/312)
 * Create stream buffer recordings from Stream Buffer Sink filter property page [issue #302](https://github.com/cplussharp/graph-studio-next/issues/302)
 * Display IPin::QueryInternalConnections result in Pin PropertyPage [issue #300](https://github.com/cplussharp/graph-studio-next/issues/300)
 * Connect newly created filter to previously selected filter for faster graph building [issue #265](https://github.com/cplussharp/graph-studio-next/issues/265)
 * New option to reserve memory below 2GB/4GB for testing 32bit LargeAddressAware filters and 64bit filters for pointer truncation bugs [issue #298](https://github.com/cplussharp/graph-studio-next/issues/298)
 * Mediainfo information display for files created by file writing filters (similar to source file filters) [issue #278](https://github.com/cplussharp/graph-studio-next/issues/278)
 * Save text/graph information with graph and tweaks to text information [issue #277](https://github.com/cplussharp/graph-studio-next/issues/277)
 * Show current sync source in status bar
 * Include frames per second in video media type details [issue #295](https://github.com/cplussharp/graph-studio-next/issues/295)
 * Implement loading from ipersistpropertybag option when loading GRFX files [issue #284](https://github.com/cplussharp/graph-studio-next/issues/284)
 * Added count*average column to statistics windows [issue #270](https://github.com/cplussharp/graph-studio-next/issues/270)
 * Integration with DirectShowSpy property frame helper in remote graph form [issue #264](https://github.com/cplussharp/graph-studio-next/issues/264)
 * Display filter merit as SDK constant (as opposed to numeric value)

*Bug Fixes:*
 * Disable DPI awareness so windows scales the application [issue #335](https://github.com/cplussharp/graph-studio-next/issues/335)
 * Targeting for Windows 8.1 and 10 with manifest file [issue #331](https://github.com/cplussharp/graph-studio-next/issues/331)
 * Apply short-cuts in the main window only if it is active (e.g. copy text with Ctrl+C in property page) [issue #342](https://github.com/cplussharp/graph-studio-next/issues/342)
 * Fixed multiple layout problems on high DPI systems [issue #255](https://github.com/cplussharp/graph-studio-next/issues/255)
 * Filter name not displayed for filters supporting IFileSourceFilter, IFileSinkFilter [issue #305](https://github.com/cplussharp/graph-studio-next/issues/305)
 * Fixed registration of internal DXVA Null Renderer filter [issue #310](https://github.com/cplussharp/graph-studio-next/issues/310)
 * Crash fix connecting to remote graph containing video renderer filter [issue #333](https://github.com/cplussharp/graph-studio-next/issues/333)
 * Crash fix closing property page launched from filters window [issue #329](https://github.com/cplussharp/graph-studio-next/issues/329)
 * Fixed bug in sync source selection in new graphs [issue #291](https://github.com/cplussharp/graph-studio-next/issues/291)
 * EVR renderer is no longer closed and recreated when graph is refereshed [issue #293](https://github.com/cplussharp/graph-studio-next/issues/293)
 * Fix to Analyzer Filter property page [issue #289](https://github.com/cplussharp/graph-studio-next/issues/289)
 * Filters having no valid CLSID are showing invalid tool tip with text from another filter [issue #288](https://github.com/cplussharp/graph-studio-next/issues/288)
 * Analzyer writer filter returns current position from IStream::Seek [issue #281](https://github.com/cplussharp/graph-studio-next/issues/281)
 * Fixed wrong loading of current sync source in GRF and GRFX files [issue #268](https://github.com/cplussharp/graph-studio-next/issues/268)
 * 32bit version running on 64bit OS no longer silently ignores exceptions that happen from user mode callbacks (e.g. key press handlers) [issue #269](https://github.com/cplussharp/graph-studio-next/issues/269)
 * Better robustness against filters that delete rather than Release() pins [issue #115](https://github.com/cplussharp/graph-studio-next/issues/115)
 * Better property page positioning [issue #213](https://github.com/cplussharp/graph-studio-next/issues/213)

### Release 0.7.0.430 (2014-11-10)

*New Features:*
 * Navigate around filter graph using keyboard. Arrow keys navigate between filters, control-arrow navigates between pins, shift key extends the selection. Newly created filters are connected to the currently selected output pin. Escape key to clear filter and pin selection. Shift F10 or menu key for context menu. Enter or Alt Enter for properties. Delete or Backspace to delete filter(s) or disconnect pin(s). Shift Delete or Backspace to disconnect filter(s).  (issue #217, #issue 252, issue #254)
 * Connect pin offers popup menu of available target pins to connect to. If selected pin already connected, reconnect with the same media type (or use intelligent or choose media type modes if enabled). If two filters are selected, connect the filters together via the first available pins. (issue #222, issue #249)
 * XML files now saved under new .grfx extension. .grfx file extension is registered with Windows (issue #152)
 * Preview and filter DbgLog file output in property page (NB DbgLog works in debug filter builds *ONLY*). Also configure registry settings for DbgLog. (issue #61)
 * Better support for creating filters directly from DLLs instead of CoCreateInstance. Scan for CLSIDs implemented by DLL, create the filter from the same DLL when graph is saved to .grfx file and reloaded (impossible to support this for .grf files though). DLLs appear on the File menu as recently used files (but don't clear the existing graph when 'opened'). Property pages are created directly from filters DLL class factory with fallback to creating via CoCreateInstance. Can be used to run two different versions of the same filter side by side in a single graph. (issue #31)
 * Disconnect selected filters (issue #236)
 * Show H.265/HEVC decoder specific configuration (issue #242)
 * Show streaming progress in windows 7 taskbar (issue #18)
 * GraphEdit-style options for auto arrange filters and auto resize to fit graph (issue #113)
 * Looped playback (issue #241)
 * Full screen playback (issue #211)
 * Resizable property dialog (issue #216)
 * Register/Unregister Filter File without restarting the program as admin (issue #218)
 * Change filter merit without restarting the program as admin (issue #219)
 * Show registered file and protocol handlers (issue #37)
 * New File option 'Clear Document Before Load'. If disabled can load several graph files into a new graph. Useful for reusing old graphs - e.g. source from one graph and renderers from another (issue #183).
 * Context menu item to find filter in filters window (issue #204)
 * Optionally save XML, GRF and screenshot on every save (issue #199, issue #174)
 * DXVA-Null-Renderer added to internal filters
 * Optionally hide filter enumeration stats from statistics window (issue #214)
 * More robust matching of different GUID formats in GUID lookup window (issue #198)
 * Command line options to open remote graph. Integrates with DirectShow Filter Graph Spy http://alax.info/blog/777 (issue #209)
 * Support for PSM_PRESSBUTTON messages in property window (issue #207)
 * Switch between property page tabs using Control-Tab (issue #168)
 * Optionally leave playback clock enabled in performance test window (issue #190)
 * More flexible input of time format in seek dialog (issue #191)
 * Save filter wrap width as preference and add 'reset spacing' menu option (issue #189)
 * Show medium info in pin details (IKsPin for kernel streaming devices)(issue #192)
 * Show filter name in title bar of pin properties dialog (issue #194)
 * Set LargeAddressAware flags to allow up to 4GB memory usage in 32bit version
 * Different icon for 64bit version
 * Various keyboard access improvements (e.g issue #221 - Ctrl+Enter for switching between property pages)
 * Visual Studio 2011 and Visual Studio 2013 project files

*Bug Fixes:*
 * More reliable pin connections when loading from .grfx file or .grf file with internal .grf parser (issue #238)
 * Property page crash in Microsoft DVBS Network provider filter when no tuner hardware present (#issue 231)
 * Crash when viewing property page of pin that no longer exists (issue #239)
 * Overflow calculating AvgTimePerFrame on H264 (issue #246)
 * Create AAC Pin with PsiConfig (issue #206)
 * Better accuracy parsing seek times (issue #226)
 * Better error feedback for performance test window graph building (issue #182)
 * Remove non-functional menu items from context menus of wrapper filters e.g. DMO wrapper filter (issue #169, issue #175)
 * Support clipboard keyboard shorcuts for text controls in floating windows - Ctrl+A/C/X/V/Z (issue #215)
 * Crash in properties windows for Analyzer Writer Filter Input pin (issue #220)
 * Fix to property window positioning (issue #213)
 * Fix for annoying keyboard focus issues (issue #210)
 * Bug fixes to apply button handling in property window (issue #208)
 * Fixed drawing issues for long pin names (issue #201)
 * Show information for filters not registered in default category (issue #196)
 * Windows 7 taskbar behaviour fixes (issue #197)
 * Set focus to existing property windows if already shown (issue #179)
 * Changed title bar when connecting to remote graph (issue #184)
 * Show error message if setting source or destination file by drag and drop fails (issue #188)
 * List supported time formats in property window (issue #193)
 * XML file loading now prompts only once for each missing source or sink file (issue #185)
 * Crash fix to internal graph parser inf tee loading (issue #171)
 * Hide tooltip if no text to show (issue #170)

### Release 0.6.1.265 (2013-07-17)

*New Features:*
 * Color connection lines according to the major media type (issue #110)
 * Word wrap filter names to adjustable margin (issue #62)
 * Toolbar buttons and keyboard shortcuts for seeking (issue #155)
 * Tooltips for filter information and connection media types (issue #170)
 * Seek dialog supports stop position, rate and other options (issue #98)
 * Seek dialog supports playback rate, pre-roll and available data (issue #157)
 * Show CRC in Select Media Type dialog (issue #142)
 * Connection retry and sorting by column in Select Media Type dialog (issue #124)
 * Show pin render failures in graph construction window (issue #126)
 * Insert filters from favorite filters window (issue #163)
 * Helper to use MPEG2Demux for Mpeg2TS Devices (issue #34)
 * Show IAMCrossbar details (issue #148)
 * List VideoAcceleratorGUIDs from IAMVideoAccelerator (issue #141)
 * Display category information in Filters window (issue #130)
 * Right/double click on connection line for output pin context menu/property page (issue #172)
 * Show Audio/Video-Render menu in output pin context menu (issue #73)
 * Store and restore previous window positions (issue #63)
 * Edit more than one graph/document in a single process (issue #136)
 * All contrl or alt keyboard shorcuts work from within dialogs (issue #90)
 * Show 64Bit/32Bit in the title bar caption (issue #161)
 * PropertyPage checkbox to enable crc calculation in Analyzer
 * Show the real timestamp in the Analyzer property page (issue #150)
 * Projects for DLL versions of internal filters (issue #16)

*Bug Fixes:*
 * Update favorites window when filters added or removed with context menu (issue #167)
 * Robustness improvements to file loading (issue #85)
 * Improved robustness for loading GRF files with internal parser (issue #171)
 * Better error messages for XML file loading (issue #112)
 * Better keyboard navigation in propperty pages (issue #168)
 * Keyboard shortcuts for horizontal and vertical spacing fixed (issue #162)
 * Change ROT exported graph name so Graph Edit can access it (issue #143)
 * Don't show graphs in same process in connect to remote graph dialog (issue #144)
 * Fixed update of favorite filters items (issue #159)
 * Added tooltips for remaining toolbar controls (issue #156)
 * Analyzer Writer filter won't crash if output file not set (issue #145)
 * Analyzer filter works when output not connected (issue #149)
 * Analyzer Writer won't crash if output file set more than once (issue #146)
 * Combo Boxes in filters dialog dropdown display fixed (issue #137)
 * Console window popup not shown until enabled in options menu (issue #139)
 * Better error messages including proppage.dll reminder when connecting to remote graph fails (issue #42)
 * Microsoft Audio Resampler DMO missing from DMO Audio Effect category (issue #134)
 * Internal GRF file parser now restores state and filename in same order as normal GRF loading (issue #135)
 * Performance test now works if the EC_PAUSED event is not generated on start of playback (issue #132)
 * Turning off clock now works without starting playback first (issue #131)
 * Save new files as GRF format by default (issue #87)
 * Check AM_MEDIA_TYPE::cbFormat before use (issue #127)
 * Added extra output to build window to warn of subwcrev problems with source paths containing spaces (issue #154)


### Release 0.6.0.191 (2013-04-25)

*New Features:*
 * Show search string in edit control in filters dialog (issue #36)
 * Search by CLSID in filters dialog (issue #40)
 * Blacklist filters to prevent DirectShow using them in graph building (issue #54)
 * Improvements to filter smart layout (issue #65)
 * Graph builder log file support (issue #75)
 * Drag and drop of files to open, add to graph, open in file reader, add source filter, set filter source/sink file (issue #76)
 * Console window to capture stdout output and show internal GRF parsing (issue #81)
 * Better support for loading and saving XML graph files (issue #85)
 * Internal graph parser for loading and fixing GRF files that fail to load normally (thanks to Alessandro Angeli for GRF parsing code) (issue #87)
 * Show SVN revision number in explorer/properties/details (issue #97)
 * More precision in playback time readouts (issue #98)
 * Added more complete menus and keyboard accelerators (issue #104)
 * Modify filter vertical and horizontal spacing with wheel mouse or menus (issue #105)
 * Shift left/middle/right click on filter to operate on first available output pin (issue #108)
 * Added baseclasses project build step for easier self-contained build (issue #109)
 * Add GRF or XML graph files to existing graph (issue #114)
 * Options setting for using 'no thread' and 'private' filter graph CLSID (issue #116)
 * Allow partial media types in select media type connection mode (issue #124)

*Bug Fixes:*
 * Load GRF files containing internal filters (issue #39)
 * Remember maximized window state on restart (issue #50)
 * Fix mediainfo integration for x86 builds running on x64 platforms (issue #120)

See Help/Help Information... menu item for some details of keyboard/mouse bindings

### Release 0.5.1.117 (2013-01-28)

*New Features:*
 * Search within filter dialog by typing (also backspace and delete) in filter list control (Thanks to erofeev.info) (issue #36)
 * Filter dialog categories for all filters (the new default) and all DMOs (issue #36)
 * New Analzyer and Analyzer Writer internal filters to analyze graph data flow (issue #72)
 * New View/Graph Statistics menu item to display IAMStats data (issue #92)
 * Mouse horizontal wheel scrolling in graph window (issue #105)
 * Control mouse wheel to zoom in graph window (issue #105)
 * Control-shift mouse wheel to change playback speed (issue #105)
 * Middle button click to delete filter, connection or refresh graph (issue #99)
 * Disconnect and reconnect connected pins by dragging (issue #100)
 * Pin context menu can insert new filters into existing connection (issue #74)
 * New Context menu on empty area of graph for filter creation (issue #56)
 * Mpeg2Demux Filter context option "Create PSI Pin" (issue #34)
 * IFileSourceFilter and IFileSinkFilter context menu item(issue #91)
 * Simple commandline interface for the application (issue #86)
 
*Changes:*
 * Pin context menu uses current connection mode for connecting new filters (issue #49)
 * Backwards connection with select media type offers input pin media types (issue #49)
 * Context menus reorganized with keyboard accelerators (issue #102)
 * Version number includes SVN revision number (issue #97)
 * Save filter connections in XML file (thanks to Vladimir Panteleev) (issue #85)
 * Filter dialog displays audio properties for WAVEFORMATEXFFMPEG (thanks to Grant Simonds) (issue #103)
 * Filter dialog detection of IDirectDrawVideo, IAMAnalogVideoDecoder, IAMAsyncReaderTimestampScaling, ISpecifyPropertyPages, IStream, IBaseFilter, IPersist, IMemInputPin
 * UI cleanup for TextInfo-Dialog => added the GraphStudioNext titlebar
 * Seek dialog usability tweaks (issue #96)
 * Keyboard usability tweaks for floating windows and tab order (issue #90)
 * Performance test window usability tweaks (issue #58), 
 * Filter dialog usability tweaks  (issue #89)

*Bug Fixes:*
 * Specify media type for GetCurFile in dialogs to prevent problems with remote graphs (issue #53)

### Release 0.5.0.1 (2012-08-30)

*New Features:*
 * Change Playback Rate
 * Favorites Filters in Pin Context Menu
 * Toolbar Button to disconnect all Filters
 * Export Current Graph for Remote Connection
 * Process ID in Program Title to identify different running instances 
 * different Background Color if connected to Remote Graph
 * Interpret fourcc based media sub types
 * Select connection MediaType
 * Toolbar Buttons to switch between Connection Modes (Intelligent, Direct, Direct with MediaType selection)
 * Time execution of an arbitrary graph over several runs (TimeMeasure Filter is now useable for every Graph)
 
*Changes:*
 * optional show GUID for known GUIDs
 * new known GUIDs and Interfaces from uuids.h, DXVA
 * Connect pins more quickly by allow dragging to a filter rather than a precise pin
 * On WinVista/7 use TaskDialog for error messages (+ search button to direct search for the error)
 * Code Cleanup / show more error messages

*Bug Fixes:*
 * Tap and Hold on Touchscreen now opens the context menu
 * "Display as Filename" now works also for remote graphs
 * running on XP again (0.5.0.0 had a linking-error on XP)


### Release 0.4.9.0 (2012-01-22)

*New Features:*
 * Lookup/Search Dialog for all known GUIDs and HRESULTs
 * Toolbar Button for "Insert Filter"
 * Support for Win7 Taskbar
 * "Insert Tee Filter" for Pin and Menu
 * Show error Messages
 * ~430 known HRESULT values
 * partial h264 SPS/PPS parser for MPE2G2VideoInfo-Sequence
 * Partial MPEG2 Extensions parser for MPE2G2VideoInfo-Sequence
 * Show Markers for IAMExtendedSeeking
 * Show IMPEG2PIDMap Mapping for Pin
 * Internal PropertyPages for IDMOQualityControl and IWMResizersProps
 
*Changes:*
 * Resizeable "Remote Graphs" Dialog
 * a lot of new known GUIDs and Interfaces from moreuuids.h, Xiph, LAVFilter, BDA 
 * Code Cleanup

*Bug Fixes:*
 * Show Pins created with MPEG2Demux-PropertyPage


### Release 0.4.5.5 (2011-12-22)

*New Features:*
 * x64 build
 * audio-decoder performance test (issue 30)
 * comparison of streamtime and worktime in performance test

*Changes:*
 * don't use MediaInfo by default
 * improved text info dialog (thanks to Kurtnoise)
 * add m2ts/mts as file extension (thanks to Kurtnoise)
 * select "All Files" as default filter index (thanks to Kurtnoise)
 * add missing uncompressed video subtypes (thanks to Kurtnoise)


### Release 0.4.5.1 (2011-11-03)

*New Features:*
  * support for [http://mediainfo.sourceforge.net MediaInfo] -> more details in the graph-report and in the property-page of a filter with IFileSourceFilter interface (issue 25)
  * unregister multiple selected filters (issue 3)
  * register a filter file (issue 19)
  * load an unregistered filter from a filter-file (issue 23)
  * save button for graph-report ("text information") (issue 27)
  * save-dialog for screenshot (issue 28)
  * auto-refresh text information on report-level change (issue 26)
  * more details in the graph-report -> clsid, filter-file, filter-version (issue 25)
  * show milliseconds
  * autofit the seekbar to the window-size


### Release 0.4.0.0 (2011-09-27)

*New Features:*
  * insert multiple selected filters (issue 4)
  * "Insert Video source" / "Insert Audio Source" (issue 8)
  * Search for filter on google (issue 13)
  * Scan Filter/Pin Interfaces and show some details (issue 15)
  * Show filtername in IFileSourceFilter/IFileSinkFilter-Dialog (issue 21)
  * Copy filter-properties on Ctrl-D (issue 22)

*Bug Fixes:*
  * length of filepath/url in dialogs (issue 1)
  * Ctrl-D for DMO filters (issue 2)
  * Crash inCrash on showing property page for unconnectet DMO Filter (issue 6)
