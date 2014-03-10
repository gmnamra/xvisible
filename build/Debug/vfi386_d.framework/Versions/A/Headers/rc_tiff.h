/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.4  2005/12/07 00:04:17  arman
 *more tiff
 *
 *Revision 1.3  2005/12/06 23:48:21  arman
 *added tag enumes
 *
 *Revision 1.2  2005/10/27 22:58:19  arman
 *z additions and cleanup
 *
 *Revision 1.1  2005/10/25 22:36:55  arman
 *c++ interface
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcTIFF_H
#define __rcTIFF_H

#include <rc_types.h>
#include <sys/stat.h>
#include <iostream>
#include <ostream>
#include <rc_macro.h>
#include <vector>
#include <rc_window.h>

extern "C" {
#include <tiffio.h>
#include <tiffiop.h>
}

enum rcMetaMorphTiffTags
  {
    uiidesc = 270,
    uic1tag = 33628,
    uic2tag = 33629,
    uic3tag = 33630,
    uic4tag = 33631
  };

    
class ImageIORegion
{
public:
 
  /** Index typedef support. An index is used to access pixel values. */
  typedef std::vector<long>  IndexType;

  /** Size typedef support. A size is used to define region bounds. */
  typedef std::vector<long>  SizeType;
  
  /** Constructor. ImageIORegion is a lightweight object that is not reference
   * counted, so the constructor is public. */
  ImageIORegion(unsigned int dimension)
  {
    m_ImageDimension = dimension;
    m_Index.resize(m_ImageDimension);
    m_Size.resize(m_ImageDimension);
    std::fill(m_Index.begin(), m_Index.end(), 0);
    std::fill(m_Size.begin(), m_Size.end(), 0);
  }
  
  /** Constructor. ImageIORegion is a lightweight object that is not reference
   * counted, so the constructor is public.  Default dimension is 2. */
  ImageIORegion()
  {
    m_ImageDimension = 2;
    m_Index.resize(2);
    m_Size.resize(2);
    std::fill(m_Index.begin(), m_Index.end(), 0);
    std::fill(m_Size.begin(), m_Size.end(), 0);
  }
  
  /** Copy constructor. ImageIORegion is a lightweight object that is not
   * reference counted, so the copy constructor is public. */
  ImageIORegion(const  ImageIORegion & region)
  { 
    m_Index =region.m_Index; 
    m_Size = region.m_Size; 
    m_ImageDimension = region.m_ImageDimension;
  }
  
  /** operator=. ImageIORegion is a lightweight object that is not reference
   * counted, so operator= is public. */
  void operator=(const  ImageIORegion & region)
  { 
    m_Index = region.m_Index;  
    m_Size = region.m_Size;
    m_ImageDimension = region.m_ImageDimension;
  };

  /** Set the index defining the corner of the region. */
  void SetIndex(const IndexType &index) 
  { m_Index = index; };

  /** Get index defining the corner of the region. */
  const IndexType& GetIndex() const
  { return m_Index; };
  
  /** Set the size of the region. This plus the index determines the
   * rectangular shape, or extent, of the region. */
  void SetSize(const SizeType &size)
  { m_Size = size; };

  /** Get the size of the region. */
  const SizeType& GetSize() const
  { return m_Size;}

  /** Convenience methods to get the size of the image in a particular
   * coordinate direction i. Do not try to access image sizes beyond the
   * the ImageDimension. */
  long GetSize(unsigned long i) const
  { return m_Size[i]; }
  long GetIndex(unsigned long i) const
  { return m_Index[i]; }
  void SetSize(const unsigned long i, long size)
  {m_Size[i] = size;}
  void SetIndex(const unsigned long i, long idx)
  {m_Index[i] = idx;}

  /** Compare two regions. */
  bool
  operator==(const  ImageIORegion & region) const
  {
    bool same = 1;
    same = (m_Index == region.m_Index);
    same = same && (m_Size == region.m_Size);
    same = same && (m_ImageDimension == region.m_ImageDimension);
    return same;
  }

  /** Compare two regions. */
  bool
  operator!=(const  ImageIORegion & region) const 
  {
    bool same = 1;
    same = (m_Index == region.m_Index);
    same = same && (m_Size == region.m_Size);
    same = same && (m_ImageDimension == region.m_ImageDimension);
    return !same;
  }
  
  /** Test if an index is inside */
  bool
  IsInside(const IndexType &index) const
  {
    for(unsigned int i=0; i<m_ImageDimension; i++)
      {
      if( index[i] < m_Index[i] ) 
        {
        return false;
        }
      if( index[i] >= m_Index[i] + m_Size[i] ) 
        {
        return false;
        }
      }
    return true;
  }

  /** Dimension of the image available at run time. */
  unsigned int GetImageDimension() const
  { return m_ImageDimension; }
  /** Dimension of the region to be written. This differs from the
   * the image dimension and is calculated at run-time by examining
   * the size of the image in each coordinate direction. 
   */
  unsigned int GetRegionDimension() const
  { 
    unsigned long dim=0;
    for (unsigned long i=0; i<m_ImageDimension; i++)
      {
      if ( m_Size[i] > 1 ) dim++;
      }
    return dim;
  }

  /** Get the number of pixels contained in this region. This just
   * multiplies the size components. */
  unsigned long GetNumberOfPixels() const;


private:
  unsigned int m_ImageDimension;
  std::vector<long> m_Index;
  std::vector<long> m_Size;
};


// Declare operator<<
std::ostream & operator<<(std::ostream &os, const ImageIORegion &region); 


class TIFFReaderInternal
{
public:
  TIFFReaderInternal();
  int Initialize();
  void Clean();
  int CanRead();
  int Open( const char *filename );
  TIFF *Image;
  unsigned int Width;
  unsigned int Height;
  unsigned short NumberOfPages;
  unsigned short CurrentPage;
  unsigned short spp;
  unsigned short SampleFormat;
  unsigned short Compression;
  unsigned short bps;
  unsigned short Photometrics;
  unsigned short PlanarConfig;
  unsigned short Orientation;
  unsigned long int TileDepth;
  unsigned int TileRows;
  unsigned int TileColumns;
  unsigned int TileWidth;
  unsigned int TileHeight;
  unsigned short NumberOfTiles;

  float XResolution;
  bool mIsSTKfile;
  
  // friend ostream& operator<< (ostream&, const TIFFReaderInternal&);
};

class TIFFImageIO 
{

public:

  TIFFImageIO();

  ~TIFFImageIO();

  TIFFImageIO(const  TIFFImageIO &);
 void operator=(const  TIFFImageIO &); 

  typedef  enum {UNKNOWNCOMPONENTTYPE,UCHAR,CHAR,USHORT,
		 SHORT,UINT,INT, ULONG,LONG, FLOAT,DOUBLE}
    IOComponentType;
  typedef  enum {UNKNOWNPIXELTYPE,SCALAR,RGB,RGBA,OFFSET,VECTOR,

                 POINT,COVARIANTVECTOR}
    IOPixelType;

  /** Set/Get the name of the file to be read. */

  rmSetStringMacro(FileName);

  rmGetStringMacro(FileName);

  bool CanReadFile(const char*);
  void ReadImageInformation();
  bool isSTKfile () const;
  int32 width () const;
  int32 height () const;
  rcPixel rcDepth () const;

  rcWindow Read ();
  vector<rcWindow> ReadPages ();
	rcWindow ReadSinglePage ();
	
//  void ReadTiles(void* buffer);
  int32 numberOfPages () const;
  reByteOrder byteOrder () const; 


  enum { NOFORMAT, RGB_, GRAYSCALE, PALETTE_RGB, PALETTE_GRAYSCALE, OTHER };
  enum { NoCompression, PackBits, JPEG, Deflate, LZW};

  void SetNumberOfDimensions(unsigned int dim);

  void SetNumberOfComponents(unsigned int c);
  unsigned int GetNumberOfComponents() const;
  void SetPixelType (IOPixelType pt);
  IOComponentType GetComponentType ();
  void SetIORegion (const ImageIORegion _arg);
  const ImageIORegion& GetIORegion ();

protected:
  void WriteSlice(std::string& fileName, const void* buffer);
  void WriteVolume(std::string& fileName, const void* buffer);
  void InitializeColors();
  rcWindow ReadGenericImage();
  int EvaluateImageAt( void* out, void* in );
  unsigned int  GetFormat();
  void GetColor( int index, unsigned short *red, unsigned short *green, unsigned short *blue );
private:
 
  bool m_Initialized;
  bool mIsTiled;
  std::string mFileName;
  unsigned int m_NumberOfComponents;
  unsigned int m_NumberOfDimensions;
  unsigned short *ColorRed;
  unsigned short *ColorGreen;
  unsigned short *ColorBlue;
  int TotalColors;
  unsigned int ImageFormat;
  TIFFReaderInternal * mInternalImage;
  int m_Compression;
  int mImageCount; 

  ImageIORegion m_IORegion;
  std::vector<double> m_Origin;
  std::vector<double> m_Spacing;
  std::vector<unsigned int> m_Strides;

  vector<uint32> mStripoffsets;
  vector<uint32> mStripbytecounts;
  uint32 mStripperimage;
  int32 mRootHeight;
  int32 mRowperstrip;

  /** The array which stores the number of pixels in the x, y, z directions. */
  std::vector<unsigned int> m_Dimensions;
  IOComponentType m_ComponentType;
  IOPixelType m_PixelType;
  bool m_UseCompression;
  rcPixel mRCdepth;
  reByteOrder mFileBO;

};



#endif /* __rcTIFF_H */
