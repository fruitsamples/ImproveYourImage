/*
	File:		MovieToImages.c
	
	Description: This example opens a movie file, allows you to save the movie as a sequence of
				 frames. It then creates a new reference movie from those images and saves it.
				 Sample Movie File : Claymation.mov in the Sample 8-Movies <-> images folder.

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

void MovieToImage( void )
{
	OSErr  err = noErr;
	Handle hOpenTypeList = NewHandle(0);
	long   numTypes = 0;
	FSSpec theFSSpec;
	Boolean isSelected, isReplacing;
	Rect naturalBounds, windowBounds;
	short refNum = -1, resID = 0;
	Movie movie = 0;
	MovieExportComponent movieExporter = 0;
	Boolean canceled = true;
	MovieImportComponent movieImporter = 0;
	Track usedTrack;
	TimeValue addedDuration;
	long outFlags;
	
    if ( !BuildMovieValidFileTypes( hOpenTypeList, &numTypes ) ) {
        HLock( hOpenTypeList );
    } else {
        DisposeHandle( hOpenTypeList );
        return;
    }

	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
    DisposeHandle( hOpenTypeList );
	if ( err ) return;
	
	// open a movie file
	err = OpenMovieFile( &theFSSpec, &refNum, fsRdPerm );
	err = NewMovieFromFile( &movie, refNum, &resID, NULL, newMovieActive, NULL );
	err = CloseMovieFile( refNum );
	refNum = -1;
	
	GetMovieBox( movie, &naturalBounds );
	windowBounds = naturalBounds;
	OffsetRect( &windowBounds, 10, 45 );
	window = NewCWindow( NULL, &windowBounds, "\pMovie <-> Images", true, documentProc, (WindowPtr)-1, true, 0);
	SetPortWindowPort( window );
	
	// play the movie
	SetMovieGWorld( movie, GetWindowPort( window), NULL );
	GoToBeginningOfMovie( movie );
	StartMovie( movie );
	while( false == IsMovieDone( movie ) ) {
		MoviesTask( NULL, 0 );
	}
	StopMovie( movie );
	
	pause();

	// save the movie as a sequence of image files.
	err = PutFile( "\pSave as image sequence:", "\psequence", &theFSSpec, &isSelected, &isReplacing);
	if (err) return;
	
	// open a movie exporter
	err = OpenADefaultComponent( MovieExportType,				// component type
								 GraphicsExporterComponentType, // subType
								 &movieExporter );				// component instance
	
	// configure the export component by showing the dialog box
	err = MovieExportDoUserDialog( movieExporter,				// component instance
								   movie,						// movie to be exported					
								   NULL,						// specific track to export
								   0,							// start time
								   GetMovieDuration( movie ),	// duration to be exported
								   &canceled );					// boolean set to true if user canceled
	if( canceled ) return;
	
	// attach a progress function; -1 for default progress function
	SetMovieProgressProc( movie, (MovieProgressUPP)-1, 0 );
	
	// convert the movie into a specified file(s) and type
	err = ConvertMovieToFile( movie,			// movie specifier
							  NULL,				// specific track for export; NULL for all tracks
							  &theFSSpec,		// output file
							  0,				// file type				
							  0,				// file creator
							  smSystemScript,	// script 
							  NULL,				// resource id
							  0,				// flags
							  movieExporter );	// movie export component to use for the operation -- pass in a component instance
							  					// this allows setting any conversion parameters with the export component directly
	
	CloseComponent( movieExporter );
	DisposeMovie( movie );
	movie = 0;
	EraseRect( &naturalBounds );
	
    hOpenTypeList = NewHandle(0);
	// now re-open the sequence of image files as a reference movie
	if ( !BuildGraphicsImporterValidFileTypes( hOpenTypeList, &numTypes ) ) {
        HLock( hOpenTypeList );
    } else {
        DisposeHandle( hOpenTypeList );
        return;
    }

	err = GetOneFileWithPreview(numTypes, (OSTypePtr)*hOpenTypeList, &theFSSpec, NULL);
	DisposeHandle( hOpenTypeList );
	if ( err ) return;

	// open a movie importer
	err = OpenADefaultComponent( MovieImportType,				// component type
								 GraphicsImporterComponentType, // subType
								 &movieImporter );				// component instance
	
	// we're using a movie importer that imports images using graphics importers
	// this subType-specific call enables the "Import Image Sequence..." behaviour,
	// letting the movie importer search for adjacent files with names matching a numeric pattern.					 
	GraphicsImageImportSetSequenceEnabled( movieImporter, true );
	canceled = true;
	
	// configure the import component by showing the user dialog box
	err = MovieImportDoUserDialog( movieImporter,	// component instance
								   &theFSSpec,		// source file; NULL for handle
								   NULL,			// data; NULL for file
								   &canceled );		// boolean set to true if user canceled
	if( canceled ) return;
	
	// create the new movie
	movie = NewMovie(newMovieActive);
	
	// do the import of the sequence
	err = MovieImportFile( movieImporter,	// movie importer component instance
						   &theFSSpec,		// data file
						   movie,			// the movie to recieve the data
						   NULL,			// specific target track
						   &usedTrack,		// pointer to track that received the imported data
						   0,				// time to place imported data 
						   &addedDuration,	// the duration of the data added to the movie
						   0,				// in flags
						   &outFlags );		// out flags
	CloseComponent( movieImporter );

	// play the reference movie.
	SetMovieGWorld(movie, GetWindowPort(window), NULL);
	GoToBeginningOfMovie( movie );
	StartMovie( movie );
	while( false == IsMovieDone( movie ) ) {
		MoviesTask( NULL, 0 );
	}
	StopMovie( movie );
	
	// save the reference movie.
	err = PutFile( "\pSave as reference movie:", "\preference movie.mov", &theFSSpec, &isSelected, &isReplacing);
	if (err) return;

	// create a movie file; has options controled by flags
	err = CreateMovieFile( &theFSSpec,						// file specifier
						   FOUR_CHAR_CODE('TVOD'),			// creator
						   smSystemScript,					// script
						   createMovieFileDeleteCurFile 	// flags
						 | createMovieFileDontOpenFile
						 | createMovieFileDontCreateResFile
						 | createMovieFileDontCreateMovie,
						   0,								// resRefNum; 0 to not open file 
						   0 );								// newMovie; 0 not to create movie
	// open the movie
	err = OpenMovieFile( &theFSSpec, &refNum, fsRdWrPerm );
	resID = movieInDataForkResID;
	
	// add the movie resource to the file
	err = AddMovieResource( movie, refNum, &resID, NULL );
	err = CloseMovieFile( refNum );
	
	UpdateMovie( movie );
	MoviesTask( NULL, 0 );
	DisposeMovie( movie );
}