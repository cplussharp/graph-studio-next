# GraphStudioNext Frequently Asked Questions

### What's a *"DirectShow Graph Editor"* and why would I want to use one?
[DirectShow](https://msdn.microsoft.com/en-us/library/windows/desktop/dd375454.aspx) is a Microsoft Windows multimedia streaming API. A DirectShow [graph](https://msdn.microsoft.com/en-us/library/windows/desktop/dd407188.aspx) is a collection of connected multimedia components configured to achieve a particular multimedia processing task (e.g. media playback, capture or conversion). A DirectShow graph editor such as Microsoft [GraphEdit]  (https://msdn.microsoft.com/en-us/library/windows/desktop/dd377601.aspx) is a test environment for creating and testing DirectShow graphs. Graphs can then be created directly in a programming language such as C++ using the DirectShow API.

### Where's the Mac/Linux/Android/iOS etc. version?
DirectShow is an exclusively Windows technology so GraphStudioNext is Windows only and not designed for portability to other platforms.

### Does GraphStudioNext also support Media Foundation?
No. Media Foundation is a later Windows API that has similar concepts to DirectShow. DirectShow and Media Foundation are not currently designed to interoperate. Hybrid components could be written to transfer data between DirectShow and Media Foundation but it would be a non-trivial task. If such hybrid components existed there might be a case for adding Media Foundation support to GraphStudioNext. There is already an analagous editing tool in Media Foundation, [Microsoft TopoEdit](https://msdn.microsoft.com/en-us/library/windows/desktop/ff485862.aspx)

### My graph behaves unexpectedly in GraphStudioNext, is it a GraphStudioNext bug?
Possibly, but consider some further steps to isolate the problem:
* Does the same problem happen in [Graph Edit](https://msdn.microsoft.com/en-us/library/windows/desktop/dd407274.aspx), Microsoft's own graph editor in the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)?
* Do the filters you are using support the operation you are attempting? Many filters do not support all DirectShow features. For example some filters don't support seeking or only support specific time formats. Consult the documentation for the filters you are using.
* If the graph throws an exception, attach a debugger to see whether the exception happens in graphstudionext.exe or a filter DLL.
* Do you have other applications installed or running that are affecting the operation of DirectShow? A notable example is the excellent tool, [Filter Graph Spy](http://alax.info/blog/777), which hooks DirectShow throughout the system at a low level and can change DirectShow behaviour (as the author himself warns).
* Some diagnostic features in the GraphStudioNext "Options" menu can cause unexpected behaviour by design. For example, "Reserve Memory below 2GB" may cause graphs to run out of memory or fail if some filters aren't flagged as LargeAddressAware.

### How do I report a bug or feature request
* https://github.com/cplussharp/graph-studio-next/issues
* [Search including closed issues](https://github.com/cplussharp/graph-studio-next/issues?utf8=%E2%9C%93&q=is%3Aissue) to see if the bug or feature request is a duplicate.
* Report what you expected to happen and what actually happened
* Say which version of GraphStudioNext you are using. If possible, reproduce the problem using the latest official build.
* If using third party non-Microsoft filters, please say what they are and which versions of them you're using.
* If possible, create a graph that shows the problem and save and include all the GRF/GRFX/PNG/TXT files in the issue
* Include sample code if relevant
* If reporting a crash, consider capturing and including a [minidump](http://www.wintellect.com/devcenter/jrobbins/how-to-capture-a-minidump-let-me-count-the-ways) of the crash. Normal minidumps are fine, full memory minidumps with heap are huge and too large to attach to an issue.

### Do I have to install the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)?
No, GraphStudioNext will work without it, but the Windows SDK does include some very important tools:
* [Graph Edit](https://msdn.microsoft.com/en-us/library/windows/desktop/dd407274.aspx), Microsoft's own graph editor.
* proppage.dll and evrprop.dll which provide property pages and the ability to connect to graphs in other processes.
* [OLE-COM Object Viewer](https://msdn.microsoft.com/en-us/library/d0kh9f4c.aspx), useful for looking at registered COM classes and looking at COM type libraries packaged inside filter DLLs.
* Header files and libraries for building DirectShow applications and filters. 
* [DirectShow sample code](https://msdn.microsoft.com/en-us/library/windows/desktop/dd375468.aspx).
* The DirectShow [baseclasses](https://msdn.microsoft.com/en-us/library/windows/desktop/dd375456.aspx). Mainly useful for building filters though there are some classes and functions that can be used for developing DirectShow client applications.

These tools are included in the Windows SDK tools for desktop development, there is no need to install Universal Windows App development tools unless you want to.

### How do I do/What is *XXXXX* in DirectShow?
* As a first step read the [MSDN DirectShow documentation](https://msdn.microsoft.com/en-us/library/windows/desktop/dd375454.aspx). It is well written and is the definitive documentation for DirectShow. Many important details can be missed on the first reading, so read it again!
* Is there a relevant [Microsoft DirectShow sample](https://msdn.microsoft.com/en-us/library/windows/desktop/dd375468.aspx) to refer to?
* Search engines are your friend. There is loads of DirectShow sample code and discussion on the web.

### I can't see the property pages for Microsoft filters
Have you installed the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk) and registered proppage.dll using [regsvr32](https://support.microsoft.com/en-gb/help/249873/how-to-use-the-regsvr32-tool-and-troubleshoot-regsvr32-error-messages)? Proppage.dll implements the property pages for many Microsoft filters. Similarly, registering evrprop.dll in the Windows SDK will provide extra property pages for the enhanced video renderer filter.

### When I attempt to connect to a remote graph I get an error
Have you installed the Windows SDK and registered proppage.dll using [regsvr32](https://support.microsoft.com/en-gb/help/249873/how-to-use-the-regsvr32-tool-and-troubleshoot-regsvr32-error-messages)? Proppage.dll implements COM proxies and stubs that are required for calling DirectShow interfaces on a graph in another process.

### Why can I see property pages for a particular filter or pin in a local graph, but not in a remote graph?
Has the filter implemented COM proxies and stubs for calling the filter's custom COM interfaces in a remote process? It's possible to build these yourself using code generated by MIDL if you have the type libraries for the filter's custom COM interfaces but it's beyond the scope of this FAQ. Sometimes the type libraries for these interfaces are included in the filter DLL and can be viewed using "View Typelib" in [OLE-COM Object Viewer](https://msdn.microsoft.com/en-us/library/d0kh9f4c.aspx) in the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk). Generally, COM proxies and stubs and property pages for Microsoft filters are implemented in proppage.dll in the Windows SDK.

### Why doesn't GraphStudioNext generate C++ or C# code?
Mainly because the authors don't have an interest in writing or using such a feature themselves. It's hard to write general purpose DirectShow code that would suit all users. However, we are not ruling such a feature out if someone else would like to write it. Feel free to write some code and get involved! There is one commercial graph editor, [Graph Edit Plus](http://www.infognition.com/GraphEditPlus/), that supports code generation in C++ and C#.
