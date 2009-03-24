The Short version
------------------
In order to run Kile on Windows, you'll need:

1. Kile and its supporting KDE libraries.  The KDE-Windows installer should take care of this for you.
2. A TeX installation available somewhere in your PATH.  The default options when installing MikTeX should make this happen
3. Viewers for whatever file types you plan to generate (PDF, PS, DVI, etc)

Helping Kile Find Helper Apps
-----------------------------
The most common problem you'll prbably encounter when trying to use Kile on Windows is Kile not being able to find all the supporting applications it depends on (latex, dvips, etc).  If this happens, the easiest solution is to just add the directories containing these programs to your PATH variable (details below).  To see which programs Kile is able to find, run the system checker (Settings -> System Checker) and see what you get.  

Note: acroread does not by default add itself to your PATH so you probably won't be able to use it directly.  However, if you have Adobe Reader setup as your default PDF viewer, setting Kile to System Default for ViewPDF should still work.  You could also just add C:\Program Files\Adobe\path\to\acrord32.exe to your path but it isn't really necessary.

Setting your PATH
-----------------
At a minimum, latex.exe and friends should be in your PATH.  MikTeX will do this for you during the install process, but if it doesn't work for some reason, you can add it manually by:
Vista: 1. Right click Computer -> Properties
       2. Click Advanced System Settings.
       3. Click Environment Variables button
       4. Find the PATH entry and double-click it.
       5. Add your LaTeX path to the start.  ie, add "C:\Program Files\MikTex 2.7\miktex\bin;" ot something similar to the start of the line (without the quotes)
       6. Add other PATHs as appropriate.  If it's not already, you may want to add your KDEROOT\bin directory also (for example, "C:\kderoot\bin;")

XP: 1. Right click My Computer -> Properties
    2. Click the Advanced tab
    3. Click the Environment Variables button
    4. Proceed as above

