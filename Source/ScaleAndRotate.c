/*
	File:		ScaleAndRotate.c
	
	Description: This example uses graphics importers to draw an image then demonstrates
	             how to perform scaling, rotation and perspective manipulation using matrix
	             operations.
	             Test Sample Image : Sample 1-Snowgum.jpg

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

void ScaleAndRotate( void )
{
	OSErr  err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	GraphicsImportComponent importer = 0;
	Rect naturalBounds, windowBounds, scaledBounds;
	MatrixRecord matrix;
	Fixed naturalWidth, naturalHeight;
	FixedPoint fromQuad[4], toQuad[4];

	BuildGraphicsImporterValidFileTypes( hOpenTypeList, &numTypes );
	HLock( hOpenTypeList );
	
	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
	DisposeHandle( hOpenTypeList );
	if ( err ) return;
	
	// locate and open a graphics importer component
	err = GetGraphicsImporterForFile( &theFSSpec, &importer );
	
	// get the native size of the image associated with the importer
	err = GraphicsImportGetNaturalBounds( importer, &naturalBounds );
	
	windowBounds = naturalBounds;
	OffsetRect( &windowBounds, 10, 45 );
	window = NewCWindow( NULL, &windowBounds, "\pScale and Rotate ", true, documentProc, (WindowPtr)-1, true, 0);
	
	// set the graphics port for drawing
	err = GraphicsImportSetGWorld( importer, GetWindowPort( window ), NULL );
	
	// draw the image
	err = GraphicsImportDraw( importer );
	
	pause();
	
	// scale the image by a factor of 2x.  
	// the top-left coordinate of an image's natural bounds is always (0,0).
	scaledBounds = naturalBounds;
	scaledBounds.right = scaledBounds.right * 2;
	scaledBounds.bottom = scaledBounds.bottom * 2;
	
	// define the rectangle in which to draw an image
	err = GraphicsImportSetBoundsRect( importer,		// importer instance
									   &scaledBounds ); // desired bounds
	
	// resize the window to fit, and redraw.
	SizeWindow( window, scaledBounds.right, scaledBounds.bottom, false );
	err = GraphicsImportDraw( importer );
	
	pause();
	
	// rotate the image ninety degrees clockwise
	// set the contents of a matrix so that it performs no transformation
	SetIdentityMatrix( &matrix );
	
	// modify the contents of a matrix so that it defines a rotation operation
	// 90 degrees to the right - anchor at 0.0 top-left 
	RotateMatrix( &matrix,			// pointer to the matrix structure
				  Long2Fix( 90 ),	// the number of degrees of rotation NOTE: THIS IS A FIXED VALUE
				  0,				// x coordinate of anchor point
				  0 );				// y coordinate of anchor point
	
		  
	// we need to return the top-left corner of the rotated image back to it's
	// origin, so add a translation value to a specified matrix 
	TranslateMatrix( &matrix,							// pointer to the matrix structure
					 Long2Fix( naturalBounds.bottom ),	// deltaH - value added to the x coordinate NOTE: FIXED VALUES
					 0 );								// deltaV - value added to the y coordinate 
	
	// set the transformation matrix to use for drawing an image			 
	err = GraphicsImportSetMatrix( importer, &matrix );
	
	// resize the window to fit, and redraw.
	// get the new bounds rect, size the window then draw
	err = GraphicsImportGetBoundsRect( importer, &scaledBounds );
	SizeWindow( window, scaledBounds.right, scaledBounds.bottom, false );
	err = GraphicsImportDraw( importer );
	
	pause();
	
	// give the image some hefty perspective.
	// set up two quadrilateral coordinates and create a translation matrix
	// from one to the other using QuadToQuadMatrix()
	// QuadToQuadMatrix was added in QuickTime 4.0.
	naturalWidth = Long2Fix( naturalBounds.right );
	naturalHeight = Long2Fix( naturalBounds.bottom );
	fromQuad[0].x = 0;
	fromQuad[0].y = 0;
	fromQuad[1].x = naturalWidth;
	fromQuad[1].y = 0;
	fromQuad[2].x = naturalWidth;
	fromQuad[2].y = naturalHeight;
	fromQuad[3].x = 0;
	fromQuad[3].y = naturalHeight;
	toQuad[0].x = naturalWidth/8;
	toQuad[0].y = naturalHeight/4;
	toQuad[1].x = naturalWidth*9/8;
	toQuad[1].y = 0;
	toQuad[2].x = naturalWidth;
	toQuad[2].y = naturalHeight*6/4;
	toQuad[3].x = 0;
	toQuad[3].y = naturalHeight;
	SetIdentityMatrix( &matrix );
	QuadToQuadMatrix( (Fixed *)fromQuad, (Fixed *)toQuad, &matrix );
	err = GraphicsImportSetMatrix( importer, &matrix );
	
	// resize the window to fit, and redraw
	// get the new bounds rect, size the window then draw
	err = GraphicsImportGetBoundsRect( importer, &scaledBounds );
	SizeWindow( window, scaledBounds.right, scaledBounds.bottom, false );
	SetPortWindowPort( window );
	EraseRect( &scaledBounds );
	err = GraphicsImportDraw( importer );

	CloseComponent( importer );
}