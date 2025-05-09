#include <gccore.h>
#include <wiikeyboard/keyboard.h>
#include <wiiuse/wpad.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

bool quitapp = false;

void keyPress_cb(char sym)
{
	// Check for escape key to exit
	if (sym == 0x1b)
		quitapp = true;
}

int main(int argc, char **argv)
{
	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb, 20, 20, rmode->fbWidth-20, rmode->xfbHeight-20,
				 rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Clear the framebuffer
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

	// Make the display visible
	VIDEO_SetBlack(false);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0HHello World!\n");
	if (KEYBOARD_Init(keyPress_cb) == 0)
		printf("keyboard initialised\n");

	do
	{
		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button
		// has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// Check for keyboard input
		int key;

		if ((key = getchar()) != EOF)
		{
			// Display the pressed character
			// Print readable characters (ASCII > 31)
			if (key > 31)
				putchar(key);
			// Convert Enter key (ASCII 13) to a newline
			else if(key == 13)
				putchar('\n');
		}

		// We return to the launcher application via exit
		if (pressed & WPAD_BUTTON_HOME)
			quitapp = true;

		// Wait for the next frame
		VIDEO_WaitVSync();
	} while (!quitapp);

	return 0;
}
