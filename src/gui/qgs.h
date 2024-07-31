/***************************************************************************
 *   Copyright (C) 2005 by Piotr Szymanski <niedakh@gmail.com>             *
 *                                                                         *
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,                  *
 * MA  02110-1301  USA                                                     *
 ***************************************************************************/

#ifndef _GSINTERPETERLIB_H_
#define _GSINTERPETERLIB_H_

#include <QtGui>
#include <QtDebug>

#define GSDLLEXPORT
#ifdef Q_OS_WIN32
	#define _Windows
#else
	#define GSDLLAPI
	#define GSDLLCALL
#endif

// do not use these gsapi internal headers - 
// use latest ones from ghostpdl distribution /psi directory which must be in include path
// can override but but internal headers get out of date with latest gs version
#ifdef USE_INTERNAL_GSAPI
#include "gsinc/iapi.h"
#include "gsinc/gdevdsp.h"
#include "gsinc/ierrors.h"
#else
#include <ierrors.h>
#include <iapi.h>
#include <gdevdsp.h>

#endif


#include <iostream>

#define QGS_DISPLAY_32 0
#define QGS_DISPLAY_24 1

using namespace std;

/** @namespace GSApiWrapper Namespace containing the callback functions for communication with
    the ghostscript library.
*/
namespace GSApiWrapper
{
	int open(void* /*handle*/, void* /*device */ );
	int preclose(void* /*handle*/, void * /* device */ );
	int close(void* /*handle*/, void * /* device */ );
	int presize(void* /*handle*/, void * /* device*/, int /* width*/, int /* height*/, int /* raster*/, unsigned int /* format */);
	int sync(void* /*handle*/, void * /* device */ );
	int separation(void* /*handle*/, void * /* device*/, int /* comp_num*/,
	               const char * /* name*/, unsigned short /* c*/, unsigned short /* m*/,
	               unsigned short /* y*/, unsigned short /* k*/);
	int GSDLLCALL handleStdin  (void* /*handle*/, char* buffer, int len );
	int GSDLLCALL handleStdout(void* /*handle*/, const char* buffer, int len );
	int GSDLLCALL handleStderr  (void* /*handle*/, const char* buffer, int len );
	int update(void* /*handle*/, void * /* device*/, int /* x*/, int /* y*/, int /* w*/, int /* h*/);
	int size(void* /*handle*/, void * /* device*/, int width, int height, int raster, unsigned int format, unsigned char *pimage);
	int page(void* /*handle*/, void * /* device*/, int /* copies*/, int /* flush*/);
};

class GSError {
public:
	QString m_name; //! the description of an error
	int m_code;     //! the error code
	bool m_haserr;
public:
	GSError();
	~GSError();
	void setError(int code, QString msg);
	inline const QString& getName() const { return m_name; }
	inline int getCode() const { return m_code; }
	inline bool hasError() const { return m_haserr; }
	inline void clear() { m_haserr = false; }
};

// for some reason this is not defined in iapi.h
typedef int (GSDLLAPIPTR PFN_gsapi_register_callout)(
    void *instance, gs_callout callout, void *state);

class GSLibFunctions {
protected:
	QString m_LibGSLocation;
#ifdef Q_OS_WIN32
	HINSTANCE hmodule;
#else
	void* hmodule;
#endif
public:
	PFN_gsapi_revision gsapi_revision;
	PFN_gsapi_new_instance gsapi_new_instance;
	PFN_gsapi_delete_instance gsapi_delete_instance;
	PFN_gsapi_set_stdio gsapi_set_stdio;
	PFN_gsapi_set_poll gsapi_set_poll;
	// used in g2 9.2 or earlier - deprecated now
	PFN_gsapi_set_display_callback gsapi_set_display_callback;
	PFN_gsapi_register_callout gsapi_register_callout;
	PFN_gsapi_init_with_args gsapi_init_with_args;
	PFN_gsapi_run_string gsapi_run_string;
	PFN_gsapi_run_string_with_length gsapi_run_string_with_length;
	PFN_gsapi_run_string_begin gsapi_run_string_begin;
	PFN_gsapi_run_string_continue gsapi_run_string_continue;
	PFN_gsapi_run_string_end gsapi_run_string_end;
	PFN_gsapi_exit gsapi_exit;
public:
	GSLibFunctions();
	~GSLibFunctions();
	static GSLibFunctions* getInstance();
	int loadLibrary(const QString& location, QString& last_error);
	void freeLibrary();
	void resetFunctions();
	bool isLoaded();
	void tryLocation(const char* str);
   void tryLocationLoop(const char* prefix);
	QString getVersion();
	int getVersionMajor();
	static void StripWhiteSpace(QString& str);
	inline const QString& libGSLocation() { return m_LibGSLocation; }
};

/** \class GSInterpreterLib
    @brief The Qt Ghostscript library wrapper
           The idea of this library is to provide a comfortable way of using
           the Ghostscript library without the need of going through its C API.

           Please check the examples to see how to use the Ghostscript library to render graphics
           and how to use it like you would use Ghostscript for converting between formats.
*/

class GSInterpreterLib : public QObject
{
	Q_OBJECT
protected:
	GSLibFunctions* m_gs;
	double m_OriginX;
	double m_OriginY;
public:
	//! @enum MessageType information about the type of stdio communication in the io signal
	enum MessageType{
		Input  /*!< stdin information, ghostscript wants to read sth from stdin.
		            Should not happen in this class, but if you subclass it, make sure to handle possible
		            stdin requests. Ghostscript library reads from stdin rarely. */,
		Output /*!< stdout message, Ghostscript library wrote to stdout */ ,
		Error  /*!< stderr message, Ghostscript library wrote to stderr*/};

	//! @typedef a QPair which first element is the beginning of a block in the file while the second is the end
	typedef QPair<unsigned long , unsigned long > Position;

	//! @exception GSerror this exception gets thrown when Ghostscript library reports a critical error

	//! Creates an instance of Ghostscript library
	GSInterpreterLib();
	//! Destroys the instance of Ghostscript library, nothing to cleanup afterwards
	~GSInterpreterLib();
	/** @brief Starts the Ghostscript library interpreter
	           Starts the Ghostscript instance, sets the arguments, which default
	           consist of papersize, antialiasing options, orientation, resolution,
	           and platform fonts usage. The default options are not set directly by
	           the developer, but are prepared from the data the developer set using
	           relevant methods.

	           In case the user used setGhostscriptArguments, those arguments are used
	           instead of the default ones, useful when you need to use Ghostscript for
	           noninteractive (no pixmaps generated) work, like converting from ps to pdf.

	           The default values should be used when you need to normally display a file
	           and you need images of the pages.

	           Also this functions takes care of catching stdio communication if selected.
	           As a result the stdio communication with Ghostscript takes place via the IO
	           signal. Please note that stdio is different then communication with Ghostscript
	           interpreter. In short, you will never need stdin and you can read stdout and
	           stderror using the IO signal.
	    @param setStdio enable catching the stdio communication
	*/
	bool start(bool setStdio=true);
	/** @brief Stops the Ghostscript instance if running (but does not delete it) */
	bool stop();
	//! @return Returns true if it library is already working on a page request
	//! @return Returns true if it library is ready to receive a new page request using the run function
	//! @return Returns true if Ghostscript interpreter is running
	bool running () { return m_running; } ;

	/** @brief Sends file data to Ghostscript library
	           This functions takes care of putting the file's contents in the Ghostscript library.
	           Parsing a file
	    @param file a pointer to the FILE structure used by fread to send information to Ghostscript
	    @param pos the beginning and ending of the data block in the file (usually the position of the page in file)
	    @param sync whether to expect an image generated from the content specified by pos
	*/
	bool run(QFile& file);
	bool run(const string* data);

	bool startRender();
	bool nextRender(const char* str);
	bool endRender();

	bool hasError();

	const GSError& getError();

	/** @brief Enables progressive updating of the image
	           Enabling progressive updating means, that before Finished() is emitted, Updated() signals
	           are emitted with the area that was updated. If disabled (by default disabled),
	    @param progr enable/disable progressive updating
	*/
	void setGhostscriptArguments( const QStringList &list );

	/** @brief Enable display setting
	           This should be set to true if you are using the interpreter to render pages or in any way
	           need a display device in your use of the Ghostscript library. If you do not need it
	           (e.g. you are just converting ps to pdf) set it to false.
	    @param display enable/disable the display device
	*/
	void setDisplay( bool display = true);

	void setAlpha( bool alpha );

	/** @brief Sets orientation of the document
	    @param orientation the orientation of the page - 0 for 0 degrees, 1 for 90 etc.
	*/
	void setOrientation( int orientation );

	/** @brief Sets size of the page in pixels
	    @param w width
	    @param h height
	*/
	void setSize( int w, int h );

	/** @brief Sets the resolution of the image
	 */
	void setDPI(double dpi);

	/** @brief Enables buffering of stdout/stderr
	           If buffered, all the output will be stored in a QString buffer and will be accessible by
	           IObuffer() function. The io() signal will be emitted nevertheless.
	    @param buffered Buffering enabled if true
	*/
	void setBuffered( bool buffered );

	/** @brief Sets the magnification of the page (
	    @param magnify a magnification value (floating point, 0.35 means 35% of the real size)
	*/
	void setMagnify( double magnify );

	/** @brief Sets the paper size/format of the document
	    @param media string containing a valid Ghostscript papersize
	*/
	void setMedia (const QString &media) ;

	/** @brief Sets the number of bits used for antialiasing
	           The number of bits for antialiasing, 1 disables.
	    @param text number of text antialiasing bits (4 is recommended for good quality and still fast antialiasing)
	    @param graphics number of graphics antialiasing bits (2 is recommended for good quality and still fast antialiasing)
	*/
	void setAABits(int text=1, int graphics=1);

	/** @brief Enables or disables platform fonts
	    @param pfonts decides whether Ghostscript library should use the fonts available in the system (true) or only the ones embedded in the document (false), by default true
	*/
	void setPlatformFonts(bool pfonts=true);

	const QImage& getImage();

	void setOrigin(double ox, double oy);

	//! @brief Returns the IO buffer
	const QString& IOBuffer() { return m_buffer;};

	QString getOutpuStreamText();

private:
	bool sendOriginTranslate();

protected:
	/**
	@brief Describing how stuff works
	       Ghostscript will do callbacks after it is started and as also as it receives content to render.
	       First open will be called upon opening the device.
	       Second there will be a presize with information about the size, here we allocate the QImage;
	       Third there will be an update called with the whole page, we must ignore it, as it was called
	       before size was.
	       Fourth the size will be called with the pointer to our image.
	       Fifth a series of update will be called with coordinates to update, not width is the width
	       and not x+width, you need to add the x manually.
	       Sixth there will a page called which tells you that rendering is done.
	       Seventh a sync after the page tells you everything is finished.
	       If closing, preclose will appear before close.
	*/
	// handling communication with libgs
	int presize(void * /* device*/, int /* width*/, int /* height*/,
	            int /* raster*/, unsigned int /* format */);
	int gs_output ( const char* buffer, int len );
	int gs_error  ( const char* buffer, int len );
	int size(void * /* device*/, int width, int height, int raster, unsigned int format, unsigned char *pimage);
	int page(void * /* device*/, int /* copies*/, int /* flush*/);
	//! Checks if code is an error code
	//! @return returns true if no error, if non-critical error returns false, if critical error, throws GSerror
	//! @throw GSerror throws GSerror if code is critical error
	bool handleExit(int code);
	// state bools
	bool m_running;  //! True if an instance is running
	bool m_ready;    //! True if not processing any data request in run()
	int  m_cnvmode;
	bool m_alpha;
	unsigned char* m_conv;

	// Parameter holders
	// Orientation of the page (a multiplicant of 90 degrees)
	// additional info
	int m_orientation; //! Holds the orientation of the page @sa setOrientation()
	double m_magnify;  //! Magnification @sa setMagnify()
	// Size of the image we want to get
	int m_width;  //! Expected width of the image
	int m_height; //! Expected height of the image
	double m_dpi;    //! Resolution
	QString m_media; //! Papersize
	int m_Gwidth;    //! Width of the image buffer we received from Ghostscript library
	int m_Gheight;   //! Height of the image buffer we received from Ghostscript library
	unsigned char * m_imageChar; //! Image buffer we received from Ghostscript library
	unsigned int m_format; //! Format of the image stored in the buffer, if not changed in a subclass, it is 32bit RGBx
	int m_raster;  //! Size of a horizontal row (y-line) in the image buffer
	int m_textaa;  //! Number of bits used for text antialiasing @sa setAABits()
	int m_graphaa; //! Number of bits used for graphics antialiasing @sa setAABits()
	bool m_pfonts; //! True if platform fonts enabled @sa setPlatformFonts()
	// toggle display setting
	bool m_display;  //! True if display usage enabled @sa setDisplay()
	bool m_buffered; //! True if buffering stdout/stderr @sa setBuffered()
	/** The following tell us if we are in a position to update in progressive rendering, from GS:
	 * The caller should not access the image buffer:
	 *  - before the first sync
	 *  - between presize and size
	 *  - after preclose
	 */
	bool m_wasSize; //! True if size() was called by the Ghostscript library and the image buffer was set
	bool m_wasPage; //! True if page() was called by the Ghostscript library and the page rendering is finished

	QImage m_img;  //! Image on which we do the rendering
	void *ghostScriptInstance = NULL; //! Pointer to the Ghostscript library instance

	QString m_buffer; //! The IO buffer

private:
	void  argsToChar();
	int m_argsCCount;
	char** m_argsChar;
	QStringList m_args;
	QStringList m_internalArgs;
	GSError m_error;
	QString m_Output;

	friend int GSApiWrapper::open(void* /*handle*/,void* /*device */ );
	friend int GSApiWrapper::preclose(void* /*handle*/, void * /* device */ );
	friend int GSApiWrapper::close(void* /*handle*/, void * /* device */ );
	friend int GSApiWrapper::presize(void* /*handle*/, void * /* device*/, int /* width*/, int /* height*/,
	                                 int /* raster*/, unsigned int /* format */);
	friend int GSApiWrapper::sync(void* /*handle*/, void * /* device */ );
	friend int GSApiWrapper::separation(void* /*handle*/, void * /* device*/, int /* comp_num*/,
	                                    const char * /* name*/, unsigned short /* c*/, unsigned short /* m*/,
	                                    unsigned short /* y*/, unsigned short /* k*/);
	friend int GSApiWrapper::handleStdin(void* /*handle*/,  char* buffer, int len );
	friend int GSApiWrapper::handleStdout(void* /*handle*/,  const char* buffer, int len );
	friend int GSApiWrapper::handleStderr(void* /*handle*/,  const char* buffer, int len );
	friend int GSApiWrapper::update(void* /*handle*/, void * /* device*/, int /* x*/, int /* y*/, int /* w*/, int /* h*/);
	friend int GSApiWrapper::size(void* /*handle*/, void * /* device*/, int width, int height, int raster, unsigned int format, unsigned char *pimage);
	friend int GSApiWrapper::page(void* /*handle*/,void * /* device*/, int /* copies*/, int /* flush*/);
};

#endif
