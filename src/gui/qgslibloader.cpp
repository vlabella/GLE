/***********************************************************************************
 * QGLE - A Graphical Interface to GLE                                             *
 * Copyright (C) 2006 J. Struyf                                                    *
 *                                                                                 *
 * This program is free software; you can redistribute it and/or                   *
 * modify it under the terms of the GNU General Public License                     *
 * as published by the Free Software Foundation; either version 2                  *
 * of the License, or (at your option) any later version.                          *
 *                                                                                 *
 * This program is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   *
 * GNU General Public License for more details.                                    *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License               *
 * along with this program; if not, write to the Free Software                     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. *
 *                                                                                 *
 * Also add information on how to contact you by electronic and paper mail.        *
 ***********************************************************************************/


#include <QtWidgets>
#include <QTextStream>

#include "qgs.h"
#include "qgle_statics.h"

#ifdef Q_OS_WIN32
	#include <windows.h>
#endif
#if defined(Q_OS_HURD) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
	#include <dlfcn.h>
#endif

#include <string>
#include <sstream>

GSLibFunctions* g_GSLibInstance = NULL;

GSLibFunctions* GSLibFunctions::getInstance() {
	if (g_GSLibInstance == NULL) {
		g_GSLibInstance = new GSLibFunctions();
	}
	return g_GSLibInstance;
}

GSLibFunctions::GSLibFunctions() {
	hmodule = NULL;
	resetFunctions();
}

GSLibFunctions::~GSLibFunctions() {
	freeLibrary();
}

QString GSLibFunctions::getVersion() {
	gsapi_revision_t rv;
	/* check DLL version */
	if (gsapi_revision(&rv, sizeof(rv)) != 0) {
		return "ERR";
	} else {
		int major = rv.revision / 100;
		int minor = rv.revision % 100;
		if (minor < 10) return QString("%1.0%2").arg(major).arg(minor);
		else return QString("%1.%2").arg(major).arg(minor);
	}
}

int GSLibFunctions::getVersionMajor() {
	gsapi_revision_t rv;
	/* check DLL version */
	if (gsapi_revision(&rv, sizeof(rv)) != 0) {
		return -1;
	} else {
		return rv.revision / 100;
	}
}

void GSLibFunctions::resetFunctions() {
	gsapi_revision = NULL;
	gsapi_new_instance = NULL;
	gsapi_delete_instance = NULL;
	gsapi_init_with_args = NULL;
	gsapi_run_string = NULL;
	gsapi_run_string_with_length = NULL;
	gsapi_run_string_begin = NULL;
	gsapi_run_string_continue = NULL;
	gsapi_run_string_end = NULL;
	gsapi_exit = NULL;
	gsapi_set_stdio = NULL;
	gsapi_set_poll = NULL;
	gsapi_set_display_callback = NULL;
	gsapi_register_callout = NULL;
}

bool GSLibFunctions::isLoaded() {
	return gsapi_revision != NULL;
}

void GSLibFunctions::StripWhiteSpace(QString& str) {
	while (str.size() > 0 && (str[str.size()-1] == ' '  ||
	                          str[str.size()-1] == '\n' ||
	                          str[str.size()-1] == '\r')) {
		str.truncate(str.size()-1);
	}
}

#ifdef Q_OS_WIN32

void GSLibFunctions::tryLocation(const char* str) {
}

int GSLibFunctions::loadLibrary(const QString& location, QString& last_error) {
	gsapi_revision_t rv;
	/* Try to open the library */
	if (location == "") {
		m_LibGSLocation = "gsdll32.dll";
		#if defined(_WIN64) || defined(__x86_64__)
			m_LibGSLocation = "gsdll64.dll";
		#endif
	std::string tmp = m_LibGSLocation.toStdString();
		hmodule = LoadLibrary((const WCHAR*)m_LibGSLocation.unicode());
		//hmodule = LoadLibrary(tmp.c_str());
	} else {
		m_LibGSLocation = location;
		std::string tmp = m_LibGSLocation.toStdString();
		hmodule = LoadLibrary((const WCHAR*)m_LibGSLocation.unicode());
		//hmodule = LoadLibrary(tmp.c_str());
	}
	if (hmodule < (HINSTANCE)HINSTANCE_ERROR) {
		/* Failed */
		LPWSTR lpMsgBuf = NULL;
		DWORD dwError = GetLastError();
		if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
		                   0, (LPWSTR) &lpMsgBuf, 0, NULL ) != 0) {
			/* Error code known */
			last_error.setUtf16((const ushort *)lpMsgBuf, wcslen(lpMsgBuf));
			StripWhiteSpace(last_error);
			if (last_error.endsWith(".")) last_error.truncate(last_error.size()-1);
			LocalFree(lpMsgBuf);
		} else {
			/* Unknown error code */
			last_error = QString("Code #%1").arg(dwError);
		}
		hmodule = (HINSTANCE)0;
		if (dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_PATH_NOT_FOUND || dwError == ERROR_MOD_NOT_FOUND) return 2;
		else return 1;
	}
	/* Get pointers to functions */
	gsapi_revision = (PFN_gsapi_revision) GetProcAddress(hmodule, "gsapi_revision");
	if (gsapi_revision == NULL) {
		last_error = "Can't find gsapi_revision";
		freeLibrary();
		return 1;
	}
	/* check DLL version */
	if (gsapi_revision(&rv, sizeof(rv)) != 0) {
		last_error = "Unable to identify Ghostscript DLL revision";
		freeLibrary();
		return 1;
	}
	gsapi_new_instance = (PFN_gsapi_new_instance) GetProcAddress(hmodule,"gsapi_new_instance");
	if (gsapi_new_instance == NULL) {
		last_error = "Can't find gsapi_new_instance";
		freeLibrary();
		return 1;
	}
	gsapi_delete_instance = (PFN_gsapi_delete_instance) GetProcAddress(hmodule, "gsapi_delete_instance");
	if (gsapi_delete_instance == NULL) {
		last_error = "Can't find gsapi_delete_instance";
		freeLibrary();
		return 1;
	}
	gsapi_set_stdio = (PFN_gsapi_set_stdio) GetProcAddress(hmodule, "gsapi_set_stdio");
	if (gsapi_set_stdio == NULL) {
		last_error = "Can't find gsapi_set_stdio";
		freeLibrary();
		return 1;
	}
	gsapi_set_poll = (PFN_gsapi_set_poll) GetProcAddress(hmodule, "gsapi_set_poll");
	if (gsapi_set_poll == NULL) {
		last_error = "Can't find gsapi_set_poll";
		freeLibrary();
		return 1;
	}
	gsapi_set_display_callback = (PFN_gsapi_set_display_callback) GetProcAddress(hmodule, "gsapi_set_display_callback");
	if (gsapi_set_display_callback == NULL) {
		last_error = "Can't find gsapi_set_display_callback";
		freeLibrary();
		return 1;
	}
	gsapi_register_callout = (PFN_gsapi_register_callout) GetProcAddress(hmodule, "gsapi_register_callout");
	if (gsapi_register_callout == NULL) {
		last_error = "Can't find gsapi_register_callout";
		freeLibrary();
		return 1;
	}
	gsapi_init_with_args = (PFN_gsapi_init_with_args) GetProcAddress(hmodule, "gsapi_init_with_args");
	if (gsapi_init_with_args == NULL) {
		last_error = "Can't find gsapi_init_with_args";
		freeLibrary();
		return 1;
	}
	gsapi_run_string = (PFN_gsapi_run_string) GetProcAddress(hmodule, "gsapi_run_string");
	if (gsapi_run_string == NULL) {
		last_error = "Can't find gsapi_run_string";
		freeLibrary();
		return 1;
	}
	gsapi_run_string_with_length = (PFN_gsapi_run_string_with_length) GetProcAddress(hmodule, "gsapi_run_string_with_length");
	if (gsapi_run_string_with_length == NULL) {
		last_error = "Can't find gsapi_run_string_with_length";
		freeLibrary();
		return 1;
	}
	gsapi_run_string_begin = (PFN_gsapi_run_string_begin) GetProcAddress(hmodule, "gsapi_run_string_begin");
	if (gsapi_run_string_begin == NULL) {
		last_error = "Can't find gsapi_run_string_begin";
		freeLibrary();
		return 1;
	}
	gsapi_run_string_continue = (PFN_gsapi_run_string_continue) GetProcAddress(hmodule, "gsapi_run_string_continue");
	if (gsapi_run_string_continue == NULL) {
		last_error = "Can't find gsapi_run_string_continue";
		freeLibrary();
		return 1;
	}
	gsapi_run_string_end = (PFN_gsapi_run_string_end) GetProcAddress(hmodule, "gsapi_run_string_end");
	if (gsapi_run_string_end == NULL) {
		last_error = "Can't find gsapi_run_string_end";
		freeLibrary();
		return 1;
	}
	gsapi_exit = (PFN_gsapi_exit) GetProcAddress(hmodule, "gsapi_exit");
	if (gsapi_exit == NULL) {
		last_error = "Can't find gsapi_exit";
		freeLibrary();
		return 1;
	}
	return 0;
}

void GSLibFunctions::freeLibrary() {
	resetFunctions();
	if (hmodule != (HINSTANCE)NULL) FreeLibrary(hmodule);
	hmodule = NULL;
}

#endif

#if defined(Q_OS_HURD) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)

void* dlopenImpl(const char* fname) {
	#ifdef RTLD_DEEPBIND
		return dlopen(fname, RTLD_NOW | RTLD_DEEPBIND);
	#else
		#ifdef Q_OS_LINUX
			// Only pertains to Linux so silence warning on other OS
			#warning "dlopen - not using RTLD_DEEPBIND"
		#endif
		return dlopen(fname, RTLD_NOW);
	#endif
}

void GSLibFunctions::tryLocation(const char* str) {
	if (hmodule == NULL) {
		m_LibGSLocation = str;
		QByteArray strdata = m_LibGSLocation.toLatin1();
		hmodule = dlopenImpl(strdata.constData());
	}
}

void GSLibFunctions::tryLocationLoop(const char* prefix) {
   std::string soName(prefix);
   soName += "/libgs.so";
   for (int version = 12; version >= 6; version--) {
      std::ostringstream toTry;
      toTry << soName << "." << version;
      tryLocation(toTry.str().c_str());
   }
   tryLocation(soName.c_str());
}

int GSLibFunctions::loadLibrary(const QString& location, QString& last_error) {
	gsapi_revision_t rv;
	// try to open the library
	// no need to hard code paths - dlopen() searches in appropriate locations
	// if it fails then its not installed properly
	if (location == "") {
		#ifdef Q_OS_LINUX
		tryLocation("libgs.so");
		#endif
		#ifdef Q_OS_MACOS
		tryLocation("libgs.dylib");
		#endif
		/*
		#ifdef Q_OS_HURD
		tryLocationLoop("/usr/lib/i386-gnu/");
		#endif // HURD
		#ifdef Q_OS_LINUX
		#if defined(__x86_64__) || defined(__ppc64__) || defined (__s390x__) || defined (__sparc64__)
			// try 64 bit libraries on 64 bit system
			tryLocationLoop("/usr/lib64");
			tryLocationLoop("/usr/local/lib64");
			tryLocationLoop("/usr/lib64/x86_64-linux-gnu");
			tryLocationLoop("/usr/local/lib64/x86_64-linux-gnu");
		#endif // 64 bit
		tryLocationLoop("/usr/lib");
		tryLocationLoop("/usr/local/lib");
		tryLocationLoop("/usr/lib/x86_64-linux-gnu");
		tryLocationLoop("/usr/local/lib/x86_64-linux-gnu");
		#endif // Q_OS_LINUX
		#ifdef Q_OS_MACOS
		tryLocation("/usr/lib/libgs.dylib");
		tryLocation("/usr/local/lib/libgs.dylib");
		#endif
		#ifdef EXTRA_GSLIB_SEARCH_LOCATION
		tryLocation(EXTRA_GSLIB_SEARCH_LOCATION);
		#endif
		*/
	} else {
		m_LibGSLocation = location;
		QString libloc = location;
		if (libloc.endsWith(".framework")) {
			libloc.append("/Ghostscript");
		}
		QByteArray strdata = libloc.toLatin1();
		hmodule = dlopenImpl(strdata.constData());
	}
	/* Error message if loading fails */
	if (hmodule == NULL) {
		/* Failed */
		last_error = QString::fromLatin1(dlerror());
		return 2;
	}
	/* Get pointers to functions */
	gsapi_revision = (PFN_gsapi_revision) dlsym(hmodule, "gsapi_revision");
	if (gsapi_revision == NULL) {
		last_error = "Can't find gsapi_revision";
		freeLibrary();
		return 1;
	}
	/* check DLL version */
	if (gsapi_revision(&rv, sizeof(rv)) != 0) {
		last_error = "Unable to identify Ghostscript DLL revision - it must be newer than needed";
		freeLibrary();
		return 1;
	}
	gsapi_new_instance = (PFN_gsapi_new_instance) dlsym(hmodule,"gsapi_new_instance");
	if (gsapi_new_instance == NULL) {
		last_error = "Can't find gsapi_new_instance";
		freeLibrary();
		return 1;
	}
	gsapi_delete_instance = (PFN_gsapi_delete_instance) dlsym(hmodule, "gsapi_delete_instance");
	if (gsapi_delete_instance == NULL) {
		last_error = "Can't find gsapi_delete_instance";
		freeLibrary();
		return 1;
	}
	gsapi_set_stdio = (PFN_gsapi_set_stdio) dlsym(hmodule, "gsapi_set_stdio");
	if (gsapi_set_stdio == NULL) {
		last_error = "Can't find gsapi_set_stdio";
		freeLibrary();
		return 1;
	}
	gsapi_set_poll = (PFN_gsapi_set_poll) dlsym(hmodule, "gsapi_set_poll");
	if (gsapi_set_poll == NULL) {
		last_error = "Can't find gsapi_set_poll";
		freeLibrary();
		return 1;
	}
	gsapi_set_display_callback = (PFN_gsapi_set_display_callback) dlsym(hmodule, "gsapi_set_display_callback");
	if (gsapi_set_display_callback == NULL) {
		last_error = "Can't find gsapi_set_display_callback";
		freeLibrary();
		return 1;
	}
	gsapi_register_callout = (PFN_gsapi_register_callout) dlsym(hmodule, "gsapi_register_callout");
	if (gsapi_register_callout == NULL) {
		last_error = "Can't find gsapi_register_callout";
		freeLibrary();
		return 1;
	}
	gsapi_init_with_args = (PFN_gsapi_init_with_args) dlsym(hmodule, "gsapi_init_with_args");
	if (gsapi_init_with_args == NULL) {
		last_error = "Can't find gsapi_init_with_args";
		freeLibrary();
		return 1;
	}
	gsapi_run_string = (PFN_gsapi_run_string) dlsym(hmodule, "gsapi_run_string");
	if (gsapi_run_string == NULL) {
		last_error = "Can't find gsapi_run_string";
		freeLibrary();
		return 1;
	}
	gsapi_run_string_with_length = (PFN_gsapi_run_string_with_length) dlsym(hmodule, "gsapi_run_string_with_length");
	if (gsapi_run_string_with_length == NULL) {
		last_error = "Can't find gsapi_run_string_with_length";
		freeLibrary();
		return 1;
	}
	gsapi_run_string_begin = (PFN_gsapi_run_string_begin) dlsym(hmodule, "gsapi_run_string_begin");
	if (gsapi_run_string_begin == NULL) {
		last_error = "Can't find gsapi_run_string_begin";
		freeLibrary();
		return 1;
	}
	gsapi_run_string_continue = (PFN_gsapi_run_string_continue) dlsym(hmodule, "gsapi_run_string_continue");
	if (gsapi_run_string_continue == NULL) {
		last_error = "Can't find gsapi_run_string_continue";
		freeLibrary();
		return 1;
	}
	gsapi_run_string_end = (PFN_gsapi_run_string_end) dlsym(hmodule, "gsapi_run_string_end");
	if (gsapi_run_string_end == NULL) {
		last_error = "Can't find gsapi_run_string_end";
		freeLibrary();
		return 1;
	}
	gsapi_exit = (PFN_gsapi_exit) dlsym(hmodule, "gsapi_exit");
	if (gsapi_exit == NULL) {
		last_error = "Can't find gsapi_exit";
		freeLibrary();
		return 1;
	}
	return 0;
}

void GSLibFunctions::freeLibrary() {
	resetFunctions();
	if (hmodule != NULL) dlclose(hmodule);
	hmodule = NULL;
}

#endif
