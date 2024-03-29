/*
	File:		GetingMoreInfo.c
	
	Description: This example demonstrates how to display multiple images.
				 Sample Test Image : Sample 5-iMac in Australia.psd

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

void MultipleImage( void )
{
	OSErr  err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	GraphicsImportComponent importer = 0;
	Rect naturalBounds, windowBounds;
	unsigned long imageCount, imageIndex;
	ImageDescriptionHandle desc = NULL;
	MatrixRecord matrix;

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
	OffsetRect( &naturalBounds, 10, 45 );
	window = NewCWindow( NULL, &naturalBounds, "\pMultiple Images", true, documentProc, (WindowPtr)-1, true, 0);
	
	// set the graphics port for drawing
	err = GraphicsImportSetGWorld( importer, GetWindowPort(window), NULL );
	
	// ask the graphics importer how many images there are in this file
	err = GraphicsImportGetImageCount( importer, &imageCount );
	
	for( imageIndex = 1; imageIndex <= imageCount; imageIndex++ ) {	
		// set the index value for the image we want to draw
		err = GraphicsImportSetImageIndex( importer, imageIndex );
		
		// each image in the file can have different dimensions, depth, etc.
		// if the image has an alpha, use StraightAlpha graphics mode to draw
		err = GraphicsImportGetImageDescription( importer, &desc );
		if( (*desc)->depth == 32 )
			err = GraphicsImportSetGraphicsMode( importer, graphicsModeStraightAlpha, NULL );
		else
			err = GraphicsImportSetGraphicsMode( importer, ditherCopy, NULL );
		DisposeHandle( (Handle)desc );
		
		// set up the matrix
		SetIdentityMatrix( &matrix );
		GraphicsImportGetDefaultMatrix( importer, &matrix );
		err = GraphicsImportSetMatrix( importer, &matrix );

		SetPortWindowPort( window );
		EraseRect( &windowBounds );
		
		// draw the image
		err = GraphicsImportDraw( importer );

		pause();
	}
	
	// draw the images again but this type don't erase
	for( imageIndex = 2; imageIndex <= imageCount; imageIndex++ ) {
		err = GraphicsImportSetImageIndex( importer, imageIndex );
		
		err = GraphicsImportGetImageDescription( importer, &desc );
		if( (*desc)->depth == 32 )
			err = GraphicsImportSetGraphicsMode( importer, graphicsModeStraightAlpha, NULL );
		else
			err = GraphicsImportSetGraphicsMode( importer, ditherCopy, NULL );
		DisposeHandle( (Handle)desc );
		
		SetIdentityMatrix( &matrix );
		GraphicsImportGetDefaultMatrix( importer, &matrix );
		err = GraphicsImportSetMatrix( importer, &matrix );

		err = GraphicsImportDraw( importer );

		pause();
	}

	CloseComponent( importer );
}