/////////////////////////////////////////////////////////////////////////////
// Name:        wx/image.h
// Purpose:     wxImage class
// Author:      Robert Roebling
// RCS-ID:      $Id$
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_IMAGE_H_
#define _WX_IMAGE_H_

#include "wx/defs.h"

#if wxUSE_IMAGE

#include "wx/object.h"
#include "wx/string.h"
#include "wx/gdicmn.h"
#include "wx/hashmap.h"

#if wxUSE_STREAMS
#  include "wx/stream.h"
#endif

// on some systems (Unixware 7.x) index is defined as a macro in the headers
// which breaks the compilation below
#undef index

#define wxIMAGE_OPTION_QUALITY  wxString(_T("quality"))
#define wxIMAGE_OPTION_FILENAME wxString(_T("FileName"))

#define wxIMAGE_OPTION_RESOLUTION            wxString(_T("Resolution"))
#define wxIMAGE_OPTION_RESOLUTIONX           wxString(_T("ResolutionX"))
#define wxIMAGE_OPTION_RESOLUTIONY           wxString(_T("ResolutionY"))

#define wxIMAGE_OPTION_RESOLUTIONUNIT        wxString(_T("ResolutionUnit"))

#define wxIMAGE_OPTION_MAX_WIDTH             wxString(_T("MaxWidth"))
#define wxIMAGE_OPTION_MAX_HEIGHT            wxString(_T("MaxHeight"))

// constants used with wxIMAGE_OPTION_RESOLUTIONUNIT
//
// NB: don't change these values, they correspond to libjpeg constants
enum wxImageResolution
{
    // Resolution not specified
    wxIMAGE_RESOLUTION_NONE = 0,

    // Resolution specified in inches
    wxIMAGE_RESOLUTION_INCHES = 1,

    // Resolution specified in centimeters
    wxIMAGE_RESOLUTION_CM = 2
};

// Constants for wxImage::Scale() for determining the level of quality
enum
{
    wxIMAGE_QUALITY_NORMAL = 0,
    wxIMAGE_QUALITY_HIGH = 1
};

// alpha channel values: fully transparent, default threshold separating
// transparent pixels from opaque for a few functions dealing with alpha and
// fully opaque
const unsigned char wxIMAGE_ALPHA_TRANSPARENT = 0;
const unsigned char wxIMAGE_ALPHA_THRESHOLD = 0x80;
const unsigned char wxIMAGE_ALPHA_OPAQUE = 0xff;

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_FWD_CORE wxImageHandler;
class WXDLLIMPEXP_FWD_CORE wxImage;
class WXDLLIMPEXP_FWD_CORE wxPalette;

//-----------------------------------------------------------------------------
// wxVariant support
//-----------------------------------------------------------------------------

#if wxUSE_VARIANT
#include "wx/variant.h"
DECLARE_VARIANT_OBJECT_EXPORTED(wxImage,WXDLLIMPEXP_CORE)
#endif

//-----------------------------------------------------------------------------
// wxImageHandler
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxImageHandler: public wxObject
{
public:
    wxImageHandler()
        : m_name(wxEmptyString), m_extension(wxEmptyString), m_mime(), m_type(wxBITMAP_TYPE_INVALID)
        { }

#if wxUSE_STREAMS
    virtual bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 );
    virtual bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true );

    virtual int GetImageCount( wxInputStream& stream );

    bool CanRead( wxInputStream& stream ) { return CallDoCanRead(stream); }
    bool CanRead( const wxString& name );
#endif // wxUSE_STREAMS

    void SetName(const wxString& name) { m_name = name; }
    void SetExtension(const wxString& ext) { m_extension = ext; }
    void SetAlExtensions(const wxArrayString& exts) { m_altExtensions = exts; }
    void SetType(wxBitmapType type) { m_type = type; }
    void SetMimeType(const wxString& type) { m_mime = type; }
    const wxString& GetName() const { return m_name; }
    const wxString& GetExtension() const { return m_extension; }
    const wxArrayString& GetAltExtensions() const { return m_altExtensions; }
    wxBitmapType GetType() const { return m_type; }
    const wxString& GetMimeType() const { return m_mime; }

#if WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED(
        void SetType(long type) { SetType((wxBitmapType)type); }
    )
#endif // WXWIN_COMPATIBILITY_2_8

protected:
#if wxUSE_STREAMS
    virtual bool DoCanRead( wxInputStream& stream ) = 0;

    // save the stream position, call DoCanRead() and restore the position
    bool CallDoCanRead(wxInputStream& stream);
#endif // wxUSE_STREAMS

    // helper for the derived classes SaveFile() implementations: returns the
    // values of x- and y-resolution options specified as the image options if
    // any
    static wxImageResolution
    GetResolutionFromOptions(const wxImage& image, int *x, int *y);


    wxString  m_name;
    wxString  m_extension;
    wxArrayString m_altExtensions;
    wxString  m_mime;
    wxBitmapType m_type;

private:
    DECLARE_CLASS(wxImageHandler)
};

//-----------------------------------------------------------------------------
// wxImageHistogram
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxImageHistogramEntry
{
public:
    wxImageHistogramEntry() { index = value = 0; }
    unsigned long index;
    unsigned long value;
};

WX_DECLARE_EXPORTED_HASH_MAP(unsigned long, wxImageHistogramEntry,
                             wxIntegerHash, wxIntegerEqual,
                             wxImageHistogramBase);

class WXDLLIMPEXP_CORE wxImageHistogram : public wxImageHistogramBase
{
public:
    wxImageHistogram() : wxImageHistogramBase(256) { }

    // get the key in the histogram for the given RGB values
    static unsigned long MakeKey(unsigned char r,
                                 unsigned char g,
                                 unsigned char b)
    {
        return (r << 16) | (g << 8) | b;
    }

    // find first colour that is not used in the image and has higher
    // RGB values than RGB(startR, startG, startB)
    //
    // returns true and puts this colour in r, g, b (each of which may be NULL)
    // on success or returns false if there are no more free colours
    bool FindFirstUnusedColour(unsigned char *r,
                               unsigned char *g,
                               unsigned char *b,
                               unsigned char startR = 1,
                               unsigned char startG = 0,
                               unsigned char startB = 0 ) const;
};

//-----------------------------------------------------------------------------
// wxImage
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxImage: public wxObject
{
public:
    // red, green and blue are 8 bit unsigned integers in the range of 0..255
    // We use the identifier RGBValue instead of RGB, since RGB is #defined
    class RGBValue
    {
    public:
      RGBValue(unsigned char r=0, unsigned char g=0, unsigned char b=0)
        : red(r), green(g), blue(b) {}
        unsigned char red;
        unsigned char green;
        unsigned char blue;
    };

    // hue, saturation and value are doubles in the range 0.0..1.0
    class HSVValue
    {
    public:
        HSVValue(double h=0.0, double s=0.0, double v=0.0)
            : hue(h), saturation(s), value(v) {}
        double hue;
        double saturation;
        double value;
    };

    wxImage() {}
    wxImage( int width, int height, bool clear = true )
        { Create( width, height, clear ); }
    wxImage( int width, int height, unsigned char* data, bool static_data = false )
        { Create( width, height, data, static_data ); }
    wxImage( int width, int height, unsigned char* data, unsigned char* alpha, bool static_data = false )
        { Create( width, height, data, alpha, static_data ); }

    // ctor variants using wxSize:
    wxImage( const wxSize& sz, bool clear = true )
        { Create( sz, clear ); }
    wxImage( const wxSize& sz, unsigned char* data, bool static_data = false )
        { Create( sz, data, static_data ); }
    wxImage( const wxSize& sz, unsigned char* data, unsigned char* alpha, bool static_data = false )
        { Create( sz, data, alpha, static_data ); }

    wxImage( const wxString& name, wxBitmapType type = wxBITMAP_TYPE_ANY, int index = -1 )
        { LoadFile( name, type, index ); }
    wxImage( const wxString& name, const wxString& mimetype, int index = -1 )
        { LoadFile( name, mimetype, index ); }
    wxImage( const char* const* xpmData )
        { Create(xpmData); }

#if wxUSE_STREAMS
    wxImage( wxInputStream& stream, wxBitmapType type = wxBITMAP_TYPE_ANY, int index = -1 )
        { LoadFile( stream, type, index ); }
    wxImage( wxInputStream& stream, const wxString& mimetype, int index = -1 )
        { LoadFile( stream, mimetype, index ); }
#endif // wxUSE_STREAMS

    bool Create( const char* const* xpmData );
#ifdef __BORLANDC__
    // needed for Borland 5.5
    wxImage( char** xpmData ) { Create(const_cast<const char* const*>(xpmData)); }
    bool Create( char** xpmData ) { return Create(const_cast<const char* const*>(xpmData)); }
#endif

    bool Create( int width, int height, bool clear = true );
    bool Create( int width, int height, unsigned char* data, bool static_data = false );
    bool Create( int width, int height, unsigned char* data, unsigned char* alpha, bool static_data = false );
    
    // Create() variants using wxSize:
    bool Create( const wxSize& sz, bool clear = true )
        { return Create(sz.GetWidth(), sz.GetHeight(), clear); }
    bool Create( const wxSize& sz, unsigned char* data, bool static_data = false )
        { return Create(sz.GetWidth(), sz.GetHeight(), data, static_data); }
    bool Create( const wxSize& sz, unsigned char* data, unsigned char* alpha, bool static_data = false )
        { return Create(sz.GetWidth(), sz.GetHeight(), data, alpha, static_data); }

    void Destroy();
   
    // initialize the image data with zeroes
    void Clear(unsigned char value = 0);

    // creates an identical copy of the image (the = operator
    // just raises the ref count)
    wxImage Copy() const;

    // return the new image with size width*height
    wxImage GetSubImage( const wxRect& rect) const;

    // Paste the image or part of this image into an image of the given size at the pos
    //  any newly exposed areas will be filled with the rgb colour
    //  by default if r = g = b = -1 then fill with this image's mask colour or find and
    //  set a suitable mask colour
    wxImage Size( const wxSize& size, const wxPoint& pos,
                  int r = -1, int g = -1, int b = -1 ) const;

    // pastes image into this instance and takes care of
    // the mask colour and out of bounds problems
    void Paste( const wxImage &image, int x, int y );

    // return the new image with size width*height
    wxImage Scale( int width, int height, int quality = wxIMAGE_QUALITY_NORMAL ) const;

    // box averager and bicubic filters for up/down sampling
    wxImage ResampleBox(int width, int height) const;
    wxImage ResampleBicubic(int width, int height) const;

    // blur the image according to the specified pixel radius
    wxImage Blur(int radius) const;
    wxImage BlurHorizontal(int radius) const;
    wxImage BlurVertical(int radius) const;

    wxImage ShrinkBy( int xFactor , int yFactor ) const ;

    // rescales the image in place
    wxImage& Rescale( int width, int height, int quality = wxIMAGE_QUALITY_NORMAL ) { return *this = Scale(width, height, quality); }

    // resizes the image in place
    wxImage& Resize( const wxSize& size, const wxPoint& pos,
                     int r = -1, int g = -1, int b = -1 ) { return *this = Size(size, pos, r, g, b); }

    // Rotates the image about the given point, 'angle' radians.
    // Returns the rotated image, leaving this image intact.
    wxImage Rotate(double angle, const wxPoint & centre_of_rotation,
                   bool interpolating = true, wxPoint * offset_after_rotation = NULL) const;

    wxImage Rotate90( bool clockwise = true ) const;
    wxImage Mirror( bool horizontally = true ) const;

    // replace one colour with another
    void Replace( unsigned char r1, unsigned char g1, unsigned char b1,
                  unsigned char r2, unsigned char g2, unsigned char b2 );

    // Convert to greyscale image. Uses the luminance component (Y) of the image.
    // The luma value (YUV) is calculated using (R * lr) + (G * lg) + (B * lb), defaults to ITU-T BT.601
    wxImage ConvertToGreyscale( double lr = 0.299, double lg = 0.587, double lb = 0.114 ) const;

    // convert to monochrome image (<r,g,b> will be replaced by white,
    // everything else by black)
    wxImage ConvertToMono( unsigned char r, unsigned char g, unsigned char b ) const;

    // these routines are slow but safe
    void SetRGB( int x, int y, unsigned char r, unsigned char g, unsigned char b );
    void SetRGB( const wxRect& rect, unsigned char r, unsigned char g, unsigned char b );
    unsigned char GetRed( int x, int y ) const;
    unsigned char GetGreen( int x, int y ) const;
    unsigned char GetBlue( int x, int y ) const;

    void SetAlpha(int x, int y, unsigned char alpha);
    unsigned char GetAlpha(int x, int y) const;

    // find first colour that is not used in the image and has higher
    // RGB values than <startR,startG,startB>
    bool FindFirstUnusedColour( unsigned char *r, unsigned char *g, unsigned char *b,
                                unsigned char startR = 1, unsigned char startG = 0,
                                unsigned char startB = 0 ) const;
    // Set image's mask to the area of 'mask' that has <r,g,b> colour
    bool SetMaskFromImage(const wxImage & mask,
                          unsigned char mr, unsigned char mg, unsigned char mb);

    // converts image's alpha channel to mask (choosing mask colour
    // automatically or using the specified colour for the mask), if it has
    // any, does nothing otherwise:
    bool ConvertAlphaToMask(unsigned char threshold = wxIMAGE_ALPHA_THRESHOLD);
    void ConvertAlphaToMask(unsigned char mr, unsigned char mg, unsigned char mb,
                            unsigned char threshold = wxIMAGE_ALPHA_THRESHOLD);


    // This method converts an image where the original alpha
    // information is only available as a shades of a colour
    // (actually shades of grey) typically when you draw anti-
    // aliased text into a bitmap. The DC drawinf routines
    // draw grey values on the black background although they
    // actually mean to draw white with differnt alpha values.
    // This method reverses it, assuming a black (!) background
    // and white text (actually only the red channel is read).
    // The method will then fill up the whole image with the
    // colour given.
    bool ConvertColourToAlpha( unsigned char r, unsigned char g, unsigned char b );

    static bool CanRead( const wxString& name );
    static int GetImageCount( const wxString& name, wxBitmapType type = wxBITMAP_TYPE_ANY );
    virtual bool LoadFile( const wxString& name, wxBitmapType type = wxBITMAP_TYPE_ANY, int index = -1 );
    virtual bool LoadFile( const wxString& name, const wxString& mimetype, int index = -1 );

#if wxUSE_STREAMS
    static bool CanRead( wxInputStream& stream );
    static int GetImageCount( wxInputStream& stream, wxBitmapType type = wxBITMAP_TYPE_ANY );
    virtual bool LoadFile( wxInputStream& stream, wxBitmapType type = wxBITMAP_TYPE_ANY, int index = -1 );
    virtual bool LoadFile( wxInputStream& stream, const wxString& mimetype, int index = -1 );
#endif

    virtual bool SaveFile( const wxString& name ) const;
    virtual bool SaveFile( const wxString& name, wxBitmapType type ) const;
    virtual bool SaveFile( const wxString& name, const wxString& mimetype ) const;

#if wxUSE_STREAMS
    virtual bool SaveFile( wxOutputStream& stream, wxBitmapType type ) const;
    virtual bool SaveFile( wxOutputStream& stream, const wxString& mimetype ) const;
#endif

    bool Ok() const { return IsOk(); }
    bool IsOk() const;
    int GetWidth() const;
    int GetHeight() const;

    wxSize GetSize() const
        { return wxSize(GetWidth(), GetHeight()); }

    // Gets the type of image found by LoadFile or specified with SaveFile
    wxBitmapType GetType() const;

    // Set the image type, this is normally only called if the image is being
    // created from data in the given format but not using LoadFile() (e.g.
    // wxGIFDecoder uses this)
    void SetType(wxBitmapType type);

    // these functions provide fastest access to wxImage data but should be
    // used carefully as no checks are done
    unsigned char *GetData() const;
    void SetData( unsigned char *data, bool static_data=false );
    void SetData( unsigned char *data, int new_width, int new_height, bool static_data=false );

    unsigned char *GetAlpha() const;    // may return NULL!
    bool HasAlpha() const { return GetAlpha() != NULL; }
    void SetAlpha(unsigned char *alpha = NULL, bool static_data=false);
    void InitAlpha();

    // return true if this pixel is masked or has alpha less than specified
    // threshold
    bool IsTransparent(int x, int y,
                       unsigned char threshold = wxIMAGE_ALPHA_THRESHOLD) const;

    // Mask functions
    void SetMaskColour( unsigned char r, unsigned char g, unsigned char b );
    // Get the current mask colour or find a suitable colour
    // returns true if using current mask colour
    bool GetOrFindMaskColour( unsigned char *r, unsigned char *g, unsigned char *b ) const;
    unsigned char GetMaskRed() const;
    unsigned char GetMaskGreen() const;
    unsigned char GetMaskBlue() const;
    void SetMask( bool mask = true );
    bool HasMask() const;

#if wxUSE_PALETTE
    // Palette functions
    bool HasPalette() const;
    const wxPalette& GetPalette() const;
    void SetPalette(const wxPalette& palette);
#endif // wxUSE_PALETTE

    // Option functions (arbitrary name/value mapping)
    void SetOption(const wxString& name, const wxString& value);
    void SetOption(const wxString& name, int value);
    wxString GetOption(const wxString& name) const;
    int GetOptionInt(const wxString& name) const;
    bool HasOption(const wxString& name) const;

    unsigned long CountColours( unsigned long stopafter = (unsigned long) -1 ) const;

    // Computes the histogram of the image and fills a hash table, indexed
    // with integer keys built as 0xRRGGBB, containing wxImageHistogramEntry
    // objects. Each of them contains an 'index' (useful to build a palette
    // with the image colours) and a 'value', which is the number of pixels
    // in the image with that colour.
    // Returned value: # of entries in the histogram
    unsigned long ComputeHistogram( wxImageHistogram &h ) const;

    // Rotates the hue of each pixel of the image. angle is a double in the range
    // -1.0..1.0 where -1.0 is -360 degrees and 1.0 is 360 degrees
    void RotateHue(double angle);

    static wxList& GetHandlers() { return sm_handlers; }
    static void AddHandler( wxImageHandler *handler );
    static void InsertHandler( wxImageHandler *handler );
    static bool RemoveHandler( const wxString& name );
    static wxImageHandler *FindHandler( const wxString& name );
    static wxImageHandler *FindHandler( const wxString& extension, wxBitmapType imageType );
    static wxImageHandler *FindHandler( wxBitmapType imageType );

    static wxImageHandler *FindHandlerMime( const wxString& mimetype );

    static wxString GetImageExtWildcard();

    static void CleanUpHandlers();
    static void InitStandardHandlers();

    static HSVValue RGBtoHSV(const RGBValue& rgb);
    static RGBValue HSVtoRGB(const HSVValue& hsv);

#if WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED_CONSTRUCTOR(
        wxImage(const wxString& name, long type, int index = -1)
        {
            LoadFile(name, (wxBitmapType)type, index);
        }
    )

#if wxUSE_STREAMS
    wxDEPRECATED_CONSTRUCTOR(
        wxImage(wxInputStream& stream, long type, int index = -1)
        {
            LoadFile(stream, (wxBitmapType)type, index);
        }
    )

    wxDEPRECATED(
        bool LoadFile(wxInputStream& stream, long type, int index = -1)
        {
            return LoadFile(stream, (wxBitmapType)type, index);
        }
    )

    wxDEPRECATED(
        bool SaveFile(wxOutputStream& stream, long type) const
        {
            return SaveFile(stream, (wxBitmapType)type);
        }
    )
#endif // wxUSE_STREAMS

    wxDEPRECATED(
        bool LoadFile(const wxString& name, long type, int index = -1)
        {
            return LoadFile(name, (wxBitmapType)type, index);
        }
    )

    wxDEPRECATED(
        bool SaveFile(const wxString& name, long type) const
        {
            return SaveFile(name, (wxBitmapType)type);
        }
    )

    wxDEPRECATED(
        static wxImageHandler *FindHandler(const wxString& ext, long type)
        {
            return FindHandler(ext, (wxBitmapType)type);
        }
    )

    wxDEPRECATED(
        static wxImageHandler *FindHandler(long imageType)
        {
            return FindHandler((wxBitmapType)imageType);
        }
    )
#endif // WXWIN_COMPATIBILITY_2_8

protected:
    static wxList   sm_handlers;

    // return the index of the point with the given coordinates or -1 if the
    // image is invalid of the coordinates are out of range
    //
    // note that index must be multiplied by 3 when using it with RGB array
    long XYToIndex(int x, int y) const;

    virtual wxObjectRefData* CreateRefData() const;
    virtual wxObjectRefData* CloneRefData(const wxObjectRefData* data) const;

private:
    friend class WXDLLIMPEXP_FWD_CORE wxImageHandler;

#if wxUSE_STREAMS
    // read the image from the specified stream updating image type if
    // successful
    bool DoLoad(wxImageHandler& handler, wxInputStream& stream, int index);

    // write the image to the specified stream and also update the image type
    // if successful
    bool DoSave(wxImageHandler& handler, wxOutputStream& stream) const;
#endif // wxUSE_STREAMS

    DECLARE_DYNAMIC_CLASS(wxImage)
};


extern void WXDLLIMPEXP_CORE wxInitAllImageHandlers();

extern WXDLLIMPEXP_DATA_CORE(wxImage)    wxNullImage;

//-----------------------------------------------------------------------------
// wxImage handlers
//-----------------------------------------------------------------------------

#include "wx/imagbmp.h"
#include "wx/imagpng.h"
#include "wx/imaggif.h"
#include "wx/imagpcx.h"
#include "wx/imagjpeg.h"
#include "wx/imagtga.h"
#include "wx/imagtiff.h"
#include "wx/imagpnm.h"
#include "wx/imagxpm.h"
#include "wx/imagiff.h"

#endif // wxUSE_IMAGE

#endif
  // _WX_IMAGE_H_
