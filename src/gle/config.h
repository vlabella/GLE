/************************************************************************
 *                                                                      *
 * GLE - Graphics Layout Engine <http://www.gle-graphics.org/>          *
 *                                                                      *
 * Modified BSD License                                                 *
 *                                                                      *
 * Copyright (C) 2009 GLE.                                              *
 *                                                                      *
 * Redistribution and use in source and binary forms, with or without   *
 * modification, are permitted provided that the following conditions   *
 * are met:                                                             *
 *                                                                      *
 *    1. Redistributions of source code must retain the above copyright *
 * notice, this list of conditions and the following disclaimer.        *
 *                                                                      *
 *    2. Redistributions in binary form must reproduce the above        *
 * copyright notice, this list of conditions and the following          *
 * disclaimer in the documentation and/or other materials provided with *
 * the distribution.                                                    *
 *                                                                      *
 *    3. The name of the author may not be used to endorse or promote   *
 * products derived from this software without specific prior written   *
 * permission.                                                          *
 *                                                                      *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR   *
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED       *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY       *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL   *
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE    *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS        *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER *
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      *
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN  *
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                        *
 *                                                                      *
 ************************************************************************/

#ifndef GLE_CONFIG_H
#define GLE_CONFIG_H

typedef enum {
	GLE_OPT_HELP,
	GLE_OPT_INFO,
	GLE_OPT_VERBOSITY,
	GLE_OPT_DEVICE,
	GLE_OPT_CAIRO,
	GLE_OPT_DPI,
	GLE_OPT_FULL_PAGE,
	GLE_OPT_LANDSCAPE,
	GLE_OPT_OUTPUT,
	GLE_OPT_NOSAVE,
	GLE_OPT_COPY,
	GLE_OPT_PREVIEW,
	GLE_OPT_GSPREVIEW,
	GLE_OPT_VERSION,
	GLE_OPT_COMPAT,
	GLE_OPT_CALC,
	GLE_OPT_CATCSV,
	GLE_OPT_TEX,
	GLE_OPT_NO_PDFTEX,
	GLE_OPT_CREATE_INC,
	GLE_OPT_TEXINCPREF,
	GLE_OPT_PAUSE,
	GLE_OPT_MKINITTEX,
	GLE_OPT_FINDDEPS,
	GLE_OPT_NO_COLOR,
	GLE_OPT_INVERSE,
	GLE_OPT_TRANSPARENT,
	GLE_OPT_BBTWEAK,
	GLE_OPT_NO_CTRL_D,
	GLE_OPT_NO_MAXPATH,
	GLE_OPT_NO_LIGATURES,
	GLE_OPT_GSOPTIONS,
	GLE_OPT_SAFEMODE,
	GLE_OPT_ALLOWREAD,
	GLE_OPT_ALLOWWRITE,
	GLE_OPT_KEEP,
	GLE_OPT_TRACE,
	GLE_OPT_DEBUG
} GLECmdLineOption;

#define GLE_CONFIG_GLE       0
#define GLE_CONFIG_TOOLS     1
#define GLE_CONFIG_TEX       2
#define GLE_CONFIG_PAPER     3

#define GLE_CONFIG_GLE_VERSION    0
#define GLE_CONFIG_GLE_INSTALL    1

#define GLE_CONFIG_PAPER_SIZE     0
#define GLE_CONFIG_PAPER_MARGINS  1

typedef enum {
	GLE_TOOL_PDFTEX_CMD,
	GLE_TOOL_PDFTEX_OPTIONS,
	GLE_TOOL_LATEX_CMD,
	GLE_TOOL_LATEX_OPTIONS,
	GLE_TOOL_DVIPS_CMD,
	GLE_TOOL_DVIPS_OPTIONS,
	GLE_TOOL_GHOSTSCRIPT_CMD,
	GLE_TOOL_GHOSTSCRIPT_LIB,
	GLE_TOOL_GHOSTSCRIPT_OPTIONS,
	GLE_TOOL_TEXT_EDITOR,
	GLE_TOOL_PDF_VIEWER
} GLEConfigTool;

#define GLE_TEX_SYSTEM  0
#define GLE_TEX_SYSTEM_LATEX  0
#define GLE_TEX_SYSTEM_VTEX   1

class GLEGlobalConfig {
protected:
	CmdLineObj* m_CmdLine;
	ConfigCollection* m_Config;
	bool m_AllowConfigBlocks;
	vector<string> m_AllowReadDirs;
	vector<string> m_AllowWriteDirs;
public:
	GLEGlobalConfig();
	~GLEGlobalConfig();
	void initCmdLine();
	inline CmdLineObj* getCmdLine() { return m_CmdLine; }
	inline void setCmdLine(CmdLineObj* cmd) { m_CmdLine = cmd; }
	inline ConfigCollection* getRCFile() { return m_Config; }
	inline void setRCFile(ConfigCollection* config) { m_Config = config; }
	inline bool allowConfigBlocks() { return m_AllowConfigBlocks; }
	inline void setAllowConfigBlocks(bool allow) { m_AllowConfigBlocks = allow; }
	inline int getNumberAllowReadDirs() { return m_AllowReadDirs.size(); }
	inline const string& getAllowReadDir(int i) { return m_AllowReadDirs[i]; }
	inline int getNumberAllowWriteDirs() { return m_AllowWriteDirs.size(); }
	inline const string& getAllowWriteDir(int i) { return m_AllowWriteDirs[i]; }
};

bool try_load_config(const string& fname);
void do_find_deps(CmdLineObj& cmdline);
void do_find_deps_sub(GLEInterface* iface, const string& loc);
void do_save_config();
bool do_load_config(const char* appname, char **argv, CmdLineObj& cmdline, ConfigCollection& coll);
void init_config(ConfigCollection* collection);
string get_tool_path(int tool, ConfigSection* tools);

const string& gle_config_margins();
const string& gle_config_papersize();

#endif
