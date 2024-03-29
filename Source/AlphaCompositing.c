/*
	File:		AlphaCompositing.c
	
	Description: This example takes two images and composites them together demonstrating the use of
	             different graphic modes.
	             Test Sample Image : Sample 3a-Beach.jpg and Sample 3b-Penguin w shadow.png

	Author:		QuickTime Engineering, DTS

	Copyright: 	� Copyright 1999 - 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first): <2> 4/22/02 released as dts sample code
										<1> 4/1/99 initial release

*/

#include "MacShell.h"

void AlphaComposite( void )
{
	OSErr err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	GraphicsImportComponent backgroundImporter = 0, foregroundImporter = 0;
	Rect backgroundBounds, windowBounds, foregroundBounds;
	MatrixRecord matrix;
	RGBColor whiteColor;
	FixedPoint backgroundCenter;

	BuildGraphicsImporterValidFileTypes( hOpenTypeList, &numTypes );
	HLock( hOpenTypeList );
	
	// prompt for a background image.
	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
	if (err) goto bail;
	err = GetGraphicsImporterForFile( &theFSSpec, &backgroundImporter );

	// prompt for a foreground image.
	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
	if (err) goto bail;
	err = GetGraphicsImporterForFile( &theFSSpec, &foregroundImporter );
	err = GraphicsImportGetNaturalBounds( backgroundImporter, &backgroundBounds );
	
	windowBounds = backgroundBounds;
	OffsetRect( &windowBounds, 10, 45 );
	window = NewCWindow( NULL, &windowBounds, "\pAlpha Composite", true, documentProc, (WindowPtr)-1, true, 0);
	
	// set the graphics port for drawing the first image
	err = GraphicsImportSetGWorld( backgroundImporter, GetWindowPort( window ), NULL );
	
	// draw the background
	err = GraphicsImportDraw( backgroundImporter );
	
	pause();
	
	// center the foreground image over the background image
	// get the native size of the foreground image
	// offset the image to center it
	// set the rectangle in which to draw an image
	err = GraphicsImportGetNaturalBounds( foregroundImporter, &foregroundBounds );
	OffsetRect( &foregroundBounds, 
			    (backgroundBounds.right - foregroundBounds.right) / 2,
			    (backgroundBounds.bottom - foregroundBounds.bottom) / 2 );
	err = GraphicsImportSetBoundsRect( foregroundImporter,
									   &foregroundBounds );
	
	// draw the foreground image over the background image,
	// using the default graphics mode, ditherCopy.
	err = GraphicsImportSetGWorld( foregroundImporter, GetWindowPort( window ), NULL );
	err = GraphicsImportDraw( foregroundImporter );

	pause();
	
	// redraw the background.
	err = GraphicsImportDraw( backgroundImporter );
	
	// draw the foreground using the transparent graphics mode, with white transparent.
	whiteColor.red = 0xffff;
	whiteColor.green = 0xffff;
	whiteColor.blue = 0xffff;
	err = GraphicsImportSetGraphicsMode( foregroundImporter,	// graphics importer instance
										 transparent,			// graphics transfer mode to use for drawing
										 &whiteColor );			// color to use for blending and transparent operations
	err = GraphicsImportDraw( foregroundImporter );

	pause();
	
	// redraw the background.
	err = GraphicsImportDraw( backgroundImporter );
	
	// draw the foreground using the straight alpha graphics mode.
	err = GraphicsImportSetGraphicsMode( foregroundImporter,		// graphics importer instance
										 graphicsModeStraightAlpha,	// graphics transfer mode to use for drawing
										 NULL );
	err = GraphicsImportDraw( foregroundImporter );

	pause();
	
	// redraw the background.
	err = GraphicsImportDraw( backgroundImporter );
	
	// draw the foreground *rotated 30 degrees*, using the straight alpha graphics mode.
	err = GraphicsImportGetMatrix( foregroundImporter, &matrix );
	backgroundCenter.x = Long2Fix( backgroundBounds.right - backgroundBounds.left ) / 2;
	backgroundCenter.y = Long2Fix( backgroundBounds.bottom - backgroundBounds.top ) / 2;
	RotateMatrix( &matrix, Long2Fix(30), backgroundCenter.x, backgroundCenter.y );
	err = GraphicsImportSetMatrix( foregroundImporter, &matrix );
	err = GraphicsImportDraw( foregroundImporter );

bail:
	if ( backgroundImporter ) CloseComponent( backgroundImporter );
	if ( foregroundImporter ) CloseComponent( foregroundImporter );
	if ( hOpenTypeList)	DisposeHandle( hOpenTypeList );
}