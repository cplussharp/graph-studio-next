# GraphStudioNext Frequently Asked Questions


### My graph behaves unexpectedly in GraphStudioNext, is it a GraphStudioNext bug?
Possibly, but consider some further steps to isolate the problem:
* Does the same problem happen in [Graph Edit](https://msdn.microsoft.com/en-us/library/windows/desktop/dd407274(v=vs.85).aspx), Microsoft's own graph editor in the Windows SDK?
* Do the filters you are using support the operation you are attempting? Many filters do not support all DirectShow features. For example some filters don't support seeking or only support specific time formats. Consult the documentation for the filters you are using.
* If the graph throws an exception, attach a debugger to see whether the crash happens in graphstudionext.exe or a filter DLL.
* Do you have other applications installed or running that are affecting the operation of DirectShow? A notable example is the excellent tool, [Filter Graph Spy](http://alax.info/blog/777), which hooks DirectShow throughout the system at a low level and can change DirectShow behaviour (as the author himself warns).
* Some diagnostic features in the GraphStudioNext options menu can cause unexpected behaviour by design. For example, "Reserve Memory below 2GB" may cause graphs to run out of memory or fail if some filters aren't flagged as LargeAddressAware.

### Do I have to install the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)?
No, GraphStudioNext will work without it, but the Windows SDK does include some very important tools:
* Graph Edit (graphedt.exe), Microsoft's own DirectShow graph editor.
* proppage.dll and evrprop.dll which provide property pages and the ability to connect to graphs in other processes.
* Ole/COM Object Viewer, useful for looking at registered COM classes and looking at COM type libraries packaged inside filter DLLs.
* Header files and import libraries for building DirectShow applications. 
* DirectShow sample code.
* The DirectShow baseclasses for building filters.  
These are included in the tools for desktop development, Universal Windows App development tools are not required.

### How do I do/What is __XXX__ in DirectShow?
* As a first step read the [MSDN DirectShow documentation](https://msdn.microsoft.com/en-us/library/windows/desktop/dd375454(v=vs.85).aspx). It is well written and is the definitive documentation for DirectShow. Many important details can be missed on the first reading, so read it again!
* Is there a relevant [Microsoft DirectShow sample](https://msdn.microsoft.com/en-us/library/windows/desktop/dd375468(v=vs.85).aspx) to refer to?
* Search engines are your friend. There is loads of DirectShow sample code and discussion on the web.

### I can't see the property pages for Microsoft filters
Have you installed the Windows SDK and registered proppage.dll (using regsvr32)? Proppage.dll implements the property pages for many Microsoft filters. Similarly, registering evrprop.dll in the Windows SDK will provide extra property pages for the enhanced video renderer filter.

### When I attempt to connect to a remote graph I get an error
Have you installed the Windows SDK and registered proppage.dll (using regsvr32)? Proppage.dll implements DirectShow COM proxy/stub implementations that are required for calling DirectShow interfaces on a graph in another process.

### Why doesn't GraphStudioNext generate C++ or C# code
Mainly because the authors don't have an interest in writing or using such a feature themselves. It's hard to write general purpose DirectShow code that would suit all users. We are not ruling such a feature out if someone else is prepared to write it. Feel free to write some code and get involved! There is one commercial graph editor,[Graph Edit Plus](http://www.infognition.com/GraphEditPlus/), that supports code generation.
