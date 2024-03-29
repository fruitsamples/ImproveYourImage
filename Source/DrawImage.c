/*
	File:		DrawImage.c
	
	Description: This sample simply draws an image from a file using Graphics Importers.
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

void DrawImage( void )
{
	OSErr  err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	Rect   bounds;
	GraphicsImportComponent importer = 0;
	
	BuildGraphicsImporterValidFileTypes( hOpenTypeList, &numTypes );
	HLock( hOpenTypeList );

	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
	DisposeHandle( hOpenTypeList );
	if ( err ) return;
	
	// locate and open a graphics importer component which can be used to draw the
	// selected file. If a suitable importer is not found the ComponentInstance
	// is set to NULL.
	err = GetGraphicsImporterForFile( &theFSSpec,	// specifies the file to be drawn
									  &importer );	// pointer to the returned GraphicsImporterComponent
		
	// get the native size of the image associated with the importer							  
	err = GraphicsImportGetNaturalBounds( importer, 	// importer instance
										  &bounds );	// returned bounds
	
	OffsetRect( &bounds, 10, 45 );
	window = NewCWindow( NULL, &bounds, "\pDraw Image", true, documentProc, (WindowPtr)-1, true, 0);
	
	// set the graphics port for drawing
	err = GraphicsImportSetGWorld( importer,				// importer instance
								   GetWindowPort( window ),	// destination graphics port or GWorld
								   NULL );					// destination GDevice, set to NULL uses GWorlds device
	
	// draw the image
	err = GraphicsImportDraw( importer );
	
	// close the importer instance
	CloseComponent( importer );
}