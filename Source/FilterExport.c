/*
	File:		FilterExport.c
	
	Description: This example uses graphics importers to draw an image then demonstrates adding a
				 filter effect to the image and using a Graphics Exporter component to export
				 the image as a PNG image.

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

//----------
static pascal Boolean theRequestSettingsFilter( DialogRef inDialog, EventRecord *ioEvent, short *outItemHit, void *inDataPtr );
static pascal Boolean theRequestSettingsFilter( DialogRef inDialog, EventRecord *ioEvent, short *outItemHit, void *inDataPtr )
{
#pragma unused (outItemHit)

	WindowRef		eventWindow;
	Boolean			result = false;
	PixMapHandle	hPixMap = GetGWorldPixMap(static_cast<GWorldPtr>(inDataPtr)); // static_cast for GCC

	if (ioEvent->what == updateEvt) {
		eventWindow = (WindowRef)(ioEvent->message);
		if (eventWindow != GetDialogWindow( inDialog )) {
			// blit the image to the screen	
			CopyBits(GetPortBitMapForCopyBits(static_cast<CGrafPtr>(inDataPtr)),
					 GetPortBitMapForCopyBits(GetWindowPort(eventWindow)),
			 		 &(*hPixMap)->bounds,
			 		 &(*hPixMap)->bounds,
					 ditherCopy,
			 		 NULL);
			 result = true;
		}
	}

	return result;
}

void FilterExport( void )
{
	OSErr  err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	Boolean isSelected, isReplacing;
	GraphicsImportComponent importer = 0;
	Rect naturalBounds, windowBounds;
	GWorldPtr sourceGWorld = NULL;
	ImageDescriptionHandle offscreenDesc = NULL;
	GWorldPtr destGWorld = NULL;
	MatrixRecord matrix;
	ImageSequence imageSequence = 0;
	QTAtomContainer videoFilter = 0;
	QTAtomContainer videoFilterList = 0;
	ImageDescriptionHandle effectDesc = NULL;
	ImageSequenceDataSource effectDataSource = 0;
	QTParameterDialog dialog = 0;
	QTParamPreviewRecord previewParameter = {0, NULL};	// sourceID, picHandle
	long aLong;
	QTAtom effectTypeAtom;
	OSType effectType;
	GraphicsExportComponent exporter = 0;
	
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
	window = NewCWindow( NULL, &windowBounds, "\pFilter Export ", true, documentProc, (WindowPtr)-1, true, 0);
	SetPortWindowPort( window );
	
	// draw the image into an offscreen GWorld
	// create the offscreen
	err = QTNewGWorld( &sourceGWorld,			// returned GWorld
					   k32ARGBPixelFormat,		// pixel format 
					   &naturalBounds,			// bounds 
					   NULL,					// cTable 	
					   NULL, 					// GDevice
					   kICMTempThenAppMemory );	// flags
	LockPixels( GetGWorldPixMap( sourceGWorld ) );
	
	// set the graphics port for drawing
	err = GraphicsImportSetGWorld( importer, sourceGWorld, NULL );
	
	// draw the image to the offscreen
	err = GraphicsImportDraw( importer );

	// blit the offscreen to the window
	CopyBits(GetPortBitMapForCopyBits(sourceGWorld),
			 GetPortBitMapForCopyBits(GetWindowPort(window)),
			 &naturalBounds,
			 &naturalBounds,
			 ditherCopy,
			 NULL);
	
	pause();

	// allocate a second offscreen GWorld to draw the filtered image into
	err = QTNewGWorld( &destGWorld, k32ARGBPixelFormat, &naturalBounds, NULL, NULL, kICMTempThenAppMemory );
	LockPixels( GetGWorldPixMap( destGWorld ) );

	// ask for a list of all the single source effects (ie, filters)
	// this function returns a QTAtomContainer holding a list of the currently installed effects components
	err = QTGetEffectsList( &videoFilterList,	// QTAtomContainer holding a list
							1,					// min number of sources that an effect must have to be added to the list
							1,					// max number of sources that an effect can have to be added to the list
							0 );				// getOptions 0 for all effects except 'none' effect

	// prompt for the user to select a filter
	// create a new atom container to build the effect
	err = QTNewAtomContainer( &videoFilter );
	
	// create the effect dialog box; choose an effect from the list of effects passed to the function
	// standard dialog includes a preview of the effect
	err = QTCreateStandardParameterDialog( videoFilterList,	// list of effects
										   videoFilter,	    // on return, this atom container holds an effect description for the effect selected 
			                               pdOptionsCollectOneValue | // options to control the behavior of the dialog
			                               pdOptionsModalDialogBox,
			                               &dialog );		// returns a reference to the dialog box
	
	// set up a preview for the source
	// creates a new QuickDraw picture handle containing the image currently in use by the graphics import component
	err = GraphicsImportGetAsPicture( importer, &previewParameter.sourcePicture );
	previewParameter.sourceID = 1;
	
	// change some of the default behaviors of the standard parameter dialog box.
	QTStandardParameterDialogDoAction( dialog,						// dialog box reference
									   pdActionSetPreviewPicture,	// action to perform
									   &previewParameter );			// the (optional) parameter to the action
	
	// show the dialog and run it							   	
	// passing a NULL event here causes QuickTime to take over until the dialog is dismissed
	err = QTIsStandardParameterDialogEvent( NULL, dialog );
	QTDismissStandardParameterDialog( dialog );

	// codecParameterDialogConfirm means OK was hit, userCanceledErr means Cancel was hit.
	if( codecParameterDialogConfirm == err )
		err = noErr;
	else goto bail;

	// parse the effect sample to find out what kind of effect it was
	effectTypeAtom = QTFindChildByIndex( videoFilter,			// atom container
										kParentAtomIsContainer,	// parent atom specifier
										kParameterWhatName,		// type of atom to be retrieved
										1,						// index
										NULL );					// atomID; NULL to ignore
	
	// copy the effect type from the effectType atom -- ie. Blur effect is type 'blur'									
	err = QTCopyAtomDataToPtr( videoFilter,			// atom container containing the leaf atom
							   effectTypeAtom,		// leaf atom who's data will be copied
							   false,				// copy fewer bytes than the number of bytes specified by the size parameter?
							   sizeof(effectType), 	// size
							   &effectType,			// target buffer pointer
							   NULL );				// returned actual size; NULL to ignore
	
	// cross platform friendly						   
	effectType = EndianU32_BtoN(effectType);

	// add a source label to the effect sample
	aLong = EndianU32_BtoN('srca');
	err = QTInsertChild( videoFilter,				// specifies the atom container that contains the parent atom
						 kParentAtomIsContainer,	// specifies the parent atom within the atom container
						 kEffectSourceName,			// type of the new atom to be inserted
						 1,							// id of new atom
						 0, 						// index -- 0 will insert at the end of the list
						 sizeof(aLong),				// data size
						 &aLong,					// the data
						 NULL );					// ptr to newly created atom -- NULL to ignore
	HLockHi(videoFilter);

	// make an image description for the effect sample and set its size
	err = MakeImageDescriptionForEffect( effectType,		// four-character code identifying the type of effect to make an image description for
										 &effectDesc );		// returned handle of correctly filled-out image description for the selected effect type
	(*effectDesc)->width = naturalBounds.right;
	(*effectDesc)->height = naturalBounds.bottom;

	// build a decompression sequence for the effect
	SetIdentityMatrix( &matrix );
	err = DecompressSequenceBeginS( &imageSequence,				// sequence id
									effectDesc,					// description
									*videoFilter,				// data
									GetHandleSize(videoFilter), // size of the data buffer
									destGWorld,					// graphics port for the destination image
									NULL,						// graphics device handle
									NULL,						// rect 
									&matrix,					// matrix to use					
									ditherCopy,					// graphics mode
									NULL,						// mask
									0,							// flags
									codecNormalQuality,			// accuracy setting
									NULL );						// codec

	// the offscreen gworld is a data source for the video effect
	err = MakeImageDescriptionForPixMap( GetGWorldPixMap(sourceGWorld), &offscreenDesc );

	// create a new data source for the sequence
	err = CDSequenceNewDataSource( imageSequence,			// sequence identifier returned by the DecompressSequenceBeginS()
								   &effectDataSource,		// returned data source identifier
								   'srca',					// four-character code describing how the input will be used
								   1,						// source input number
								   (Handle)offscreenDesc,	// input data description (ImageDescriptionHandle)
								   NULL,					// transfer procedure
								   0 );						// refCon
	
	// sets data in a new frame to a specific data source
	err = CDSequenceSetSourceData( effectDataSource,		// data source identifier 
								   GetPixBaseAddr(GetGWorldPixMap(sourceGWorld)),	// pointer to data
								   (**offscreenDesc).dataSize );					// data size
	
	// render the effect to the second offscreen
	err = DecompressSequenceFrameS( imageSequence,				// sequence identifier
									*videoFilter,				// source data  
									GetHandleSize(videoFilter),	// data size
									0,							// in flags
									NULL,						// out flags
									NULL );						// async completion function; NULL for synchronous
	
	// blit the now filtered image to the screen		
	CopyBits(GetPortBitMapForCopyBits(destGWorld),
			 GetPortBitMapForCopyBits(GetWindowPort(window)),
			 &naturalBounds,
			 &naturalBounds,
			 ditherCopy,
			 NULL);

	pause();

	// write the filtered image out as a PNG file.
	err = PutFile( "\pSave the filtered image:", "\pfiltered.png", &theFSSpec, &isSelected, &isReplacing);
	if (err) goto bail;

	// find and open the PNG Graphics Exporter component
	err = OpenADefaultComponent( GraphicsExporterComponentType,	// component type
								 kQTFileTypePNG,				// component subType
								 &exporter );					// returned component instance
	
	// set the input for the graphics exporter
	err = GraphicsExportSetInputGWorld( exporter,		// component instance
										destGWorld );	// input source
	
	// set the destination for the export								
	err = GraphicsExportSetOutputFile( exporter,
									   &theFSSpec );	// export destination
	
	// the depth to use for the exported image
	err = GraphicsExportSetDepth( exporter,
								  k24RGBPixelFormat );	// depth
	
	// do the export
	err = GraphicsExportDoExport( exporter,
								  NULL );		// actual size written; NULL to ignore
	
	// blit the image to the screen	
	CopyBits(GetPortBitMapForCopyBits(destGWorld),
			 GetPortBitMapForCopyBits(GetWindowPort(window)),
			 &naturalBounds,
			 &naturalBounds,
			 ditherCopy,
			 NULL);
	
	CloseComponent ( exporter );
	
	pause();
	
	// write out the image again as a .jpg using the settings dialog
	OpenADefaultComponent( GraphicsExporterComponentType, kQTFileTypeJPEG, &exporter );
	
	// does the export component supports the exporter setting dialog?
	if ( CallComponentCanDo( exporter , kGraphicsExportRequestSettingsSelect )) {
	
		// set the input for the graphics exporter
		err = GraphicsExportSetInputGWorld( exporter,		// component instance
											destGWorld );	// input source
		
		// get the settings using the settings dialog
		err = GraphicsExportRequestSettings( exporter,
									   		 NewModalFilterYDUPP( theRequestSettingsFilter ),
									   		 destGWorld );
									   		 
		err = PutFile( "\pSave the filtered image:", "\pfiltered.jpg", &theFSSpec, &isSelected, &isReplacing);
		if (err) goto bail;		

		// set the destination for the export								
		err = GraphicsExportSetOutputFile( exporter,
									   	   &theFSSpec );	// export destination		
		// do the export
		err = GraphicsExportDoExport( exporter,
								  	  NULL );				// actual size written; NULL to ignore
	}
	
	// blit the image to the screen	
	CopyBits(GetPortBitMapForCopyBits(destGWorld),
			 GetPortBitMapForCopyBits(GetWindowPort(window)),
			 &naturalBounds,
			 &naturalBounds,
			 ditherCopy,
			 NULL);	
bail:
	// clean up
	if ( imageSequence ) 	CDSequenceEnd( imageSequence );
	if ( videoFilter ) 		QTDisposeAtomContainer( videoFilter );
	if ( videoFilterList ) 	QTDisposeAtomContainer( videoFilterList );
	if ( offscreenDesc)		DisposeHandle( (Handle)offscreenDesc );
	if ( effectDesc )		DisposeHandle( (Handle)effectDesc );
	if ( importer )			CloseComponent( importer );
	if ( exporter )			CloseComponent( exporter );
	if ( sourceGWorld )		DisposeGWorld( sourceGWorld );
	if ( destGWorld )		DisposeGWorld( destGWorld );
	if ( previewParameter.sourcePicture ) KillPicture( previewParameter.sourcePicture );
}