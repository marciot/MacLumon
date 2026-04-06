#pragma once

extern PicHandle gLumonPic, gLumonIcon;

PicHandle GetLumonIcon();

void SetupLogos ();
void DisposeLogos ();

void GetLumonSize (short fontSize, short *width, short *height);
void GetGlobeSize (short fontSize, short *width, short *height);
void GetGlobeRect (short fontSize, Rect *logoRect, short x, short y);

void DrawLumonStr (PicHandle lumonIcon);
void DrawLumonGlobe (short fontSize, const Rect &logoRect);
