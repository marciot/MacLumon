#include "Lumon.h"
#include "Logo.h"

enum {
	kIdleBounceSpeed  = 1,
	kMinTicksPerFrame = 1
};

static Rect bounceRect;
static short dxBounce, dyBounce;

void StartIdleMode () {
	// Adjust the font sizes for small screens

	if ((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) < 500) {
		PenSize (1,1);
		TextSize (36);
	} else {
		PenSize (2,2);
		TextSize (48);
	}

	short logoWidth, logoHeight;
	GetGlobeSize (&logoWidth, &logoHeight);

	// Choose a random initial position of the logo which is fully inside the screen

	const short centerX = kIdleBounceSpeed + logoWidth  / 2 + (unsigned short)Random() % (qd.screenBits.bounds.right  - logoWidth  - kIdleBounceSpeed * 2);
	const short centerY = kIdleBounceSpeed + logoHeight / 2 + (unsigned short)Random() % (qd.screenBits.bounds.bottom - logoHeight - kIdleBounceSpeed * 2);
	dxBounce = kIdleBounceSpeed;
	dyBounce = kIdleBounceSpeed;

	GetGlobeRect (&bounceRect, centerX, centerY);

	// Give the logo region a black border so CopyBits doesn't leave a trail

	InsetRect (&bounceRect, -kIdleBounceSpeed, -kIdleBounceSpeed);
}

void DrawIdleMode () {
	BackColor( blackColor );
	ForeColor( whiteColor );

	EraseRect (&qd.screenBits.bounds);

	// Draw the logo

	Rect logoRect = bounceRect;
	InsetRect (&logoRect, kIdleBounceSpeed, kIdleBounceSpeed);
	DrawLumonGlobe (logoRect, gLumonIcon);

	// Reset these colors to avoid artifacting with CopyBits on color Macs

	BackColor( whiteColor );
	ForeColor( blackColor );
}

void AnimateIdleMode () {
	static long lastTicks = TickCount();
	const long currentTicks = TickCount();

	if ((currentTicks - lastTicks) >= kMinTicksPerFrame) {		
		Rect newRect = bounceRect;

		OffsetRect (&newRect, dxBounce, dyBounce);

		if ((newRect.right > qd.screenBits.bounds.right) || (newRect.left < qd.screenBits.bounds.left)) {
			dxBounce = -dxBounce;
			OffsetRect (&newRect, dxBounce * 2, 0);
		}
		if ((newRect.bottom > qd.screenBits.bounds.bottom) || (newRect.top < qd.screenBits.bounds.top)) {
			dyBounce = -dyBounce;
			OffsetRect (&newRect, 0, dyBounce * 2);
		}

		CopyBits (&qd.screenBits, &qd.screenBits, &bounceRect, &newRect, srcCopy, NULL);
		bounceRect = newRect;

		lastTicks = currentTicks;
	}
}