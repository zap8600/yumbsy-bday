#define CNFG_IMPLEMENTATION

#include "rawdraw_sf.h"

void HandleKey(int keycode, int bDown) { }
void HandleButton(int x, int y, int button, int bDown) { }
void HandleMotion(int x, int y, int mask) { }
int HandleDestroy() { return 0; }
int main() {
	CNFGSetup( "Yumbsy bday", 1024, 768 );
	while(CNFGHandleInput()) {
        CNFGClearFrame();
		CNFGSwapBuffers();		
	}
}