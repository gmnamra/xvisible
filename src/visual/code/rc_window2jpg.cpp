/*
 *
 *$Id $
 *$Log: $
 *
 */


#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#include <QuickTime/QuickTime.h>
#include <stdio.h>
#include <rc_window.h>
#include "rc_window2jpg.h"

static OSStatus exportCGImageToJPGFile(CGImageRef imageRef, CFStringRef path);

bool rfImageExport2JPG (const rcWindow& image, std::string filePathName)
{
    OSStatus  err = eventNotHandledErr;
	CFStringRef path = CFStringCreateWithCString (NULL, filePathName.c_str (),  kCFStringEncodingMacRoman);
	CGImageRef ir = image.CGImage();
	err = exportCGImageToJPGFile (ir, path);
	return err == noErr;
}

static OSStatus exportCGImageToJPGFile(CGImageRef imageRef, CFStringRef path)
{
    Handle                     dataRef = NULL;
    OSType                     dataRefType;
    GraphicsExportComponent    graphicsExporter;
    unsigned long              sizeWritten;
    ComponentResult            result;
	
    result = QTNewDataReferenceFromFullPathCFString(path,
													kQTNativeDefaultPathStyle, 0, &dataRef, &dataRefType);
    result = OpenADefaultComponent(GraphicsExporterComponentType,
								   kQTFileTypeJPEG, &graphicsExporter);
    result = GraphicsExportSetInputCGImage(graphicsExporter, imageRef);
    result = GraphicsExportSetOutputDataReference(graphicsExporter,
												  dataRef, dataRefType);
    result = GraphicsExportDoExport(graphicsExporter, &sizeWritten);
    CloseComponent(graphicsExporter);
    DisposeHandle(dataRef);
    return result;
}

