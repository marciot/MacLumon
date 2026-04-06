#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <ToolUtils.h>

#include "Lumon.h"
#include "Logo.h"

enum {
	mApple         = 32000,
	mFile          = 32001,
	mEdit          = 32002,

	// Items in mFile
	mQuit          = 9
};

enum {
	kQuitTries     = 3,
	kBootTicks     = 120,
	kIdleTicks     = 3600,
};

Boolean   gQuit = false;
RgnHandle gMenuBarRgn;
short     gSavedMenuBarHeight;

// This function will show menu bar when the mouse
// hovers over it and hide it otherwise.

static void AutoHideMenuBar () {
	static Boolean menuBarVisible = true;

	Point mouseLoc;
	GetMouse (&mouseLoc);

	Boolean cursorInMenuBar = mouseLoc.v < gSavedMenuBarHeight;

	if (menuBarVisible != cursorInMenuBar) {
		if (cursorInMenuBar) {
			// Show the menu bar
			LMSetMBarHeight (gSavedMenuBarHeight);
			DrawMenuBar();
		} else {
			// Hide the menu bar
			LMSetMBarHeight (0);

			// Repaint the area under the menu bar

			Rect menuRect = qd.screenBits.bounds;
			menuRect.bottom = menuRect.top + gSavedMenuBarHeight;
			InvalRect (&menuRect);
		}
		menuBarVisible = cursorInMenuBar;
	}
}

static void SetupMenus () {
	gSavedMenuBarHeight = LMGetMBarHeight();

	// Get the menu bar region

	Rect menuRect = qd.screenBits.bounds;
	menuRect.bottom = menuRect.top + gSavedMenuBarHeight;

	gMenuBarRgn = NewRgn ();
	RectRgn (gMenuBarRgn, &menuRect);

	AutoHideMenuBar ();

	// Set up our menus

	Handle ourMenuBar = GetNewMBar( 128 );
	SetMenuBar ( ourMenuBar );
	AppendResMenu ( GetMenuHandle( mApple ), 'DRVR' );
	ReleaseResource ( ourMenuBar );
	DrawMenuBar();
}

static void DoMenuSelection (long choice) {
	Str255 daName;
	const int        menuId  = HiWord(choice);
	const int        itemNum = LoWord(choice);
	const MenuHandle hMenu   = GetMenuHandle(menuId);

	switch (menuId)  {

		case mApple:
			switch (itemNum) {
				case 1:
					Alert (128, NULL);
					break;
				default:
					GetMenuItemText( hMenu, itemNum, daName );
					OpenDeskAcc( daName );
					break;
			}
			break;

		case mFile:
			static triesRemaining = kQuitTries;
			if (itemNum == mQuit) {
				if (triesRemaining--) {
					Alert (129, NULL);
				} else {
					gQuit = true;
				}
			}
			break;
	}
	HiliteMenu(0);
}

void main () {
	enum {bootMode, workMode, idleMode} state = bootMode;
	unsigned long gTimer = TickCount ();
	Point currentMousePoint, lastMousePoint;
	WindowPtr mainWindow;

	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	// Initialize the random number generator

	GetDateTime((unsigned long*) &qd.randSeed);

	// Create our main window

	LMSetPaintWhite(0);

	Rect windRect = qd.screenBits.bounds;
	mainWindow = NewWindow( nil, &windRect, "\pLumon Industries", false, plainDBox,
							(WindowPtr) -1, false, 0 );

	SetPort (mainWindow);
	BackPat (&qd.black);
	#ifdef ACCENT_COLOR
		BackColor (ACCENT_COLOR);
	#endif
	PenMode (patBic);
	ShowWindow (mainWindow);

	// Set up our menus and other resources

	SetupMenus ();
	SetupFonts ();
	SetupLogos ();

	// Run the main event loop

	while (!gQuit) {

		// Process events from the system

		EventRecord event;

		if (GetNextEvent(everyEvent, &event)) {
			GrafPtr savedPort;
			WindowPtr window;   /* for handling window events */

			switch (event.what) {

				case updateEvt:
					window = (WindowPtr) event.message;
					if (window == mainWindow) {
						BeginUpdate (window);
						UnionRgn (window->visRgn, gMenuBarRgn, window->visRgn);

						#ifdef ACCENT_COLOR
							BackColor (ACCENT_COLOR);
						#endif

						switch (state) {
							case bootMode:
								DrawBootLogo ();
								break;
							case workMode:
								DrawWorkMode ();
								break;
							case idleMode:
								DrawIdleMode ();
								break;
						}

						EndUpdate (window);
						UnionRgn (window->visRgn, gMenuBarRgn, window->visRgn);
					}
					break;

				case mouseDown:
					switch (FindWindow(event.where, &window)) {
						case inDrag:
							Rect dragRect = (**GetGrayRgn()).rgnBBox;
							DragWindow(window, event.where, &dragRect);
							break;
						case inSysWindow:
							SystemClick(&event, window);
							break;
						case inGoAway:
							break;
						case inMenuBar:
							DoMenuSelection(MenuSelect(event.where));
							gTimer = TickCount();
							break;
						case inContent:
							/*if (window != FrontWindow()) {
								SelectWindow(window);
								event->what = nullEvent;
							}*/
							break;
					}
					break;

				case keyDown:
				case autoKey:
					if ((event.modifiers & cmdKey) != 0) {
						const char key = event.message & charCodeMask;
						const long menuResult = MenuKey (key);
						if (menuResult) {
							DoMenuSelection (menuResult);
						}
					}
					break;
			}
		}

		// Handle our state transitions

		switch (state) {

			case bootMode:
				if ((TickCount() - gTimer) > kBootTicks) {
					StartWorkMode ();
					state = workMode;
				}
				break;

			case workMode:
				AnimateWorkMode();

				if (Button()) {
					gTimer = TickCount(); // Reset the idle timer
				}

				// Check to see whether we should start the screen saver
				if ((TickCount() - gTimer) > kIdleTicks) {
					HideCursor ();
					SelectWindow (mainWindow);
					StartIdleMode ();
					GetMouse (&lastMousePoint);
					InvalRect (&qd.screenBits.bounds);
					state = idleMode;
				}
				break;

			case idleMode: {
				AnimateIdleMode ();

				// Check to see whether we should stop the screen saver
				GetMouse (&currentMousePoint);
				if (!EqualPt(currentMousePoint, lastMousePoint) || (event.what == keyDown)) {
					ShowCursor ();
					gTimer = TickCount();
					InvalRect (&qd.screenBits.bounds);
					state = workMode;
				}
				break;
			}
		}

		// Do other tasks

		AutoHideMenuBar ();
		SystemTask();

	} // !gDone

	// Clean up

	if (state == idleMode) {
		ShowCursor ();
	}

	DisposeRgn (gMenuBarRgn);
	DisposeLogos ();
}