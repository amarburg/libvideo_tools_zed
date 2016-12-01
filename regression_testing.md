
Fixed length run at 1200 frames @ VGA

commit    ZedRecorder     Recorder
56feb8c   25.081s (52.7924 FPS) 25.464s
Added CONAN_LIBS to tools/CMakeLists.txt 50.9486 FPS  --  getting ~56-57 FPS if not recording

Re-enabled running without writing SVO, also moved timing code closer to for loop (e2cf5d9)
60.1083 FPS when not recording
~55 FPS when recording (17.7184 MB/sec) this is consistent with the Zed tools
-- perhaps try benchmarking streaming to SSD ... simple DD test gives 194-218

Re-enabled libvideoio dependency in conanfile.py.   Regression.

Added just g3log as a conan dependency (f5bce02).  No regression.

enabled one logger message (878468c).  No apparent regression.


re-enabled my own log sink (still just a few test messages, though) (e505ce1).  
Very minor performance drop (~1fps?)

Try sl::zed::Camera::sticktoCPUCore(3).   21.8124 FPS.


Re-enabled g3log outputs rather than cout (992f0af).  No regression (faster?)
as a check, 120 frames at hd720 = 15.5709 FPS
hd1080 = 6.75292   FPS
noting that these match the performance of the zed viewer software (~7 FPS in HD1080, 15-20GPS in HD720)

Added "count" message in main loop (29b5bab).  No regression.

Put duration checking back in (7ce2b7a).  No apparent regression.


Re-added display code which copies matrix from Zed to and OpenCV mat (but does not yet imshow) (4ecc066).
Perhaps a slight regression (2-3 FPS)

Added TCLAP and libactive_object as conan deps.   No apparent regression.

Switched back to --std=c++11.  No regression.


Interestingly, if I include all of liblogger's conan deps (snappy, zlib), but not liblogger itself, I don't see a regression,
maybe a couple of FPS.


Change zlib to static linkage, force rebuild of liblogger, regression goes away.

Funny, change `default_options = ... "zlib:shared=False", ...` to True in liblogger's conanfile,
rebuild (`conan ... --build=liblogger`) and I can make the regression appear.

Return to original conanfile configuration -- depending on just libvideoio -- but with the
static liblogger, and no regression.



Realized, of course, I can just get zlib from the system, don't need to build it.   So I removed
it from liblogger's conanfile.py (liblogger commit 795b22086da8e168f35c4c44a9c0b52e98ce3eb5).   No regression.
