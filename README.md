For Windows
Download the precompiled binaries or source code:

Visit the official libxml2 website: http://xmlsoft.org/.
Direct download link for Windows binaries: http://xmlsoft.org/sources/win32/.
Use a package manager like vcpkg for easier setup:

```bash
vcpkg install libxml2
vcpkg integrate install

```

```
{
    "name": "xmlconverter",
    "version": "1.0.0",
    "dependencies": [
        "libxml2"
    ],
	
	"builtin-baseline": "2960d7d80e8d09c84ae8abf15c12196c2ca7d39a" 
}

```


1. Enable Vcpkg Manifest Mode
You need to explicitly enable manifest mode in Visual Studio.

Via Visual Studio UI:
Right-click on your project in Solution Explorer and select Properties.
Go to Configuration Properties → vcpkg.
Set Enable vcpkg Manifest to Yes.
Click Apply and close the Properties window.
Via Command Line:
If you are building your project via msbuild, pass the following flag:

``` cmd
msbuild /p:VcpkgEnableManifest=true
```
