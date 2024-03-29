/*
	File:		DeepImages.c
	
	Description: This example demonstrates QuickTimes support for 16bit deep components by rendering
	             an image containing 48bit and 24bit RGB data. It then allows you to keep doubling the
	             brightness of every pixel pinning the value at white.
				 Sample Test Image : Sample 9-48RGB vs 24RGB.tiff

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

void DeepImages( void )
{
	OSErr err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	GraphicsImportComponent importer = 0;
	Rect naturalBounds, windowBounds;
	ImageDescriptionHandle desc = NULL, offscreenDesc = NULL;
	Handle h = NULL;
	OSType pixelFormat;
	GWorldPtr gworld = NULL;
	PixMapHandle pixmap;
	MatrixRecord matrix;
	ImageSequence imageSequence = 0;
	
	BuildGraphicsImporterValidFileTypes( hOpenTypeList, &numTypes );
	HLock( hOpenTypeList );

	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
	DisposeHandle( hOpenTypeList );
	if ( err ) return;
	
	// locate and open a graphics importer component
	err = GetGraphicsImporterForFile( &theFSSpec, &importer );

	// Find out the real colorspace.
	err = GraphicsImportGetImageDescription( importer, &desc );
	err = GetImageDescriptionExtension( desc, &h, kImageDescriptionColorSpace, 1 );
	if( !h || !*h ) return;
	pixelFormat = *(OSType*)*h;
	pixelFormat = EndianU32_BtoN(pixelFormat);
	
	// Draw the image into an offscreen gworld of that colorspace.
	err = GraphicsImportGetNaturalBounds( importer, &naturalBounds );
	err = QTNewGWorld( &gworld, pixelFormat, &naturalBounds, NULL, NULL, kICMTempThenAppMemory );
	pixmap = GetGWorldPixMap( gworld );
	LockPixels( pixmap );
	err = GraphicsImportSetGWorld( importer, gworld, NULL );
	err = GraphicsImportDraw( importer );
	
	windowBounds = naturalBounds;
	OffsetRect( &windowBounds, 10, 45 );
	window = NewCWindow( NULL, &windowBounds, "\pDeep Images", true, documentProc, 
			(WindowPtr)-1, true, 0);
	
	// Copy the image from the offscreen to the window using an image decompression sequence.
	err = MakeImageDescriptionForPixMap( pixmap, &offscreenDesc );

	SetIdentityMatrix( &matrix );
	
	err = DecompressSequenceBegin( &imageSequence, offscreenDesc, GetWindowPort(window), NULL, NULL, 
			&matrix, ditherCopy, NULL, 0, codecNormalQuality, NULL );

	err = DecompressSequenceFrameS( imageSequence, GetPixBaseAddr( pixmap ),
			(**offscreenDesc).dataSize, 0, NULL, NULL );

	if( k48RGBCodecType == pixelFormat ) {
		short i;
		long rowBytes, width, height, x, y;
		Ptr baseAddr, rowPtr;
		
		rowBytes = QTGetPixMapHandleRowBytes( pixmap );
		baseAddr = GetPixBaseAddr( pixmap );
		width = naturalBounds.right;
		height = naturalBounds.bottom;
		
		for( i = 1; i <= 8; i++ ) {
			pause();
	
			// Double the brightness of every pixel.  Pin at white.
			rowPtr = baseAddr;
			for( y = 0; y < height; y++ ) {
				UInt16 channel, *channelPtr = (UInt16 *)rowPtr;
				for( x = 0; x < width; x++ ) {
					channel = EndianU16_BtoN(*channelPtr);
					channel = (channel >= 0x8000UL) ? 0xffff : (channel * 2);
					*channelPtr++ = EndianU16_NtoB(channel);
					channel = EndianU16_BtoN(*channelPtr);
					channel = (channel >= 0x8000UL) ? 0xffff : (channel * 2);
					*channelPtr++ = EndianU16_NtoB(channel);
					channel = EndianU16_BtoN(*channelPtr);
					channel = (channel >= 0x8000UL) ? 0xffff : (channel * 2);
					*channelPtr++ = EndianU16_NtoB(channel);
				}
				rowPtr += rowBytes;
			}
			
			err = DecompressSequenceFrameS( imageSequence, GetPixBaseAddr( pixmap ),
						(**offscreenDesc).dataSize, 0, NULL, NULL );
		}
	}
	
	CDSequenceEnd( imageSequence );
	DisposeHandle( (Handle)desc );
	DisposeHandle( (Handle)offscreenDesc );
	DisposeHandle( h );
	if( gworld ) DisposeGWorld( gworld );
	CloseComponent( importer );
}