#include "Lumon.h"
#include "Logo.h"
#include "FixedMath.h"

static const unsigned char *logoStr = "\pLUMON";

// Returns a QuickDraw picture with the Lumon icon

PicHandle GetLumonIcon() {
	Rect myRect;

	BackColor( blackColor );
	ForeColor( whiteColor );

	RgnHandle rgn1 = NewRgn();
	OpenRgn();
		SetRect (&myRect, 47, 12, 53, 18);
		FrameOval(&myRect);

		SetRect (&myRect, 35, 45, 65, 85);
		FrameOval(&myRect);
	CloseRgn (rgn1);

	Point savedPenLoc;
	GetPen (&savedPenLoc);

	RgnHandle rgn2 = NewRgn();
	OpenRgn();
		MoveTo (47, 15);
		LineTo (35, 65);
		LineTo (65, 65);
		LineTo (53, 15);
		LineTo (47, 15);
	CloseRgn (rgn2);
	UnionRgn (rgn1, rgn2, rgn1);

	MoveTo (savedPenLoc.h, savedPenLoc.v);

	SetRect (&myRect, 0, 0, 100, 100);
	ClipRect (&myRect);

	PicHandle myIcon = OpenPicture (&myRect);
	PaintRoundRect(&myRect, 50, 50);
	EraseRgn (rgn1);
	ClosePicture();

	DisposeRgn (rgn1);
	DisposeRgn (rgn2);
	ClipRect (&qd.screenBits.bounds);

	return myIcon;
}

// Draws the string "LUMON", but substitutes the 'O' with the Lumon icon, which
// should be provided to this function

void DrawLumonStr (PicHandle lumonIcon) {
	TextFont (helvetica);
	TextFace (bold);

	FontInfo fi;
	GetFontInfo (&fi);

	// Measure the 'O' character, it seems about 95% as tall as the line

	const oWidth  = CharWidth('O');
	const oHeight = scaleBy (fi.ascent, 0.95);

	// Draw 'LUM N' leaving a space for the 'O' and capturing it's position

	Point oPos;

	DrawChar ('L');
	DrawChar ('U');
	DrawChar ('M');
	GetPen (&oPos);
	Move(oWidth,0);
	DrawChar ('N');

	// Draw the icon where the 'O' would be

	Rect iconRect;
	SetRect (&iconRect, oPos.h, oPos.v - oHeight, oPos.h + oWidth, oPos.v);

	DrawPicture (lumonIcon, &iconRect);
}

void GetGlobeSize (short *width, short *height) {
	TextFont (helvetica);
	TextFace (bold);

	FontInfo fi;
	GetFontInfo (&fi);

	const short logoLineHeight = fi.ascent + fi.descent;
	const short logoPadding    = scaleBy (logoLineHeight, 0.25);

	*width  = (StringWidth (logoStr) - 2) / 4 * 6;
	*height = (logoLineHeight + logoPadding * 2) * 2;
}

void GetGlobeRect (Rect *logoRect, short x, short y) {
	short logoWidth, logoHeight;

	GetGlobeSize (&logoWidth, &logoHeight);

	SetRect (logoRect,
		x - logoWidth  / 2,
		y - logoHeight / 2,
		x + logoWidth / 2,
		y + logoHeight / 2
	);
}

void DrawLumonGlobe (const Rect &logoRect, PicHandle lumonIcon) {
	EraseOval (&logoRect);
	FrameOval (&logoRect);

	// Draw latitude lines

	const short ph = qd.thePort->pnSize.h;
	const short pv = qd.thePort->pnSize.v;

	const short logoWidth  = logoRect.right  - logoRect.left;
	const short logoHeight = logoRect.bottom - logoRect.top;

	// Foreshortening of latitude lines halfway up from equator to pole
	const short f = scaleBy (logoWidth, 0.06698); // logoWidth / 2 * (1 - sqrt(0.75))

	MoveTo (logoRect.left  + f,      logoRect.top + logoHeight / 4);
	LineTo (logoRect.right - f - ph, logoRect.top + logoHeight / 4);

	MoveTo (logoRect.left  + f,      logoRect.top + logoHeight / 4 * 3 - pv);
	LineTo (logoRect.right - f - ph, logoRect.top + logoHeight / 4 * 3 - pv);

	// Draw longitude lines

	const short step = logoWidth / 6;

	Rect tmp = logoRect;
	InsetRect (&tmp, step, 0);
	FrameOval (&tmp);

	InsetRect (&tmp, step, 0);
	FrameOval (&tmp);

	// Draw the text
	TextFont (helvetica);
	TextFace (bold);

	FontInfo fi;
	GetFontInfo (&fi);

	const short lineHeight  = fi.ascent + fi.descent;
	const short strWidth    = StringWidth (logoStr);

	MoveTo (
		logoRect.left + logoWidth/2 - strWidth/2,
		logoRect.top  + logoHeight/2 + fi.ascent/2
	);

	TextMode (srcCopy);
	DrawLumonStr (lumonIcon);
}

void DrawBootLogo () {
	Rect myRect = qd.screenBits.bounds;

	BackColor( blackColor );
	ForeColor( whiteColor );

	EraseRect (&myRect);

	// Adjust the font sizes for small screens

	if ((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) < 500) {
		PenSize (1,1);
		TextSize (36);
	} else {
		PenSize (2,2);
		TextSize (48);
	}

	Rect logoRect;
	GetGlobeRect (&logoRect, myRect.right / 2, myRect.bottom / 2);
	DrawLumonGlobe (logoRect, gLumonIcon);
}