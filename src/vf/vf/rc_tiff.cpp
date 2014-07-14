/*
*
	*$Id $
	*$Log$
	*Revision 1.3  2005/12/07 00:04:17  arman
	*more tiff
	*
	*Revision 1.2  2005/10/27 22:58:19  arman
	*z additions and cleanup
	*
	*Revision 1.1  2005/10/25 22:37:20  arman
	*tiff support
	*
	*
	*
	* Copyright (c) 2002 Reify Corp. All rights reserved.
*/
#include "rc_tiff.h"
#include <iostream>
#include <fstream>
#include "rc_systeminfo.h"
using namespace std;
static const char *photoNames[] = {
	"min-is-white",				/* PHOTOMETRIC_MINISWHITE */
		"min-is-black",				/* PHOTOMETRIC_MINISBLACK */
		"RGB",				        /* PHOTOMETRIC_RGB */
		"paletted",	                /* PHOTOMETRIC_PALETTE */
		"transparency mask",		/* PHOTOMETRIC_MASK */
		"separated",				/* PHOTOMETRIC_SEPARATED */
		"YCbCr",					/* PHOTOMETRIC_YCBCR */
		"?",
		"CIE L*a*b*",				/* PHOTOMETRIC_CIELAB */
	};

#define	NPHOTONAMES	(sizeof (photoNames) / sizeof (photoNames[0]))



ostream& operator<< (ostream& ous, const TIFFReaderInternal& tim)
{
	if (tim.Image)
	{
		ous << " Pages" << "\t" << tim.NumberOfPages << endl;
		ous << "Width"                << "\t" <<   tim.Width << endl;
		ous << "Height"                   << "\t" << tim.Height << endl;
		ous << " SamplesPerPixel"   << "\t" <<       tim.spp << endl;
		ous << " Compression"        << "\t" <<      tim.Compression <<  endl;
		ous << " BitsPerSample"      << "\t" <<      tim.bps <<  endl;
		const char* ps =  (tim.Photometrics < NPHOTONAMES) ? photoNames[tim.Photometrics] : "?"; 
		ous << " Photometrics"        << "\t" << ps << endl;

		ps  = (tim.SampleFormat == SAMPLEFORMAT_UINT ) ? "unsigned"
			: (tim.SampleFormat == SAMPLEFORMAT_INT )  ? "signed"
			: (tim.SampleFormat == SAMPLEFORMAT_IEEEFP ) ? "float" : "<unknown>";
		ous << "SampleFormat"         << "\t" << ps << endl;

		ps = ((tim.PlanarConfig != PLANARCONFIG_CONTIG) && (tim.spp>1)) ? ", organised in separated planes" : " planar";
		ous << " PlanarConfig"         << "\t" << ps << endl;

#ifdef CPP11FIXED // @note c++11 issue
        //	ous << " Orientation"           << "\t" <<   tim.Orientation <<    
			ous << "TileDepth"              << "\t" <<  tim.TileDepth <<       
			ous << " TileRows"              << "\t" <<   tim.TileRows <<       
			ous << " TileColumns"         << "\t" <<     tim.TileColumns <<    
			ous << " TileWidth"             << "\t" <<   tim.TileWidth <<      
			ous << " TileHeight"            << "\t" <<   tim.TileHeight <<     
			ous << "NumberOfTiles"      << "\t" <<      tim.NumberOfTiles <<  endl;
#endif        
		return ous;
	}
    return ous;
}

void TIFFReaderInternal::Clean()
{
	if ( Image )
	{
		TIFFClose(Image);
	}
	Image=NULL;
	Width = 0;
	Height = 0;
	spp = 1;
	Compression = 0;
	bps = 1;
	Photometrics = PHOTOMETRIC_MINISBLACK;
	PlanarConfig = PLANARCONFIG_CONTIG;
	SampleFormat = SAMPLEFORMAT_UINT;
	TileDepth = 0;
	CurrentPage = 0;
	NumberOfPages = 1;
	XResolution = 0.0;
	NumberOfTiles = 0;
	TileRows = 0;
	TileColumns = 0;
	TileWidth = 0;
	TileHeight = 0;
	mIsSTKfile = false;
}

TIFFReaderInternal::TIFFReaderInternal()
{
	Image           = NULL;
	Clean();
}

int TIFFReaderInternal::Initialize()
{
	if ( Image )
	{
		if ( !TIFFGetField(Image, TIFFTAG_IMAGEWIDTH, &this->Width) ||
			!TIFFGetField(Image, TIFFTAG_IMAGELENGTH, &this->Height) )
		{
			return 0;
		}

// If the number of pages is still zero we look if the image is tiled
		if(TIFFIsTiled(Image))
		{
			NumberOfTiles = TIFFNumberOfTiles(Image);

			if ( !TIFFGetField(Image,TIFFTAG_TILEWIDTH,&this->TileWidth)
				|| !TIFFGetField(Image,TIFFTAG_TILELENGTH,&this->TileHeight)
				)
			{
				std::cout << "Error: Cannot read tile width and tile length from file" << std::endl;
			}
			else
			{
				TileRows = Height/TileHeight;
				TileColumns = Width/TileWidth;
			}
		}



		TIFFGetFieldDefaulted(Image, TIFFTAG_ORIENTATION, &this->Orientation);
		TIFFGetField(Image, TIFFTAG_SAMPLESPERPIXEL, &this->spp);
		TIFFGetField(Image, TIFFTAG_COMPRESSION, &this->Compression);
		TIFFGetField(Image, TIFFTAG_BITSPERSAMPLE, &this->bps);
		TIFFGetField(Image, TIFFTAG_PHOTOMETRIC, &this->Photometrics);
		TIFFGetField(Image, TIFFTAG_PLANARCONFIG, &this->PlanarConfig);
		TIFFGetField(Image, TIFFTAG_SAMPLEFORMAT,    &this->SampleFormat);

		if ( !TIFFGetField(Image, TIFFTAG_TILEDEPTH, &this->TileDepth) )
		{
			TileDepth = 0;
		}
	}

	return 1;
}

int TIFFReaderInternal::CanRead()
{
	return ( Image && ( Width > 0 ) && ( Height > 0 ) &&
		( spp == 1 || spp == 4 || spp == 3) && 
		( Compression == COMPRESSION_NONE || Compression == COMPRESSION_PACKBITS) &&
		( Photometrics == PHOTOMETRIC_RGB ||
		Photometrics == PHOTOMETRIC_MINISWHITE ||
		Photometrics == PHOTOMETRIC_MINISBLACK ||
		Photometrics == PHOTOMETRIC_PALETTE ) &&
		PlanarConfig == PLANARCONFIG_CONTIG &&
		( !TileDepth ) && ( bps == 8   || bps == 16)   );
}


int TIFFReaderInternal::Open( const char *filename )
{
	Clean();
	struct stat fs;
	if ( stat(filename, &fs) )
	{
		return 0;
	}

// TIFFOpen also reads the first directory
	Image = TIFFOpen(filename, "r");

	if ( !Image)
	{
		Clean();
		return 0;
	}
	if ( !Initialize() )
	{
		Clean();
		return 0;
	}



	TIFF* tif = Image;
	TIFFDirectory *td;
	td = &tif->tif_dir;

	rmAssert (NumberOfPages == 1);

	for (int32 cf = 0; cf < td->td_customValueCount; cf++)
	{
		const TIFFFieldInfo *fip = td->td_customValues[cf].info;
		if (!mIsSTKfile)
		{
			mIsSTKfile = fip->field_tag == uic2tag || fip->field_tag == uic3tag || fip->field_tag == uic4tag;
			if (mIsSTKfile)
				NumberOfPages = td->td_customValues[cf].count;
		}
	}

//	cerr << *this << endl;

        
/*
	** Custom tag support.
	*/
	{
		int  i;
		short count;

		count = (short) TIFFGetTagListCount(tif);
		for(i = 0; i < count; i++)
		{
			ttag_t  tag = TIFFGetTagListEntry(tif, i);
			const TIFFFieldInfo *fip;
			uint16 value_count;
			int j, mem_alloc = 0;
			void *raw_data;

			fip = TIFFFieldWithTag(tif, tag);
			if(fip == NULL)
				continue;

			if(fip->field_passcount) {
				if(TIFFGetField(tif, tag, &value_count, &raw_data) != 1)
					continue;
			} else {
				if (fip->field_readcount == TIFF_VARIABLE
					|| fip->field_readcount == TIFF_VARIABLE2)
					value_count = 1;
				else if (fip->field_readcount == TIFF_SPP)
					value_count = td->td_samplesperpixel;
				else
					value_count = fip->field_readcount;
				if (fip->field_type == TIFF_ASCII
					|| fip->field_readcount == TIFF_VARIABLE
					|| fip->field_readcount == TIFF_VARIABLE2
					|| fip->field_readcount == TIFF_SPP
				|| value_count > 1) {
					if(TIFFGetField(tif, tag, &raw_data) != 1)
						continue;
				} else {
					raw_data = _TIFFmalloc(
						_TIFFDataSize(fip->field_type)
						* value_count);
					mem_alloc = 1;
					if(TIFFGetField(tif, tag, raw_data) != 1)
						continue;
				}
			}

			fprintf(stderr, "  %s: ", fip->field_name);

			for(j = 0; j < value_count; j++) {
				if(fip->field_type == TIFF_BYTE)
					fprintf(stderr, "%u",
					(unsigned int) ((uint16 *) raw_data)[j]);
				else if(fip->field_type == TIFF_UNDEFINED)
					fprintf(stderr, "0x%x",
					(unsigned int) ((unsigned char *) raw_data)[j]);		    else if(fip->field_type == TIFF_SBYTE)
					fprintf(stderr, "%d", (int) ((uint16 *) raw_data)[j]);
				else if(fip->field_type == TIFF_SHORT)
					fprintf(stderr, "%u",
					(unsigned int)((unsigned short *) raw_data)[j]);
				else if(fip->field_type == TIFF_SSHORT)
					fprintf(stderr, "%d", (int)((short *) raw_data)[j]);
				else if(fip->field_type == TIFF_LONG)
					fprintf(stderr, "%lu",
					(unsigned long)((unsigned long *) raw_data)[j]);
				else if(fip->field_type == TIFF_SLONG)
					fprintf(stderr, "%ld", (long)((long *) raw_data)[j]);
				else if(fip->field_type == TIFF_RATIONAL
					|| fip->field_type == TIFF_SRATIONAL
					|| fip->field_type == TIFF_FLOAT)
					fprintf(stderr, "%f", ((float *) raw_data)[j]);
				else if(fip->field_type == TIFF_IFD)
					fprintf(stderr, "0x%x",
					(int)((unsigned long *) raw_data)[j]);
				else if(fip->field_type == TIFF_ASCII) {
					fprintf(stderr, "%s", (char *) raw_data);
					break;
				}
				else if(fip->field_type == TIFF_DOUBLE)
					fprintf(stderr, "%f", ((double *) raw_data)[j]);
				else if(fip->field_type == TIFF_FLOAT)
					fprintf(stderr, "%f", ((float *)raw_data)[j]);
				else {
					fprintf(stderr, "<unsupported data type in TIFFPrint>");
					break;
				}

				if(j < value_count - 1)
					fprintf(stderr, ",");
			}
			fprintf(stderr, "\n");
			if(mem_alloc)
				_TIFFfree(raw_data);
		}
	}

	return 1;
}



TIFFImageIO::TIFFImageIO()
{
	SetNumberOfDimensions(2);
	m_PixelType = SCALAR;
	m_ComponentType = UCHAR;

	InitializeColors();
	mInternalImage = new TIFFReaderInternal;

	m_Spacing[0] = 1.0;
	m_Spacing[1] = 1.0;

	m_Origin[0] = 0.0;
	m_Origin[1] = 0.0;

	m_Compression = TIFFImageIO::PackBits;
	m_UseCompression = true;
	mRCdepth =  rcPixelUnknown;
	mFileBO = eByteOrderUnknown;

}

void TIFFImageIO::InitializeColors()
{
	ColorRed    = 0;
	ColorGreen  = 0;
	ColorBlue   = 0;
	TotalColors = -1;  
	ImageFormat = TIFFImageIO::NOFORMAT;
}

int32 TIFFImageIO::width () const
{
	return (int32) mInternalImage->Width;
}

int32 TIFFImageIO::height () const
{
	return (int32) mInternalImage->Height;
}

rcPixel TIFFImageIO::rcDepth () const
{
	return mRCdepth;
}

reByteOrder TIFFImageIO::byteOrder () const
{
	return mFileBO;
}

void TIFFImageIO::SetNumberOfComponents(unsigned int c)
{
  m_NumberOfComponents = c;
}

unsigned int TIFFImageIO::GetNumberOfComponents() const 
{
  return m_NumberOfComponents;
}


void TIFFImageIO::SetPixelType (IOPixelType pt)
{
  m_PixelType = pt;
}


TIFFImageIO::IOComponentType TIFFImageIO::GetComponentType ()
{
  return m_ComponentType;
}

  void TIFFImageIO::SetIORegion (const ImageIORegion _arg)
{
  if (this->m_IORegion != _arg)
    this->m_IORegion = _arg;
}

const ImageIORegion& TIFFImageIO::GetIORegion ()
{
  return m_IORegion;
}

void TIFFImageIO::ReadImageInformation()
{
	m_Spacing[0] = 1.0;  // We'll look for TIFF pixel size information later,
	m_Spacing[1] = 1.0;  // but set the defaults now

	m_Origin[0] = 0.0;
	m_Origin[1] = 0.0;

	m_Dimensions[0] = width ();
	m_Dimensions[1] = height ();

	switch ( GetFormat() )
	{
		case TIFFImageIO::GRAYSCALE:
		case TIFFImageIO::PALETTE_GRAYSCALE:
		SetNumberOfComponents( 1 );
		SetPixelType(SCALAR);
		break;
		case TIFFImageIO::RGB_:      
		SetNumberOfComponents( mInternalImage->spp );
		SetPixelType(RGB);
		break;
		case TIFFImageIO::PALETTE_RGB:      
		SetNumberOfComponents( 3 );
		SetPixelType(RGB);
		break;
		default:
		SetNumberOfComponents( 4 );
		SetPixelType(RGBA);
	}

	if ( !mInternalImage->CanRead() )
	{
		SetNumberOfComponents( 4 );
		SetPixelType(RGBA);
	}

	m_ComponentType = (mInternalImage->bps <= 8) ? UCHAR : USHORT;


// if the tiff file is multi-pages
	if(mInternalImage->NumberOfPages>0)
	{
		SetNumberOfDimensions(3);
		m_Dimensions[2] = mInternalImage->NumberOfPages;
		m_Spacing[2] = 1.0;
		m_Origin[2] = 0.0;
	}

// if the tiff is tiled
	if(mInternalImage->NumberOfTiles>0)
	{
		SetNumberOfDimensions(3);
		m_Dimensions[0] = mInternalImage->TileWidth;
		m_Dimensions[1] = mInternalImage->TileHeight;
		m_Dimensions[2] = mInternalImage->NumberOfTiles;
		m_Spacing[2] = 1.0;
		m_Origin[2] = 0.0;
		mIsTiled = true;
	}

	//@note set the rcPixel size that matched our reading
	mRCdepth = (mInternalImage->bps == 8 && mInternalImage->spp == 1) ? 
		rcPixel8 : (mInternalImage->bps == 16 && mInternalImage->spp == 1) ? 
		rcPixel16 : (mInternalImage->bps == 8 && mInternalImage->spp == 3) ? 
		rcPixel32S : (mInternalImage->bps == 8 && mInternalImage->spp == 4) ? 
		rcPixel32S : rcPixelUnknown;
		
 	mStripperimage = (uint32) mInternalImage->Image->tif_dir.td_stripsperimage;
	mStripoffsets.resize (mStripperimage); 
	for (uint32 ii = 0; ii < mStripoffsets.size (); ii++) mStripoffsets[ii] = mInternalImage->Image->tif_dir.td_stripoffset[ii];
	mStripbytecounts.resize (mStripperimage); 	
	for (uint32 ii = 0; ii < mStripbytecounts.size (); ii++) mStripbytecounts[ii] = mInternalImage->Image->tif_dir.td_stripbytecount[ii];	
   
	mRootHeight = mStripperimage * height ();
	mRowperstrip = mInternalImage->Image->tif_dir.td_rowsperstrip;		
		
	mFileBO = TIFFIsBigEndian(mInternalImage->Image) == 0 ? eByteOrderLittleEndian : eByteOrderBigEndian;
	
	return;
}


void TIFFImageIO::SetNumberOfDimensions(unsigned int dim)
 {
   if(dim != m_NumberOfDimensions)
     {
	m_Dimensions.resize( dim );
	m_Origin.resize( dim );
	m_Spacing.resize( dim );
	m_Strides.resize( dim+2 );
	m_NumberOfDimensions = dim;
     }
 }


int32 TIFFImageIO::numberOfPages () const
{
   if(mInternalImage && isSTKfile ())
	return mInternalImage->NumberOfPages;
}


bool TIFFImageIO::isSTKfile () const
{
	return mInternalImage->mIsSTKfile;
}

TIFFImageIO::~TIFFImageIO()
{
	mInternalImage->Clean();
	delete mInternalImage;
}

bool TIFFImageIO::CanReadFile(const char* file) 
{ 
// First check the extension
	std::string filename = file;
	if(  filename == "" )
	{
//  rmWarningMacro(<<"No filename specified.");
		return false;
	}   

// Now check if this is a valid TIFF image
	TIFFErrorHandler save = TIFFSetErrorHandler(0);
	int res = mInternalImage->Open(file);
	if (res)
	{
		ReadImageInformation ();
		TIFFSetErrorHandler(save);
		return true;
	}
	mInternalImage->Clean();
	TIFFSetErrorHandler(save);
	return false;
}


// out is a rcWindow& 
//@note perhaps this the place to add tile support similar to the templated vtk stuff
rcWindow TIFFImageIO::ReadGenericImage()
{
	if ( mInternalImage->PlanarConfig != PLANARCONFIG_CONTIG  ||  rcDepth () == rcPixelUnknown)
//		GetFormat () == TIFFImageIO::PALETTE_RGB ||
	{
		rmExceptionMacro (<< "This reader can only do PLANARCONFIG_CONTIG");
	}

	bool success = true; // start believing


	rcWindow thisImage (width (), height (), rcDepth ());
	uint32 planeOffset = mImageCount * (mStripoffsets[mStripperimage-1]+mStripbytecounts[mStripperimage-1] - mStripoffsets [0]); 
	planeOffset += mStripoffsets[0];

	mInternalImage->Image->tif_dir.td_stripoffset[0] = planeOffset;
	for (uint32 i = 1; i < mStripperimage; i++) 
		mInternalImage->Image->tif_dir.td_stripoffset[i] = mInternalImage->Image->tif_dir.td_stripoffset[i-1] + 
		mInternalImage->Image->tif_dir.td_stripbytecount[i-1];

	if (rcDepth() == rcPixel32S && mInternalImage->spp == 3 && mInternalImage->bps == 8)
	{
		rcWindow tmpStore (width(), mRowperstrip, rcDepth () );
		for(uint y=0; success && (y< mStripperimage ); ++y)
		{
			success &= ( TIFFReadRawStrip(mInternalImage->Image, (tstrip_t) ( y ) , 
				tmpStore.rowPointer (0), -1) > 0);
			if( !success ) break;

			// for each row in the strip
			for (uint32 rows = 0; rows < mRowperstrip; rows++)
			{
					// Get the right row in the dest
					// unpack the row in 24bit source in to 32 bit dest
			}
		}
	}
	else
	{

		for(uint y=0; success && (y< mStripperimage ); ++y)
		{
			success &= ( TIFFReadRawStrip(mInternalImage->Image, (tstrip_t) ( y ) , 
				thisImage.rowPointer (y * mRowperstrip), -1) > 0);
			if( !success ) break;
		}

		if (rcDepth() == rcPixel16 && (rfPlatformByteOrder() != byteOrder ()))
		{
			for (uint32 rows = 0; rows < height (); rows++)
				TIFFSwabArrayOfShort ((uint16*) thisImage.rowPointer (rows), thisImage.width () );
		}	
	}
	if (success)
		return thisImage;
	else
		return rcWindow ();
}


int TIFFImageIO::EvaluateImageAt( void* out, void* in )
{
	unsigned char *image = (unsigned char*)out;
	unsigned char *source = (unsigned char*)in;

	int increment;
	unsigned short red, green, blue, alpha;
	switch ( GetFormat() )
	{
		case TIFFImageIO::GRAYSCALE:
		if ( mInternalImage->Photometrics == 
			PHOTOMETRIC_MINISBLACK )
		{
			if(m_ComponentType == USHORT)
			{
				unsigned short *image = (unsigned short*)out;
				unsigned short *source = (unsigned short*)in;
				*image = *source;
			}
			else
			{
				*image = *source;
			}
		}
		else
		{
			*image = ~( *source );
		}
		increment = 1;
		break;
		case TIFFImageIO::PALETTE_GRAYSCALE:
		GetColor(*source, &red, &green, &blue);
		*image = static_cast<unsigned char>(red >> 8);
		increment = 1;
		break;
		case TIFFImageIO::RGB_: 
		red   = *(source);
		green = *(source+1);
		blue  = *(source+2);
		*(image)   = red;
		*(image+1) = green;
		*(image+2) = blue;
		if ( mInternalImage->spp == 4 )
		{
			alpha = *(source+3);
			*(image+3) = 255-alpha;       
		}
		increment = mInternalImage->spp;
		break;
		case TIFFImageIO::PALETTE_RGB:
		GetColor(*source, &red, &green, &blue);     
		*(image)   = static_cast<unsigned char>(red >> 8);
		*(image+1) = static_cast<unsigned char>(green >> 8);
		*(image+2) = static_cast<unsigned char>(blue >> 8);
		increment = 3;
		break;
		default:
		return 0;
	}

	return increment;
}


void TIFFImageIO::GetColor( int index, unsigned short *red, 
	unsigned short *green, unsigned short *blue )
{
	*red   = 0;
	*green = 0;
	*blue  = 0;
	if ( index < 0 ) 
	{
		std::cout << "Color index has to be greater than 0" << std::endl;
		return;
	}
	if ( TotalColors > 0 && 
		ColorRed && ColorGreen && ColorBlue )
	{
		if ( index >= TotalColors )
		{
			std::cout << "Color index has to be less than number of colors ("
				<< TotalColors << ")" << std::endl;
			return;
		}
		*red   = *(ColorRed   + index);
		*green = *(ColorGreen + index);
		*blue  = *(ColorBlue  + index);
		return;
	}

	unsigned short photometric;

	if (!TIFFGetField(mInternalImage->Image, TIFFTAG_PHOTOMETRIC, &photometric)) 
	{
		if ( mInternalImage->Photometrics != PHOTOMETRIC_PALETTE )
		{
			std::cout << "You can only access colors for palette images" << std::endl;
			return;
		}
	}

	unsigned short *red_orig, *green_orig, *blue_orig;

	switch (mInternalImage->bps) 
	{
		case 1: case 2: case 4:
		case 8: case 16:
		break;
		default:
		std::cout <<  "Sorry, can not image with " 
			<< mInternalImage->bps
			<< "-bit samples" << std::endl;
		return;
	}
	if (!TIFFGetField(mInternalImage->Image, TIFFTAG_COLORMAP,
		&red_orig, &green_orig, &blue_orig)) 
	{
		std::cout << "Missing required \"Colormap\" tag" << std::endl;
		return;
	}
	TotalColors = (1L << mInternalImage->bps);

	if ( index >= TotalColors )
	{
		std::cout << "Color index has to be less than number of colors ("
			<< TotalColors << ")" << std::endl;
		return;
	}
	ColorRed   =   red_orig;
	ColorGreen = green_orig;
	ColorBlue  =  blue_orig;

	*red   = *(red_orig   + index);
	*green = *(green_orig + index);
	*blue  = *(blue_orig  + index);
}


unsigned int TIFFImageIO::GetFormat( )
{
	unsigned int cc;  

	if ( ImageFormat != TIFFImageIO::NOFORMAT )
	{
		return ImageFormat;
	}


	switch ( mInternalImage->Photometrics )
	{
		case PHOTOMETRIC_RGB: 
		case PHOTOMETRIC_YCBCR: 
		ImageFormat = TIFFImageIO::RGB_;
		return ImageFormat;
		case PHOTOMETRIC_MINISWHITE:
		case PHOTOMETRIC_MINISBLACK:
		ImageFormat = TIFFImageIO::GRAYSCALE;
		return ImageFormat;
		case PHOTOMETRIC_PALETTE:
		for( cc=0; cc<256; cc++ ) 
		{
			unsigned short red, green, blue;
			GetColor( cc, &red, &green, &blue );
			if ( red != green || red != blue )
			{
				ImageFormat = TIFFImageIO::PALETTE_RGB;
				return ImageFormat;
			}
		}
		ImageFormat = TIFFImageIO::PALETTE_GRAYSCALE;
		return ImageFormat;
	}
	ImageFormat = TIFFImageIO::OTHER;
	return ImageFormat;
}

#if 0
/** Read a tiled tiff */  
void TIFFImageIO::ReadTiles(void* buffer)
{
	unsigned char* volume = reinterpret_cast<unsigned char*>(buffer);

	for(unsigned int col = 0;col<mInternalImage->Width;col+=mInternalImage->TileWidth)
	{
		for(unsigned int row = 0;row<mInternalImage->Height;row+=mInternalImage->TileHeight)
		{
			unsigned char *tempImage;
			tempImage = new unsigned char[ mInternalImage->TileWidth * mInternalImage->TileHeight * mInternalImage->spp];

			if(TIFFReadTile(mInternalImage->Image,tempImage, col,row,0,0)<0)
			{
				std::cout << "Cannot read tile : "<< row << "," << col << " from file" << std::endl;
				if ( tempImage != buffer )
				{
					delete [] tempImage;
				}

				return;
			}

			unsigned int xx, yy;
			for ( yy = 0; yy < mInternalImage->TileHeight; yy++ )
			{
				for ( xx = 0; xx <  mInternalImage->TileWidth; xx++ )
				{
					for(unsigned int i=0;i< mInternalImage->spp;i++)
					{
						*volume = *(tempImage++);
						volume++;
					}
				}
			}
		}
	}
}
#endif

/** Read a multipage tiff */  
vector<rcWindow> TIFFImageIO::ReadPages()
{
	vector<rcWindow> pages (numberOfPages ());
	mImageCount = 0;
	for(unsigned int page = 0;page<pages.size ();page++)
	{
		//@note if we can not read it try the do all ReadRGBA
		//The problem is that this only supports 32 bit images 
		//@note change when time and z are properly incorported
		pages[page] = Read ();
		pages[page].frameBuf()->setTimestamp( rcTimestamp::from_seconds((double) page) );
		mImageCount++;
//		TIFFReadDirectory(mInternalImage->Image);
	}
	return pages;
}

/** Read a multipage tiff */  
rcWindow TIFFImageIO::ReadSinglePage ()
{
	rcWindow page; 
	mImageCount = 1;

	page = Read ();
	return page;
}


static void
SwabLongTriple (uint32* tp, unsigned long n)
{
	unsigned char* cp;
	unsigned char t;

/* XXX unroll loop some */
	while (n-- > 0) {
	cp = (unsigned char*) tp;
	t = cp[3]; cp[3] = cp[1]; cp[1] = t;
	tp += 1;
}
}

rcWindow TIFFImageIO::Read()
{

	if ( mInternalImage->Compression == COMPRESSION_OJPEG )
	{
		rmExceptionMacro (<< "This reader cannot read old JPEG compression");
	}

	//@note if we can not read it try the do all ReadRGBA
		//The problem is that this only supports 32 bit images 
	if ( !mInternalImage->CanRead() )
	{
		//@note using rcWindow since it is ref counted and will be deleted upon dtor
		rcWindow tempImage (width (), height (), rcPixel32S);
		if (TIFFReadRGBAImageOriented(mInternalImage->Image, 
			width(), height(), (uint32*) tempImage.rowPointer (0), ORIENTATION_TOPLEFT))
		{


		//@note rcWindow assues that the start of every row is (currently 16byte) aligned 
		//there for we copy from the contigious space to the all-rows-aligned space
			if( GetComponentType () == USHORT)
			{
				rmExceptionMacro (<< " multibyte components not supported ");
			}
			else if ( GetComponentType () == UCHAR) 
			{
			bool badindians = rfPlatformByteOrder() != byteOrder ();

			for (uint32 j = 0; j < height (); j++)
					{
					if (badindians)
							SwabLongTriple ((uint32 *) tempImage.rowPointer (j), tempImage.width () );
					}

//				rcFrameRef buf(new rcFrame ((char *) (tempImage.rowPointer (0)), tempImage.rowUpdate (), 
//					width (), height (), rcPixel32S, false));
//				return rcWindow (buf); 
				return tempImage;
			}
			else 
			{
				rmAssert (0);
			}
		}
		else
		{
			std::cout << "Problem reading RGB image" << std::endl;
			return rcWindow ();
		}
	}
	else
	{
		return ReadGenericImage();
	}
}



