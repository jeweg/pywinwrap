pywinwrap
=========
Pywinwrap is a simple console application to conveniently start a Python console script on Windows.
When run, it will look for a Python script in the same directory as itself and with the same name, safe for a different suffix in place of its extension (default is ```-script.py```), and run it as a Python script, delegating all arguments.

This approach is similar to ```run.exe``` that comes with setuptools, which unfortunately refused to work for me on some machines. So I wrote my own.

* Shebang lines (e.g. ```#!c:\python27\python.exe```) with a path to a Python interpreter (e.g. as set by the setuptools Windows installers) are recognized and that particular interpreter is started. If there is no valid shebang line, Python is assumed to be in the path.
* Shebang detection works for scripts in **ANSI**, **UTF-8**, and **UTF-16** big and little endian encoding, with or without **BOM**.
* Same executable works with **32-bit** and **64-bit** Python versions.
* Comes with two [UPX](http://upx.sourceforge.net/)-compressed prebuilt versions: one dynamically linked to the Visual C runtime (```msvcr100.dll```, 6 KB) and a statically-linked version (29 KB) that is a little bigger, but should work anywhere.

**Usage**

Just rename script and a copy of the executable (```<name>-script.py``` and ```<name>.exe```, respectively) and deploy.

**Disclaimer**

This is an intentionally simple approach that happens to work very well for my use cases.
Some alternatives:
* A wrapping ```.bat``` file. Unfortunately, there doesn't seem to be workaround for the annoying prompt when interrupting a script via Ctrl-C.
* Adding ```.py``` to *PATHEXT*. That works well, but requires some user cooperation or additional setup.
* A much more sophisticated approach is described in  [PEP-397](http://www.python.org/dev/peps/pep-0397/) and implemented [here](https://bitbucket.org/pypa/pylauncher). That seems a bit more invasive, though.

**Todo**
* There is no equivalent to setuptools' ```cli.exe``` to start GUI scripts yet.
* Trailing arguments in the shebang line are not supported yet.
