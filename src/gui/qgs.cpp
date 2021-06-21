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

#include "qgs.h"
#include <math.h>

#define MINIMUM(a, b)    ((a) < (b) ? (a) : (b))

GSInterpreterLib* g_GSInterpreterLib = NULL;

namespace GSApiWrapper
{

int GSDLLCALL handleStdin(void *, char *, int)
{
	return 0;
}

int GSDLLCALL handleStdout(void *, const char *str, int len)
{
	if (g_GSInterpreterLib != NULL) {
		return g_GSInterpreterLib->gs_output(str,len);
	}
	return 0;
}

int GSDLLCALL handleStderr(void *, const char *str, int len)
{
	if (g_GSInterpreterLib != NULL) {
		return g_GSInterpreterLib->gs_error(str,len);
	}
	return 0;
}

int open(void*, void*)
{
	return 0;
}

int preclose(void *, void *)
{
	return 0;
}

int close(void *, void *)
{
	return 0;
}


int presize(void *, void * device, int width, int height,
        int raster, unsigned int format)
{
	if (g_GSInterpreterLib != NULL) {
		return g_GSInterpreterLib->presize(device, width, height, raster, format);
	}
	return 0;
}

int size(void *, void *device, int width, int height, int raster, unsigned int format, unsigned char *pimage)
{
	if (g_GSInterpreterLib != NULL) {
		return g_GSInterpreterLib->size(device, width, height, raster, format, pimage);
	}
	return 0;
}

int sync(void*, void*)
{
	return 0;
}

int page(void *, void * device, int copies, int flush)
{
	if (g_GSInterpreterLib != NULL) {
		return g_GSInterpreterLib->page(device, copies, flush);
	}
	return 0;
}

int update(void*, void *, int, int, int, int)
{
	return 0;
}

int separation(void*, void*, int, const char*, unsigned short, unsigned short, unsigned short, unsigned short)
{
	return 0;
}

int rectangle_request(void *handle, void *device,void **memory, int *ox, int *oy,int *raster, int *plane_raster,int *x, int *y, int *w, int *h)
{
	// signal no more rectangles
	*w = 0;
	*h = 0;
	return 0;
}

#ifdef USE_INTERNAL_GSAPI
	// old gs api - depricated - 9.2 and earlier
	// will not work with newer gs that has V3
	#define GS_ERROR(name) e_##name
	#ifdef QGS_USE_MAJOR_V2
	// Newer versions of GhostScript support "separation", but we don't need this
	typedef display_callback display_callback_qgs;
	display_callback_qgs device = {
		sizeof ( display_callback_qgs ),
		DISPLAY_VERSION_MAJOR,
		DISPLAY_VERSION_MINOR,
		&open,
		&preclose,
		&close,
		&presize,
		&size,
		&sync,
		&page,
		&update,
		NULL,
		NULL,
		&separation
	};
	#else
	// For now use older version of structure, which is compatible with GhostScript 8.15
	// This version is shipped with Fedora Core 5
	typedef struct display_callback_v1_s display_callback_qgs;
	display_callback_qgs device = {
		sizeof ( display_callback_qgs ),
		DISPLAY_VERSION_MAJOR_V1,
		DISPLAY_VERSION_MINOR_V1,
		&open,
		&preclose,
		&close,
		&presize,
		&size,
		&sync,
		&page,
		&update,
		NULL,
		NULL
	};
	#endif
#else
	// using latest and greatest version of ghostpdl
	#define GS_ERROR(name) gs_error_##name
	//
	// latest and greatest V3 has adjust_band_height rectangle_request which are not utilized
	// but must be present for when using gsapi_register_callout function
	//
	typedef struct display_callback_s display_callback_qgs;
	display_callback_qgs device = {
		sizeof ( display_callback_qgs ),
		DISPLAY_VERSION_MAJOR,
		DISPLAY_VERSION_MINOR,
		&open,
		&preclose,
		&close,
		&presize,
		&size,
		&sync,
		&page,
		&update,
		NULL,
		NULL,
		&separation,
		NULL,
		&rectangle_request
	};
#endif

}

/********* GSInterpreterLib ************/

GSInterpreterLib::GSInterpreterLib() :
	m_running(false),
	m_cnvmode(QGS_DISPLAY_32),
	m_alpha(false),
	m_conv(NULL),
	m_orientation(0),
	m_magnify(1.0),
	m_width(0),
	m_height(0),
	m_dpi(100.0),
	m_media(QString::null),
	m_Gwidth(0),
	m_Gheight(0),
	m_imageChar(0),
	m_format(0),
	m_raster(0),
	m_textaa(1),
	m_graphaa(1),
	m_pfonts(true),
	m_display(true),
	m_wasSize(false),
	m_wasPage(false),
	m_argsCCount(0),
	m_argsChar(0),
	ghostScriptInstance(NULL)
{
	m_gs = GSLibFunctions::getInstance();
	if (!m_gs->isLoaded()) return;
	int exit = m_gs->gsapi_new_instance(&ghostScriptInstance, this);
	if (exit && !handleExit(exit)) return;
	if (m_gs->getVersionMajor() < 8) {
		/* 32 bit does not work with GhostView 7.04 */
		m_cnvmode = QGS_DISPLAY_24;
	}
	g_GSInterpreterLib = this;
}

GSInterpreterLib::~GSInterpreterLib()
{
	if (running())	m_gs->gsapi_exit(ghostScriptInstance);
	if (m_argsChar)
	{
		for (int i=0;i<m_argsCCount;i++) delete [] *(m_argsChar+i);
		delete [] m_argsChar;
	}
	if (m_gs->isLoaded()) m_gs->gsapi_delete_instance(ghostScriptInstance);
	if (m_conv != NULL) delete[] m_conv;
}

// interpreter state functions

static int
  my_callout_handler(void *instance,
                     void *callout_handle,
                     const char *device_name,
                     int id,
                     int size,
                     void *data)
{
    /* On entry, callout_handle == the value of state passed in
     * to gsapi_register_callout. */
    /* We are only interested in callouts from the display device. */
    if (device_name == NULL || strcmp(device_name, "display"))
        return -1;

    if (id == DISPLAY_CALLOUT_GET_CALLBACK)
    {
        /* Fill in the supplied block with the details of our callback
         * handler, and the handle to use. In this instance, the handle
         * is the pointer to our test structure. */
        gs_display_get_callback_t *cb = (gs_display_get_callback_t *)data;
        cb->callback = (display_callback*)(&(GSApiWrapper::device));
        cb->caller_handle = callout_handle;

        return 0;
    }
    return -1;
}


bool GSInterpreterLib::start(bool setStdio)
{
	if ( setStdio )
	{
		m_gs->gsapi_set_stdio(ghostScriptInstance, &(GSApiWrapper::handleStdin),
		                                           &(GSApiWrapper::handleStdout),
		                                           &(GSApiWrapper::handleStderr));
	}
	// depricated - use register_callout
	//int call = m_gs->gsapi_set_display_callback(ghostScriptInstance, (display_callback*)(&(GSApiWrapper::device)));
	int state = 0;
	int call = m_gs->gsapi_register_callout(ghostScriptInstance, my_callout_handler, &state);
	if (call && !handleExit(call)) return false;
	argsToChar();
	call = m_gs->gsapi_init_with_args(ghostScriptInstance, m_argsCCount, m_argsChar);
	if (call && !handleExit(call)) return false;
	QString set;
	set.sprintf("<< /Orientation %d >> setpagedevice .locksafe",m_orientation);
	QByteArray strdata = set.toLatin1();
	m_gs->gsapi_run_string_with_length(ghostScriptInstance, strdata.constData(), set.length(), 0, &call);
	m_running = handleExit (call);
	return m_running;
}

bool GSInterpreterLib::stop()
{
	if (m_running)
	{
		int exit = m_gs->gsapi_exit(ghostScriptInstance);
		if (exit) handleExit(exit);
		m_running = false;
	}
	return m_running;
}

// set options
void GSInterpreterLib::setGhostscriptArguments(const QStringList &list)
{
	if ( m_args != list )
	{
		m_args=list;
		stop();
	}
}

void GSInterpreterLib::setOrientation( int orientation )
{
	if( m_orientation != orientation )
	{
		m_orientation = orientation;
		stop();
	}
}

void GSInterpreterLib::setMagnify( double magnify )
{
	if( m_magnify != magnify )
	{
		m_magnify = magnify;
		stop();
	}
}

void GSInterpreterLib::setMedia( const QString &media )
{
	if( m_media != media )
	{
		m_media = media;
		stop();
	}
}

void GSInterpreterLib::setSize( int w, int h )
{
	if ( m_width != w )
	{
		m_width=w;
		stop();
	}
	if ( m_height != h )
	{
		m_height=h;
		stop();
	}
}

void GSInterpreterLib::setDPI( double dpi )
{
	if ( m_dpi != dpi )
	{
		m_dpi=dpi;
		stop();
	}
}

void GSInterpreterLib::setDisplay( bool display )
{
	if( m_display != display )
	{
		m_display = display;
		stop();
	}
}

void GSInterpreterLib::setAlpha( bool alpha )
{
	m_alpha = alpha;
}

void GSInterpreterLib::setPlatformFonts( bool pfonts )
{
	if( m_pfonts != pfonts )
	{
		m_pfonts = pfonts;
		stop();
	}
}

void GSInterpreterLib::setAABits(int text, int graphics )
{
	if( m_textaa != text )
	{
		m_textaa = text;
		stop();
	}
	if( m_graphaa != graphics )
	{
		m_graphaa = graphics;
		stop();
	}
}


void GSInterpreterLib::setBuffered( bool buffered )
{
	if( m_buffered != buffered ) m_buffered = buffered;
}

void GSInterpreterLib::setOrigin(double ox, double oy)
{
	m_OriginX = ox;
	m_OriginY = oy;
}

QString GSInterpreterLib::getOutpuStreamText()
{
	return m_Output;
}

bool GSInterpreterLib::sendOriginTranslate()
{
	if (m_OriginX != 0.0 || m_OriginY != 0.0) {
		// Add a translate if bounding box not at origin zero (to support LaTeX processed output)
		int exit_code;
		QString str = QString("%1 %2 translate\n").arg(-m_OriginX).arg(-m_OriginY);
		QByteArray strdata = str.toLatin1();
		m_gs->gsapi_run_string_continue(ghostScriptInstance, strdata.constData(), str.length(), 0, &exit_code);
		if (exit_code && !handleExit(exit_code)) return false;
	}
	return true;
}

bool GSInterpreterLib::run(QFile& file)
{
	int exit_code;
	char buf[4096];
	if (!m_gs->isLoaded()) return false;
	if (!running()) start();
	m_wasPage = false;
	m_gs->gsapi_run_string_begin(ghostScriptInstance, 0, &exit_code);
	if (exit_code && !handleExit(exit_code)) return false;
	if (!sendOriginTranslate()) return false;
	while (true) {
		qint64 nbRead = file.read(buf, sizeof(buf)-1);
		if (nbRead <= 0) {
			// Make sure to exit if nothing more in file
			break;
		}
		buf[nbRead] = 0;
		m_gs->gsapi_run_string_continue(ghostScriptInstance, buf, nbRead, 0, &exit_code);
		if (exit_code && !handleExit(exit_code)) return false;
	}
	m_gs->gsapi_run_string_end(ghostScriptInstance, 0, &exit_code);
	if (exit_code && !handleExit(exit_code)) return false;
	return true;
}

bool GSInterpreterLib::run(const string* data)
{
	int exit_code;
	int return_code;
	if (!m_gs->isLoaded()) return false;
	if (!running()) start();
	m_wasPage = false;
	return_code = m_gs->gsapi_run_string_begin(ghostScriptInstance, 0, &exit_code);
	if (exit_code && !handleExit(exit_code)) return false;
	if (!sendOriginTranslate()) return false;
	unsigned int toRead = data->size();
	unsigned int offset = 0;
	while (toRead > 0)
	{
		unsigned int maxSize = 65535;
		unsigned int readNow = std::min(toRead, maxSize);
		return_code = m_gs->gsapi_run_string_continue(ghostScriptInstance, data->data() + offset, readNow, 0, &exit_code);
		if (exit_code && !handleExit(exit_code)) return false;
		offset += readNow;
		toRead -= readNow;
	}
	return_code = m_gs->gsapi_run_string_end(ghostScriptInstance, 0, &exit_code);
	if (exit_code && !handleExit(exit_code)) return false;
	return true;
}

bool GSInterpreterLib::startRender()
{
	int exit_code = 0;
	if (!m_gs->isLoaded()) return false;
	if (!running()) start();
	m_wasPage = false;
	m_gs->gsapi_run_string_begin(ghostScriptInstance, 0, &exit_code);
	if (exit_code && !handleExit(exit_code)) return false;
	else return true;
}

bool GSInterpreterLib::nextRender(const char* str)
{
	int exit_code = 0;
	m_gs->gsapi_run_string_continue(ghostScriptInstance, str, strlen(str), 0, &exit_code);
	if (exit_code && !handleExit(exit_code)) return false;
	else return true;
}

bool GSInterpreterLib::endRender()
{
	int exit_code = 0;
	m_gs->gsapi_run_string_end(ghostScriptInstance, 0, &exit_code);
	if (exit_code && !handleExit(exit_code)) return false;
	else return true;
}

int GSInterpreterLib::gs_output(const char* buffer, int len)
{
	m_Output.append(QString::fromLocal8Bit(buffer, len));
	return len;
}

int GSInterpreterLib::gs_error(const char* buffer, int len)
{
	m_Output.append(QString::fromLocal8Bit(buffer, len));
	return len;
}

int GSInterpreterLib::presize(void * /* device*/, int width, int height, int raster, unsigned int format)
{
	m_Gwidth=width;
	m_Gheight=height;
	m_raster=raster;
	m_format=format;
	return 0;
}

int GSInterpreterLib::size(void * /* device*/, int /* wd */, int /* hi*/, int /*raster*/, unsigned int /*format*/, unsigned char *pimage)
{
	m_wasSize = true;
	m_imageChar = pimage;
	return 0;
}

int GSInterpreterLib::page(void * /* device*/, int /* copies*/, int /* flush*/)
{
	m_wasPage=true;
	// assume the image given by the gs is of the requested widthxheight
	if (m_cnvmode == QGS_DISPLAY_24) {
		int dest = 0;
		if (m_conv != NULL) delete[] m_conv;
		m_conv = new unsigned char[m_Gwidth*m_Gheight*4];
		for (int j = 0; j < m_Gheight; j++) {
			int src = j * m_raster;
			if (m_alpha) {
				for (int i = 0; i < m_Gwidth; i++) {
					unsigned char red = m_conv[dest++] = m_imageChar[src++];
					unsigned char green = m_conv[dest++] = m_imageChar[src++];
					unsigned char blue = m_conv[dest++] = m_imageChar[src++];
					if (red == 0xFF && green == 0xFF && blue == 0xFF) {
						m_conv[dest++] = 0;
					} else {
						m_conv[dest++] = 0xFF;
					}
				}
			} else {
				for (int i = 0; i < m_Gwidth; i++) {
					m_conv[dest++] = m_imageChar[src++];
					m_conv[dest++] = m_imageChar[src++];
					m_conv[dest++] = m_imageChar[src++];
					m_conv[dest++] = 0;
				}
			}
		}
		if (m_alpha) {
			QImage image(m_conv, m_Gwidth, m_Gheight, QImage::Format_ARGB32);
			m_img = image.copy();
		} else {
			QImage image(m_conv, m_Gwidth, m_Gheight, QImage::Format_RGB32);
			m_img = image.copy();
		}
	} else {
		if (m_alpha) {
			int dest = 0;
			for (int j = 0; j < m_Gheight; j++) {
				int src = j * m_raster;
				for (int i = 0; i < m_Gwidth; i++) {
					unsigned char red = m_imageChar[dest++] = m_imageChar[src++];
					unsigned char green = m_imageChar[dest++] = m_imageChar[src++];
					unsigned char blue = m_imageChar[dest++] = m_imageChar[src++];
					if (red == 0xFF && green == 0xFF && blue == 0xFF) {
						m_imageChar[dest++] = 0x0;
					} else {
						m_imageChar[dest++] = 0xFF;
					}
					src++;
				}
			}
			QImage image(m_imageChar, m_Gwidth, m_Gheight, QImage::Format_ARGB32);
			m_img = image.copy();
		} else {
			if (m_raster != 4*m_Gwidth) {
				int dest = 0;
				for (int j = 0; j < m_Gheight; j++) {
					int src = j * m_raster;
					for (int i = 0; i < m_Gwidth; i++) {
						m_imageChar[dest++] = m_imageChar[src++];
						m_imageChar[dest++] = m_imageChar[src++];
						m_imageChar[dest++] = m_imageChar[src++];
						m_imageChar[dest++] = 0;
						src++;
					}
				}
			}
			QImage image(m_imageChar, m_Gwidth, m_Gheight, QImage::Format_RGB32);
			m_img = image.copy();
		}
	}
	return 0;
}

const QImage& GSInterpreterLib::getImage() {
	return m_img;
}

bool GSInterpreterLib::handleExit(int code)
{
	// qWarning("Got code %d",code);
	if ( code>=0 )
		return true;
	else if ( code <= -100 )
	{
		switch (code)
		{
			case GS_ERROR(Fatal):
				m_error.setError(GS_ERROR(Fatal), "fatal internal error");
				return false;
			case GS_ERROR(ExecStackUnderflow):
				m_error.setError(GS_ERROR(ExecStackUnderflow), "stack overflow");
				return false;
			// no error or not important
			case GS_ERROR(Quit):
			case GS_ERROR(Info):
			case GS_ERROR(InterpreterExit):
			case GS_ERROR(NeedInput):
#ifdef USE_INTERNAL_GSAPI
			case GS_ERROR(RemapColor):
			case GS_ERROR(NeedStdin):
			case GS_ERROR(NeedStdout):
			case GS_ERROR(NeedStderr):
#else
			case GS_ERROR(Remap_Color):
			case GS_ERROR(NeedFile):
#endif
			case GS_ERROR(VMreclaim):
			default:
				return true;
		}
	}
	else
	{
		const char* errors[]= { "", ERROR_NAMES };
		m_error.setError(code, errors[-code]);
		return false;
	}
}

void GSInterpreterLib::argsToChar()
{
	if (m_argsChar)
	{
		for (int i=0;i<m_argsCCount;i++) delete [] *(m_argsChar+i);
		delete [] m_argsChar;
	}
	QStringList internalArgs;
	if (m_args.isEmpty())
	{
		internalArgs << "-dMaxBitmap=10000000";
		#ifdef Q_WS_MAC
			int fwpos = m_gs->libGSLocation().lastIndexOf(QString(".framework"));
			// If the Ghostscript framework is not installed in a default location it won't find its resources and fonts
			if (fwpos != -1) {
				QString path = m_gs->libGSLocation().left(fwpos+10);
				internalArgs << QString("-I%1/Resources/lib").arg(path);
				internalArgs << QString("-I%1/Resources/fonts").arg(path);
			}
		#endif
		internalArgs << "-dSAFER";
		internalArgs << "-dBATCH";
		internalArgs << "-dNOPAUSE";
		internalArgs << "-dNOPAGEPROMPT";
		internalArgs << QString("-r%1").arg(m_dpi);
		internalArgs << QString("-g%1x%2").arg(m_width).arg(m_height);
		internalArgs << QString("-dDisplayResolution=%1").arg(m_dpi);
		internalArgs << QString("-dTextAlphaBits=%1").arg(m_textaa);
		internalArgs << QString("-dGraphicsAlphaBits=%1").arg(m_graphaa);
		if ( !m_pfonts ) internalArgs << "-dNOPLATFONTS";
	}
	else
	{
		internalArgs = m_args;
	}

	if (m_display)
	{
		int format = DISPLAY_COLORS_RGB | DISPLAY_DEPTH_8 | DISPLAY_LITTLEENDIAN | DISPLAY_TOPFIRST;
		if (m_cnvmode == QGS_DISPLAY_32) {
			if (m_alpha) {
				// GhostScript does not support ALPHA channel
				// format |= DISPLAY_ALPHA_LAST;
				format |= DISPLAY_UNUSED_LAST;
			} else {
				format |= DISPLAY_UNUSED_LAST;
			}
		}
		internalArgs << "-sDEVICE=display";
		internalArgs << QString().sprintf("-dDisplayFormat=%d", format);
	}

	int t=internalArgs.count();
	char ** args=static_cast <char**> (new char* [t]);
	for (int i=0;i<t;i++)
	{
		*(args+i)=new char [internalArgs[i].length()+1];
		qstrcpy (*(args+i),internalArgs[i].toLocal8Bit());
	}
	m_argsChar=args;
	m_argsCCount=t;

}

bool GSInterpreterLib::hasError()
{
	return m_error.hasError();
}

const GSError& GSInterpreterLib::getError()
{
	return m_error;
}

GSError::GSError()
{
	m_code = 0;
	m_haserr = false;
}

GSError::~GSError()
{
}

void GSError::setError(int code, QString msg)
{
	m_code = code;
	m_name = msg;
	m_haserr = true;
}
