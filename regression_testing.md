
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
