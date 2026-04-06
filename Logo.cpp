#include "Lumon.h"
#include "Logo.h"
#include "FixedMath.h"

// When drawing the LUMON logo, we selectively determine
// whether to draw a picture or use a Helvetica font. The
// picture looks muddy at smaller sizes, but Helvetica may
// not exist in larger sizes, so for font sizes smaller
// than maxFontSize, use Helvetica, otherwise use the picture.

#define kMaxFontSize 24

static const unsigned char *logoStr = "\pLUMON";
static PicHandle gLumonPic;

#if kMaxFontSize
	static PicHandle gLumonIcon;
#endif

void SetupLogos () {
	#if kMaxFontSize
		gLumonIcon = GetLumonIcon ();
	#endif

	gLumonPic  = (PicHandle)GetResource ('PICT',128);
}

void DisposeLogos () {
	#if kMaxFontSize
		KillPicture (gLumonIcon);
	#endif
	ReleaseResource ((Handle)gLumonPic);
}

#if kMaxFontSize
// Returns a QuickDraw picture with the Lumon icon

PicHandle GetLumonIcon() {
	Rect myRect;

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
#endif

/* This function checks whether an appropriate font exists for
   drawing the logo, if so, it populates the fontInfo structure
   and returns true. If this function returns false and we should
   use the scaled picture instead.
 */
static Boolean GetLumonFontInfo (FontInfo *fontInfo, short fontSize) {
	#if kMaxFontSize
		if ((fontSize <= kMaxFontSize) && RealFont (helvetica, fontSize)) {
			TextFont (helvetica);
			TextFace (bold);
			TextSize (fontSize);

			GetFontInfo (fontInfo);
			return true;
		}
	#endif
	return false;
}

void GetLumonSize (short fontSize, short *width, short *height) {
		FontInfo fi;

#if kMaxFontSize
	if (GetLumonFontInfo (&fi, fontSize)) {
		const short logoLineHeight = fi.ascent + fi.descent;
		*width  = StringWidth (logoStr);
		*height = logoLineHeight;
	} else
#endif
	{
		const Fixed picAspect = FixDiv (
			Long2Fix((*gLumonPic)->picFrame.right  - (*gLumonPic)->picFrame.left),
			Long2Fix((*gLumonPic)->picFrame.bottom - (*gLumonPic)->picFrame.top)
		);

		*height = fontSize;
		*width  = scaleBy( FixMul (picAspect, fontSize), 0.75);
	}
}

void GetGlobeSize (short fontSize, short *width, short *height) {
	short strWidth, strHeight;

	GetLumonSize (fontSize, &strWidth, &strHeight);
	const short logoPadding = scaleBy (strHeight, 0.25);

	*width  = scaleBy(strWidth, 0.9) / 4 * 6;
	*height = (strHeight + logoPadding * 2) * 2;
}

void GetGlobeRect (short fontSize, Rect *logoRect, short x, short y) {
	short logoWidth, logoHeight;

	GetGlobeSize (fontSize, &logoWidth, &logoHeight);

	SetRect (logoRect,
		x - logoWidth  / 2,
		y - logoHeight / 2,
		x + logoWidth / 2,
		y + logoHeight / 2
	);
}

void DrawLumonGlobe (short fontSize, const Rect &logoRect) {
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

	short strWidth, lineHeight;

	GetLumonSize (fontSize, &strWidth, &lineHeight);

#if kMaxFontSize
	FontInfo fi;

	if (GetLumonFontInfo (&fi, fontSize)) {
		MoveTo (
			logoRect.left + logoWidth/2 - strWidth/2,
			logoRect.top  + logoHeight/2 + fi.ascent/2
		);

		TextMode (notSrcCopy);
		DrawLumonStr (gLumonIcon);
	} else
#endif
	{
		// At larger sizes, a Helvetica font may not exist,
		// so used a scaled picture.

		tmp.left   = logoRect.left + logoWidth/2 - strWidth/2;
		tmp.right  = tmp.left + strWidth;
		tmp.bottom = logoRect.top + logoHeight/2 + lineHeight/2;
		tmp.top    = tmp.bottom - lineHeight;

		DrawPicture (gLumonPic, &tmp);
	}
}


void DrawBootLogo () {
	Rect myRect = qd.screenBits.bounds;

	EraseRect (&myRect);

	PenSize (gPenSize, gPenSize);
	TextSize (globeFontSize);

	Rect logoRect;
	GetGlobeRect (globeFontSize, &logoRect, myRect.right / 2, myRect.bottom / 2);
	DrawLumonGlobe (globeFontSize, logoRect);
}