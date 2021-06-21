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

#define GRAPHDEF

#include "all.h"
#include "tokens/stokenizer.h"
#include "cutils.h"
#include "mem_limits.h"
#include "token.h"
#include "core.h"
#include "glearray.h"
#include "polish.h"
#include "pass.h"
#include "var.h"
#include "graph.h"
#include "begin.h"
#include "axis.h"
#include "gprint.h"
#include "key.h"
#include "justify.h"
#include "file_io.h"
#include "keyword.h"
#include "run.h"

#define GLEG_CMD_AXIS     1
#define GLEG_CMD_LABELS   2
#define GLEG_CMD_SIDE     3
#define GLEG_CMD_SUBTICKS 4
#define GLEG_CMD_TICKS    5

#define BAR_SET_COLOR   0
#define BAR_SET_FILL    1
#define BAR_SET_TOP     2
#define BAR_SET_SIDE    3
#define BAR_SET_PATTERN 4
#define BAR_SET_BACKGROUND 5

extern int trace_on;
static int xxgrid[GLE_AXIS_MAX+1];

KeyInfo* g_keyInfo = 0;
GLEGraphBlockData* g_graphBlockData = 0;

vector<GLELet*> g_letCmds;
int g_nbar = 0;
GLERC<GLEColor> g_graph_background;

int freedataset(int i);
void free_temp(void);
void set_sizelength(void);
void draw_graph(KeyInfo* keyinfo, GLEGraphBlockInstance* graphBlock) throw (ParserError);
void do_set_bar_color(const char* tk, bar_struct* bar, int type);
void do_set_bar_style(const char* tk, bar_struct* bar);
void do_axis_part_all(int xset) throw (ParserError);
bool do_remaining_entries(int ct, bool isCommandCheck);
void graph_freebars();
double get_next_exp(TOKENS tk,int ntk,int *curtok);
void data_command(GLESourceLine& sline);
void do_discontinuity();
void do_bar(int& ct, GLEGraphBlockInstance* graphBlock);
void do_fill(int& ct, GLEGraphBlockInstance* graphBlock);
void do_hscale(int& ct);
void do_letsave(GLESourceLine& sline);
void do_size(int& ct);
void do_key(int& ct);
void do_vscale(int& ct);
void do_scale(int& ct);
void do_colormap(int& ct);
void do_main_title(int& ct);
void do_noticks(int& ct);
void do_names(int& ct);
void do_places(int& ct);
void do_title(int& ct);
void do_datasets(int& ct, GLEGraphBlockInstance* graphBlock);
bool is_dataset_identifier(const char* ds);
GLESub* sub_find(const string& s);

GLEAxis xx[GLE_AXIS_MAX+1];
GLEColorMap* g_colormap;

bool check_axis_command_name(const char* name, const char* cmp) {
	int type = axis_type(name);
	if (type != GLE_AXIS_ALL) {
		int len = strlen(name);
		if (len > 2 && name[1] >= '0' && name[1] <= '9') {
			return str_i_equals(name+2, cmp);
		} else if (len > 1) {
			return str_i_equals(name+1, cmp);
		}
	}
	return false;
}

void ensureDataSetCreated(int d) {
	if (dp[d] == NULL) {
		dp[d] = new GLEDataSet(d);
		copy_default(d);
		if (ndata < d) ndata = d;
	}
}

void ensureDataSetCreatedAndSetUsed(int d) {
	ensureDataSetCreated(d);
	dp[d]->axisscale = true;
	g_graphBlockData->getOrder()->addDataSet(d);
}

void replace_exp(string& exp);

fill_data::fill_data():
	layer(0),
	da(0),
	db(0),
	type(0),
	xmin(GLE_INF),
	ymin(GLE_INF),
	xmax(-GLE_INF),
	ymax(-GLE_INF)
{
}

GLEInternalClassDefinitions::GLEInternalClassDefinitions() {
	m_keySeparator = new GLEClassDefinition("key_separator");
	m_keySeparator->addField("lstyle");
	m_drawCommand = new GLEClassDefinition("draw_command");
	m_drawCommand->addField("index");
	m_fill = new GLEClassDefinition("fill");
	m_fill->addField("index");
	m_bar = new GLEClassDefinition("bar");
	m_bar->addField("index");
}

GLEGraphDataSetOrder::GLEGraphDataSetOrder(GLEGraphBlockData* data):
	m_data(data),
	m_order(new GLEArrayImpl())
{
}

void GLEGraphDataSetOrder::addDataSet(int dataSetID) {
	std::set<int>::iterator found(m_isIn.find(dataSetID));
	if (found == m_isIn.end()) {
		m_isIn.insert(dataSetID);
		m_order->addInt(dataSetID);
	}
}

void GLEGraphDataSetOrder::addObject(GLEDataObject* object) {
	m_order->addObject(object);
}

GLEGraphBlockData::GLEGraphBlockData(GLEGraphBlockBase* graphBlockBase):
	m_graphBlockBase(graphBlockBase),
	m_order(new GLEGraphDataSetOrder(this))
{
}

class GLEGraphDrawCommand {
public:
	GLEGraphDrawCommand(int layer);
	virtual ~GLEGraphDrawCommand();

	void createGraphDrawCommand(GLESourceLine& sline);
	void draw();
	int getLayer() const;

private:
	GLESub* m_sub;
	GLEArrayImpl m_arguments;
	int m_layer;
};

GLEGraphDrawCommand::GLEGraphDrawCommand(int layer):
	m_sub(0),
	m_layer(layer)
{
}

GLEGraphDrawCommand::~GLEGraphDrawCommand() {
}

void GLEGraphDrawCommand::createGraphDrawCommand(GLESourceLine& sline) {
	GLEParser* parser = get_global_parser();
	parser->setString(sline.getCodeCStr());
	Tokenizer* tokens = parser->getTokens();
	tokens->ensure_next_token_i("DRAW");
	std::string subName(tokens->next_token());
	str_to_uppercase(subName);
	m_sub = sub_find((char*)subName.c_str());
	if (m_sub != NULL) {
		GLESubCallInfo info(m_sub);
		parser->pass_subroutine_call(&info, tokens->token_pos_col());
		parser->evaluate_subroutine_arguments(&info, &m_arguments);
	} else {
		g_throw_parser_error("function '", subName.c_str(), "' not defined");
	}
}

void GLEGraphDrawCommand::draw() {
	getGLERunInstance()->sub_call(m_sub, &m_arguments);
}

int GLEGraphDrawCommand::getLayer() const {
	return m_layer;
}

GLEGraphPart::GLEGraphPart() {
}

GLEGraphPart::~GLEGraphPart() {
}

void GLEGraphPart::drawLayer(int /* layer */) {
}

void GLEGraphPart::addToOrder(GLEGraphDataSetOrder* /* order */) {
}

void GLEGraphPart::drawLayerObject(int /* layer */, GLEMemoryCell* /* object */) {
}

GLEGraphDrawCommands::GLEGraphDrawCommands() {
}

GLEGraphDrawCommands::~GLEGraphDrawCommands() {
}

std::set<int> GLEGraphDrawCommands::getLayers() {
	std::set<int> result;
	for (int i = 0; i < int(m_drawCommands.size()); ++i) {
		result.insert(m_drawCommands[i]->getLayer());
	}
	return result;
}

void GLEGraphDrawCommands::drawLayerObject(int layer, GLEMemoryCell* object) {
	GLEClassInstance* classObj = getGLEClassInstance(object, g_graphBlockData->getGraphBlockBase()->getClassDefinitions()->getDrawCommand());
	if (classObj != 0) {
		int index = classObj->getArray()->getInt(0);
		if (m_drawCommands[index]->getLayer() == layer) {
			g_gsave();
			g_beginclip();
			g_set_path(true);
			g_newpath();
			g_box_stroke(xbl, ybl, xbl+xlength, ybl+ylength, false);
			g_clip();
			g_set_path(false);
			g_set_hei(g_fontsz);
			m_drawCommands[index]->draw();
			g_endclip();
			g_grestore();
		}
	}
}

void GLEGraphDrawCommands::doDrawCommand(GLESourceLine& sline, GLEGraphBlockInstance* graphBlock)
{
	int index = m_drawCommands.size();
	GLEGraphDrawCommand* cmd = new GLEGraphDrawCommand(graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_DRAW_COMMAND));
	m_drawCommands.push_back(cmd);
	GLEGraphDataSetOrder* order = graphBlock->getData()->getOrder();
	GLEClassDefinition* classDef = graphBlock->getGraphBlockBase()->getClassDefinitions()->getDrawCommand();
	GLEClassInstance* classObj = new GLEClassInstance(classDef);
	order->addObject(classObj);
	classObj->getArray()->addInt(index);
	cmd->createGraphDrawCommand(sline);
}

GLEGraphBlockInstance::GLEGraphBlockInstance(GLEGraphBlockBase* parent):
	GLEBlockInstance(parent),
	m_graphBlockBase(parent),
	m_layer(GLE_GRAPH_LAYER_UNDEFINED),
	m_data(0)
{
	m_drawCommands = new GLEGraphDrawCommands();
	m_axis = new GLEGraphPartAxis();

	m_graphParts.push_back(new GLEGraphPartGrid());
	m_graphParts.push_back(new GLEGraphPartFills());
	m_graphParts.push_back(new GLEGraphPartBars());
	m_graphParts.push_back(m_axis);
	m_graphParts.push_back(new GLEGraphPartLines());
	m_graphParts.push_back(new GLEGraphPartErrorBars());
	m_graphParts.push_back(new GLEGraphPartMarkers());
	m_graphParts.push_back(m_drawCommands);
}

GLEGraphBlockInstance::~GLEGraphBlockInstance() {
}

void GLEGraphBlockInstance::executeLine(GLESourceLine& sline) {
	execute_graph(sline, false, this);
}

void GLEGraphBlockInstance::endExecuteBlock() {
	draw_graph(g_keyInfo, this);
}

void GLEGraphBlockInstance::doDrawCommand(GLESourceLine& sline) {
	m_drawCommands->doDrawCommand(sline, this);
}

int GLEGraphBlockInstance::getLayer() const {
	return m_layer;
}

int GLEGraphBlockInstance::getLayerWithDefault(int defaultLayer) const {
	if (m_layer == GLE_GRAPH_LAYER_UNDEFINED) {
		return defaultLayer;
	} else {
		return m_layer;
	}
}

void GLEGraphBlockInstance::setLayer(int layer) {
	m_layer = layer;
}

void GLEGraphBlockInstance::drawParts() {
	typedef std::set<int> LayerSet;
	LayerSet allLayers;
	GLEVectorAutoDelete<LayerSet> partLayers;
	GLEGraphDataSetOrder* order = getData()->getOrder();
	for (int i = 0; i < int(m_graphParts.size()); ++i) {
		m_graphParts[i]->addToOrder(order);
	}
	for (int i = 0; i < int(m_graphParts.size()); ++i) {
		LayerSet layers(m_graphParts[i]->getLayers());
		allLayers.insert(layers.begin(), layers.end());
		partLayers.push_back(new LayerSet(layers.begin(), layers.end()));
	}
	for (LayerSet::const_iterator layer(allLayers.begin()); layer != allLayers.end(); ++layer) {
		for (int i = 0; i < int(m_graphParts.size()); ++i) {
			if (partLayers[i]->count(*layer) != 0) {
				m_graphParts[i]->drawLayer(*layer);
			}
		}
		GLEArrayImpl* orderArray = order->getArray();
		for (unsigned int j = 0; j < orderArray->size(); ++j) {
			for (int i = 0; i < int(m_graphParts.size()); ++i) {
				if (partLayers[i]->count(*layer) != 0) {
					m_graphParts[i]->drawLayerObject(*layer, orderArray->get(j));
				}
			}
		}
	}
}

GLEGraphPartAxis* GLEGraphBlockInstance::getAxis() {
	return m_axis;
}

void GLEGraphBlockInstance::setData(GLEGraphBlockData* data) {
	m_data = data;
}

GLEGraphBlockData* GLEGraphBlockInstance::getData() {
	return m_data;
}

GLEGraphBlockBase* GLEGraphBlockInstance::getGraphBlockBase() {
	return m_graphBlockBase;
}

GLEGraphBlockBase::GLEGraphBlockBase():
	GLEBlockBase("graph", false),
	m_classDefinitions(new GLEInternalClassDefinitions())
{
}

GLEGraphBlockBase::~GLEGraphBlockBase() {

}

GLEBlockInstance* GLEGraphBlockBase::beginExecuteBlockImpl(GLESourceLine& sline, int *pcode, int *cp) {
	GLEGraphBlockInstance* graphBlock = new GLEGraphBlockInstance(this);
	begin_graph(this, graphBlock);
	return graphBlock;
}

bool GLEGraphBlockBase::checkLine(GLESourceLine& sline) {
	return execute_graph(sline, true, 0);
}

// Font sizes in a graph are based in g_fontsz, but this was set in the original
// GLE to a fixed proportion of the largest dimension of a graph. The size of an
// axis is defined in terms of its base size, which is equal to g_fontsz.
// A useful value for base is 0.25

void begin_graph(GLEGraphBlockBase* graphBlockBase, GLEGraphBlockInstance* graphBlock) throw (ParserError) {
	g_colormap = NULL;
	for (unsigned int i = 0; i < g_letCmds.size(); i++) {
		deleteLet(g_letCmds[i]);
	}
	g_letCmds.clear();
	delete g_keyInfo;
	g_keyInfo = new KeyInfo();
	delete g_graphBlockData;
	g_graphBlockData = new GLEGraphBlockData(graphBlockBase);
	graphBlock->setData(g_graphBlockData);
	g_hscale = .7;
	g_vscale = .7;
	g_discontinuityThreshold = GLE_INF;
	if (g_get_compatibility() == GLE_COMPAT_35) {
		g_nobox = false;
	} else {
		g_nobox = true;
	}
	g_center = false;
	g_auto_s_h = false;
	g_auto_s_v = false;
	g_math = false;
	for (int i = 1; i <= GLE_AXIS_MAX; i++) {
		xxgrid[i] = 0;
		vinit_axis(i);
	}
	graph_init();
	g_get_usersize(&g_xsize,&g_ysize);
	g_get_hei(&g_fontsz);
	set_sizelength();
	dp[0] = new GLEDataSet(0);  /* dataset for default settings */
}

bool execute_graph(GLESourceLine& sline, bool isCommandCheck, GLEGraphBlockInstance* graphBlock) {
	begin_init();
	int st = begin_token(sline, srclin, tk, &ntk, outbuff, !isCommandCheck);
	if (!st) {
		return false;
	}
	int ct = 1;
	if (str_i_equals(tk[ct],"BAR")) {
		if (isCommandCheck) return true;
		do_bar(ct, graphBlock);
	} else if (str_i_equals(tk[ct],"DATA")) {
		if (isCommandCheck) return true;
		data_command(sline);
	} else kw("FILL") {
		if (isCommandCheck) return true;
		do_fill(ct, graphBlock);
	} else kw("HSCALE") {
		if (isCommandCheck) return true;
		do_hscale(ct);
	} else kw("LET") {
		if (isCommandCheck) return true;
		do_letsave(sline);
	} else kw("SIZE") {
		if (isCommandCheck) return true;
		do_size(ct);
	} else kw("KEY") {
		if (isCommandCheck) return true;
		do_key(ct);
	} else kw("VSCALE") {
		if (isCommandCheck) return true;
		do_vscale(ct);
	} else kw("SCALE") {
		if (isCommandCheck) return true;
		do_scale(ct);
	} else kw("COLORMAP") {
		if (isCommandCheck) return true;
		do_colormap(ct);
	} else kw("TITLE") {
		if (isCommandCheck) return true;
		do_main_title(ct);
	} else kw("DISCONTINUITY") {
		if (isCommandCheck) return true;
		do_discontinuity();
	} else kw("BACKGROUND") {
		if (isCommandCheck) return true;
		g_graph_background = next_color;
	} else kw("BEGIN") {
		ct++;
		kw("LAYER") {
			if (isCommandCheck) return true;
			graphBlock->setLayer(int(floor(next_exp + 0.5)));
		}
	} else kw("END") {
		ct++;
		kw("LAYER") {
			if (isCommandCheck) return true;
			graphBlock->setLayer(GLE_GRAPH_LAYER_UNDEFINED);

		}
	} else if (check_axis_command_name(tk[ct],"NOTICKS")) {
		if (isCommandCheck) return true;
		do_noticks(ct);
	} else if (str_i_str(tk[ct],"AXIS") != NULL) {
		if (isCommandCheck) return true;
		do_axis_part_all(GLEG_CMD_AXIS);
	} else if (str_i_str(tk[ct],"LABELS") != NULL) {
		if (isCommandCheck) return true;
		do_axis_part_all(GLEG_CMD_LABELS);
	} else if (str_i_str(tk[ct],"SIDE") != NULL) {
		if (isCommandCheck) return true;
		do_axis_part_all(GLEG_CMD_SIDE);
	} else if (str_i_str(tk[ct],"SUBTICKS") != NULL) {
		if (isCommandCheck) return true;
		do_axis_part_all(GLEG_CMD_SUBTICKS);
	} else if (str_i_str(tk[ct],"TICKS") != NULL) {
		if (isCommandCheck) return true;
		do_axis_part_all(GLEG_CMD_TICKS);
	} else if (str_i_str(tk[ct],"DRAW") != NULL) {
		if (isCommandCheck) return true;
		graphBlock->doDrawCommand(sline);
	} else if (check_axis_command_name(tk[ct],"NAMES")) {
		if (isCommandCheck) return true;
		do_names(ct);
	} else if (check_axis_command_name(tk[ct],"PLACES")) {
		if (isCommandCheck) return true;
		do_places(ct);
	} else if (check_axis_command_name(tk[ct],"TITLE")) {
		if (isCommandCheck) return true;
		do_title(ct);
	} else if (is_dataset_identifier(tk[ct])) {
		if (isCommandCheck) return true;
		do_datasets(ct, graphBlock);
	} else if (do_remaining_entries(ct, isCommandCheck)) {
		if (isCommandCheck) return true;
	}
	return false;
}

void do_bar(int& ct, GLEGraphBlockInstance* graphBlock) {
	/* bar d1,d2,d3 from d4,d5,d6 color red,green,blue fill grey,blue,green
		dist exp, width exp, LSTYLE 3,445,1
		3dbar .5 .5 top black,black,black side red,green,blue  notop */
	int ng = 0,fi;
	char *ss;
	if (g_nbar + 1 >= MAX_NB_BAR) {
		g_throw_parser_error("too many bar commands in graph block");
	}
	g_nbar++;
	br[g_nbar] = new bar_struct();
	br[g_nbar]->ngrp = 0;
	GLEGraphDataSetOrder* order = graphBlock->getData()->getOrder();
	GLEClassDefinition* classDef = graphBlock->getGraphBlockBase()->getClassDefinitions()->getBar();
	GLEClassInstance* classObj = new GLEClassInstance(classDef);
	order->addObject(classObj);
	classObj->getArray()->addInt(g_nbar);
	br[g_nbar]->layer = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_FILL);
	ct = 2;
	ss = strtok(tk[ct],",");
	while (ss!=NULL) {
	  if (toupper(*ss)=='D') {
		ng = (br[g_nbar]->ngrp)++;
		int d = get_dataset_identifier(ss);
		ensureDataSetCreatedAndSetUsed(d);
		br[g_nbar]->to[ng] = d;
 	  }
	  ss = strtok(0,",");
	}
	br[g_nbar]->horiz = false;
	for (int i = 0; i <= ng; i++) {
		br[g_nbar]->color[i] = new GLEColor(0.0);
		br[g_nbar]->fill[i] = new GLEColor(i == 0 ? 0.0 : 1.0 - ng/i);
		br[g_nbar]->from[i] = 0;
		g_get_line_width(&br[g_nbar]->lwidth[i]);
		strcpy(br[g_nbar]->lstyle[i] ,"1\0");
	}
	ct++;
	while (ct<=ntk) {
		kw("DIST") {
			br[g_nbar]->dist = next_exp;
		} else kw("WIDTH") {
			br[g_nbar]->width = next_exp;
		} else kw("3D") {
			br[g_nbar]->x3d = next_exp;
			br[g_nbar]->y3d = next_exp;
		} else kw("NOTOP") {
			br[g_nbar]->notop = true;
		} else kw("HORIZ") {
			br[g_nbar]->horiz = true;
		} else kw("LSTYLE") {
			next_str((char *) br[g_nbar]->lstyle);
		} else kw("STYLE") {
			ct += 1;
			do_set_bar_style(tk[ct], br[g_nbar]);
		} else kw("LWIDTH") {
			br[g_nbar]->lwidth[0] = next_exp;
		} else kw("FROM") {
			fi = 0;
			ct +=1;
			ss = strtok(tk[ct],",");
			while (ss!=NULL) {
				if (toupper(*ss)=='D') {
					int di = get_dataset_identifier(ss);
					ensureDataSetCreatedAndSetUsed(di);
					br[g_nbar]->from[fi++] = di;
				}
				ss = strtok(0,",");
			}
	   } else kw("COLOR") {
		   ct += 1;
		   do_set_bar_color(tk[ct], br[g_nbar], BAR_SET_COLOR);
	   } else kw("SIDE") {
		   ct += 1;
		   do_set_bar_color(tk[ct], br[g_nbar], BAR_SET_SIDE);
	   } else kw("TOP") {
		   ct += 1;
		   do_set_bar_color(tk[ct], br[g_nbar], BAR_SET_TOP);
	   } else kw("FILL") {
		   ct++;
		   do_set_bar_color(tk[ct], br[g_nbar], BAR_SET_FILL);
	   } else kw("PATTERN") {
		   ct++;
		   do_set_bar_color(tk[ct], br[g_nbar], BAR_SET_PATTERN);
	   } else kw("BACKGROUND") {
		   ct++;
		   do_set_bar_color(tk[ct], br[g_nbar], BAR_SET_BACKGROUND);
	   } else {
		   g_throw_parser_error("unrecognised bar sub command '", tk[ct], "'");
	   }
	   ct++;
	}
}

/* fill x1,d2 color green xmin 1 xmax 2 ymin 1 ymax 2   */
/* fill d1,x2 color green xmin 1 xmax 2 ymin 1 ymax 2   */
/* fill d1,d2 color green xmin 1 xmax 2 ymin 1 ymax 2   */
/* fill d1 color green xmin 1 xmax 2 ymin 1 ymax 2      */
void do_fill(int& ct, GLEGraphBlockInstance* graphBlock) {
	if (nfd + 1 >= MAX_NB_FILL) {
		g_throw_parser_error("too many fill commands in graph block");
	}
	fd[++nfd] = new fill_data();
	GLEGraphDataSetOrder* order = graphBlock->getData()->getOrder();
	GLEClassDefinition* classDef = graphBlock->getGraphBlockBase()->getClassDefinitions()->getFill();
	GLEClassInstance* classObj = new GLEClassInstance(classDef);
	order->addObject(classObj);
	classObj->getArray()->addInt(nfd);
	fd[nfd]->layer = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_FILL);
	char s1[40],s2[40];
	ct = 2;
	strcpy(s1,strtok(tk[ct],","));
	char *ss = strtok(0,",");
	if (ss == NULL) {
		strcpy(s2,"");
	} else {
		strcpy(s2,ss);
		strtok(0,",");
	}
	if (str_i_equals(s1,"X1")) {
		fd[nfd]->type = 1;
		int d = get_dataset_identifier(s2);
		fd[nfd]->da = d;
	} else if (str_i_equals(s2,"X2")) {
		fd[nfd]->type = 2;
		int d = get_dataset_identifier(s1);
		fd[nfd]->da = d;
	} else if (!str_i_equals(s2,"")) {
		fd[nfd]->type = 3;
		int d = get_dataset_identifier(s1);
		int d2 = get_dataset_identifier(s2);
		fd[nfd]->da = d;
		fd[nfd]->db = d2;
	} else if (toupper(*s1)=='D') {
		fd[nfd]->type = 4;
		int d = get_dataset_identifier(s1);
		fd[nfd]->da = d;
	} else {
		g_throw_parser_error("invalid fill option, wanted d1,d2 or x1,d1 or d1,x2 or d1");
	}
	if (fd[nfd]->da != 0) {
		ensureDataSetCreatedAndSetUsed(fd[nfd]->da);
	}
	if (fd[nfd]->db != 0) {
		ensureDataSetCreatedAndSetUsed(fd[nfd]->db);
	}
	ct++;
	fd[nfd]->color = new GLEColor(1.0 - nfd * 0.1);
	fd[nfd]->xmin = -GLE_INF;
	fd[nfd]->xmax = GLE_INF;
	fd[nfd]->ymin = -GLE_INF;
	fd[nfd]->ymax = GLE_INF;
	while (ct<=ntk)  {
	   kw("COLOR") {
		   ct++;
		   fd[nfd]->color = pass_color_var(tk[ct]);
	   } else kw("XMIN") {
		   fd[nfd]->xmin = next_exp;
	   } else kw("XMAX") {
		   fd[nfd]->xmax = next_exp;
	   } else kw("YMIN") {
		   fd[nfd]->ymin = next_exp;
	   } else kw("YMAX") {
		   fd[nfd]->ymax = next_exp;
	   } else {
		g_throw_parser_error("unrecognised fill sub command: '", tk[ct], "'");
	   }
	   ct++;
	}
}

void do_key(int& ct) {
	ct = 2;
	while (ct<=ntk)  {
		kw("OFFSET") {
			g_keyInfo->setOffsetX(next_exp);
			g_keyInfo->setOffsetY(next_exp);
		}
		else kw("MARGINS") {
			double mx = next_exp;
			double my = next_exp;
			g_keyInfo->setMarginXY(mx, my);
		}
		else kw("ABSOLUTE") {
			if (g_xsize*g_ysize==0) {
				g_xsize = 10; g_ysize = 10;
				g_get_usersize(&g_xsize,&g_ysize);
			}
			window_set(false);
			store_window_bounds_to_vars();
			set_sizelength();

			g_keyInfo->setOffsetX(next_exp);
			g_keyInfo->setOffsetY(next_exp);
			g_keyInfo->setAbsolute(true);
		}
		else kw("BACKGROUND") g_keyInfo->setBackgroundColor(next_color);
		else kw("BOXCOLOR") g_keyInfo->setBoxColor(next_color);
		else kw("ROW") g_keyInfo->setBase(next_exp);
		else kw("LPOS") g_keyInfo->setLinePos(next_exp);
		else kw("LLEN") g_keyInfo->setLineLen(next_exp);
		else kw("NOBOX") g_keyInfo->setNoBox(true);
		else kw("NOLINE") g_keyInfo->setNoLines(true);
		else kw("COMPACT") g_keyInfo->setCompact(true);
		else kw("HEI") g_keyInfo->setHei(next_exp);
		else kw("POSITION") next_str(g_keyInfo->getJustify());
		else kw("POS") next_str(g_keyInfo->getJustify());
		else kw("JUSTIFY") {
			next_str(g_keyInfo->getJustify());
			g_keyInfo->setPosOrJust(false);
		}
		else kw("JUST") {
			next_str(g_keyInfo->getJustify());
			g_keyInfo->setPosOrJust(false);
		}
		else kw("DIST") g_keyInfo->setDist(next_exp);
		else kw("COLDIST") g_keyInfo->setColDist(next_exp);
		else kw("OFF") g_keyInfo->setDisabled(true);
		else kw("SEPARATOR") {
			GLEClassDefinition* keySeparatorDefinition = g_graphBlockData->getGraphBlockBase()->getClassDefinitions()->getKeySeparator();
			GLEClassInstance* keySeparator = new GLEClassInstance(keySeparatorDefinition);
			g_graphBlockData->getOrder()->addObject(keySeparator);
			ct++;
			kw("LSTYLE") {
				keySeparator->getArray()->addInt((int)floor(next_exp + 0.5));
			}
		}
		else g_throw_parser_error("unrecognised KEY sub command: '",tk[ct],"'");
		ct++;
	}
}

void do_hscale(int& ct) {
	if (str_i_equals(tk[ct+1], "AUTO")) g_auto_s_h = true;
	else g_hscale = next_exp;
}

void do_letsave(GLESourceLine& sline) {
	GLELet* let = parseLet(sline);
	g_letCmds.push_back(let);
}

void do_size(int& ct) {
	g_xsize = next_exp;
	g_ysize = next_exp;
	/* ! set up some more constants */
	set_sizelength();
	do_remaining_entries(ct+1, false);
}

void do_scale(int& ct) {
	if (str_i_equals(tk[ct+1], "AUTO")) {
		g_auto_s_v = true;
		g_auto_s_h = true;
		ct++;
	} else {
		g_hscale = next_exp;
		g_vscale = next_exp;
	}
	do_remaining_entries(ct+1, false);
}

void do_vscale(int& ct) {
	if (str_i_equals(tk[ct+1], "AUTO")) g_auto_s_v = true;
	else g_vscale = next_exp;
}

void do_colormap(int& ct) {
	g_colormap = new GLEColorMap();
	g_colormap->setFunction(tk[++ct]);
	g_colormap->setWidth((int)floor(next_exp+0.5));
	g_colormap->setHeight((int)floor(next_exp+0.5));
	ct++;
	while (ct <= ntk) {
		kw("COLOR") g_colormap->setColor(true);
		else kw("INVERT") g_colormap->setInvert(true);
		else kw("ZMIN") g_colormap->setZMin(next_exp);
		else kw("ZMAX") g_colormap->setZMax(next_exp);
		else kw("INTERPOLATE") {
			string tmp;
			next_str_cpp(tmp);
			IpolType ipolType(IPOL_TYPE_BICUBIC);
			if (str_i_equals(tmp, "BICUBIC")) {
				ipolType = IPOL_TYPE_BICUBIC;
			} else if (str_i_equals(tmp, "NEAREST")) {
				ipolType = IPOL_TYPE_NEAREST;
			} else {
				g_throw_parser_error("unknown interpolation type '", tmp.c_str(), "'");
			}
			g_colormap->setIpolType(ipolType);
		}
		else kw("PALETTE") {
			string tmp;
			next_str_cpp(tmp);
			str_to_uppercase(tmp);
			g_colormap->setPalette(tmp);
		}
		else g_throw_parser_error("expecting colormap sub command, not '", tk[ct], "'");
		ct++;
	}
	g_colormap->readData();
}

void do_names(int& ct) {
	int t = axis_type_check(tk[1]);
	xx[t].label_off = false;
	if (ntk >= 3 && str_i_equals(tk[2], "FROM") && toupper(tk[3][0]) == 'D') {
		// Support syntax "xnames from d1" to retrieve names from the data set
		xx[t].setNamesDataSet(get_dataset_identifier(tk[3]));
	} else {
		ct = 1;
		while (ct<ntk)  {
			next_quote(strbuf);
			xx[t].addName(strbuf);
		}
	}
}

void do_places(int& ct) {
	int t = axis_type_check(tk[1]);
	xx[t].label_off = false;
	ct = 1;
	while (ct<ntk)  {
		xx[t].addPlace(next_exp);
	}
}

void do_noticks(int& ct) {
	int t = axis_type_check(tk[1]);
	ct = 1;
	xx[t].clearNoTicks();
	if (t <= 2) xx[t+2].clearNoTicks();
	while (ct<ntk)  {
		double pos = next_exp;
		xx[t].addNoTick(pos);
		if (t <= 2) xx[t+2].addNoTick(pos);
	}
}

void do_main_title(int& ct) {
	/* should change position and size of main title */
	int t = GLE_AXIS_T;
	xx[GLE_AXIS_T].off = 0;
	ct = 1;
	next_vquote_cpp(xx[t].title);
	ct = 3;
	xx[t].title_dist = g_fontsz*.7;
	xx[t].title_hei = g_fontsz*g_get_fconst(GLEC_TITLESCALE);
	while (ct<=ntk)  {
	         kw("HEI")     xx[t].title_hei = next_exp;
	    else kw("OFF")     xx[t].title_off = true;
	    else kw("COLOR")   xx[t].title_color = next_color;
	    else kw("FONT")    xx[t].title_font = next_font;
	    else kw("DIST")    xx[t].title_dist = next_exp;
	    else g_throw_parser_error("expecting title sub command, not '", tk[ct], "'");
	    ct++;
	}
}

void do_title(int& ct) {
	int t = axis_type_check(tk[1]);
	ct = 1;
	next_vquote_cpp(xx[t].title);
	ct = 3;
	while (ct<=ntk)  {
	         kw("HEI")     xx[t].title_hei = next_exp;
	    else kw("OFF")     xx[t].title_off = true;
	    else kw("ROT")     xx[t].title_rot = true;
	    else kw("ROTATE")  xx[t].title_rot = true;
	    else kw("COLOR")   xx[t].title_color = next_color;
	    else kw("FONT")    xx[t].title_font = next_font;
	    else kw("DIST")    xx[t].title_dist = next_exp;
	    else kw("ADIST")   xx[t].title_adist = next_exp;
	    else kw("ALIGN") {
		string base;
		next_str_cpp(base);
		xx[t].setAlignBase(str_i_equals(base, "BASE"));
	    } else {
		g_throw_parser_error("expecting title sub command, not '", tk[ct], "'");
	    }
	    ct++;
	}
}

void do_datasets(int& ct, GLEGraphBlockInstance* graphBlock) {
	int d = get_dataset_identifier(tk[1]); /* dataset number (right$(k$,2))  (0=dn) */
	if (d != 0) {
		ensureDataSetCreatedAndSetUsed(d);
		do_dataset(d, graphBlock);
	} else {
		for (d = 0; d < MAX_NB_DATA; d++) {
			if (dp[d] != NULL) {
				do_dataset(d, graphBlock);
			}
		}
	}
}

bool do_remaining_entries(int ct, bool isCommandCheck) {
	int nb_found = 0;
	bool found = true;
	while (found && ct <= ntk) {
		kw("NOBOX") {
			if (isCommandCheck) return true;
			g_nobox = true;
		} else kw("BOX") {
			if (isCommandCheck) return true;
			g_nobox = false;
		} else kw("NOBORDER") {
			if (isCommandCheck) return true;
			g_nobox = true; // for compatibility with v3.5
		} else kw("BORDER") {
			if (isCommandCheck) return true;
			g_nobox = false;
		} else kw("CENTER") {
			if (isCommandCheck) return true;
			g_center = true;
		} else kw("FULLSIZE") {
			if (isCommandCheck) return true;
			g_vscale = 1;
			g_hscale = 1;
			g_nobox = true;
		} else kw("MATH") {
			if (isCommandCheck) return true;
			g_math = true;
			xx[GLE_AXIS_Y].offset = 0.0;
			xx[GLE_AXIS_Y].has_offset = true;
			xx[GLE_AXIS_Y].ticks_both = true;
			xx[GLE_AXIS_X].offset = 0.0;
			xx[GLE_AXIS_X].has_offset = true;
			xx[GLE_AXIS_X].ticks_both = true;
			xx[GLE_AXIS_X2].off = true;
			xx[GLE_AXIS_Y2].off = true;
		} else {
			found = false;
		}
		if (found) {
			ct++;
			nb_found++;
		}
	}
	return nb_found > 0;
}

void do_axis(int axis, bool craxis) throw (ParserError) {
	int ct = 2;
	while (ct <= ntk)  {
		kw("BASE") xx[axis].base = next_exp;
		else kw("COLOR") xx[axis].setColor(next_color);
		else kw("DSUBTICKS") xx[axis].dsubticks = next_exp;
		else kw("DTICKS") {
			xx[axis].dticks = next_exp;
			if (craxis) xx[axis].label_off = false;
		} else kw("FTICK") {
			xx[axis].ftick = next_exp;
			xx[axis].has_ftick = true;
		}
		else kw("SYMTICKS") xx[axis].ticks_both = get_on_off(tk, &ct);
		else kw("SHIFT") xx[axis].shift = next_exp;
		else kw("ANGLE") xx[axis].label_angle = next_exp;
		else kw("GRID") {
			xxgrid[axis] = true;
			if (str_i_equals(tk[ct+1],"ONTOP")) {
				xx[axis].setGridOnTop(true);
				ct++;
			}
		}
		else kw("NEGATE") {
			xx[axis].negate = true;
		}
		else kw("FONT") xx[axis].label_font = next_font;
		else kw("LOG") xx[axis].log = true;  /* don't make log depend on craxis, otherwise grid may not line up */
		else kw("LIN") xx[axis].log = false;
		else kw("LSTYLE") next_str(xx[axis].side_lstyle);
		else kw("LWIDTH") xx[axis].side_lwidth = next_exp;
		else kw("MIN") {
			double value = next_exp;
			if (craxis) xx[axis].getRange()->setMinSet(value);
		}
		else kw("MAX") {
			double value = next_exp;
			if (craxis) xx[axis].getRange()->setMaxSet(value);
		}
		else kw("OFFSET") {
			double offs = next_exp;
			if (craxis) {
				xx[axis].offset = offs;
				xx[axis].has_offset = true;
				if (!g_math) {
					if (axis == GLE_AXIS_X) xx[GLE_AXIS_X0].off = false;
					if (axis == GLE_AXIS_Y) xx[GLE_AXIS_Y0].off = false;
				}
			}
		}
		else kw("ROUNDRANGE") xx[axis].roundRange = get_on_off(tk, &ct);
		else kw("HEI") 	xx[axis].label_hei = next_exp;
		else kw("NOLAST") xx[axis].nolast = true;
		else kw("LAST") xx[axis].nolast = !get_on_off(tk, &ct);
		else kw("FIRST") xx[axis].nofirst = !get_on_off(tk, &ct);
		else kw("NOFIRST") xx[axis].nofirst = true;
		else kw("NSUBTICKS") xx[axis].nsubticks = (int) next_exp;
		else kw("NTICKS") {
			xx[axis].nticks = (int) next_exp;
			if (craxis) xx[axis].label_off = false;
		}
		else kw("ON") xx[axis].off = false;
		else kw("OFF") xx[axis].off = true;
		else kw("FORMAT") next_str_cpp(xx[axis].format);
		else kw("SCALE") {
			if (str_i_equals(tk[ct+1],"QUANTILE")) {
				ct++;
				GLERC<GLEAxisQuantileScale> q_scale = new GLEAxisQuantileScale();
				while (true) {
					if (str_i_equals(tk[ct+1], "LOWER")) {
						ct++;
						q_scale->setLowerQuantile(next_exp);
					} else if (str_i_equals(tk[ct+1], "UPPER")) {
						ct++;
						q_scale->setUpperQuantile(next_exp);
					} else if (str_i_equals(tk[ct+1], "FACTOR")) {
						ct++;
						double factor = next_exp;
						q_scale->setLowerQuantileFactor(factor);
						q_scale->setUpperQuantileFactor(factor);
					} else if (str_i_equals(tk[ct+1], "LFACTOR")) {
						ct++;
						q_scale->setLowerQuantileFactor(next_exp);
					} else if (str_i_equals(tk[ct+1], "UFACTOR")) {
						ct++;
						q_scale->setUpperQuantileFactor(next_exp);
					} else {
						break;
					}
				}
				xx[axis].setQuantileScale(q_scale.get());
			}
		}
		else {
			g_throw_parser_error("expecting axis sub command, found '", tk[ct], "'");
		}
		ct++;
	}
}

void do_labels(int axis, bool showerr) throw (ParserError) {
	int ct = 2;
	while (ct <= ntk)  {
		if (*tk[ct]==' ') ct++;
		kw("HEI") xx[axis].label_hei = next_exp;
		else kw("OFF") {
			if (showerr) {
				xx[axis].label_off = true;
				xx[axis].has_label_onoff = true;
			}
		}
		else kw("ON") {
			if (showerr) {
				xx[axis].label_off = false;
				xx[axis].has_label_onoff = true;
				xx[axis].off = false;
			}
		}
		else kw("COLOR") xx[axis].label_color = next_color;
		else kw("FONT") xx[axis].label_font = next_font;
		else kw("DIST") xx[axis].label_dist = next_exp;
		else kw("ALIGN") {
			ct++;
			kw("LEFT") xx[axis].label_align = JUST_LEFT;
			else kw("RIGHT") xx[axis].label_align = JUST_RIGHT;
		}
		else kw("LOG") {
			ct++;
			kw("OFF") xx[axis].lgset = GLE_AXIS_LOG_OFF;
			else kw("L25B") xx[axis].lgset = GLE_AXIS_LOG_25B;
			else kw("L25") xx[axis].lgset = GLE_AXIS_LOG_25;
			else kw("L1") xx[axis].lgset = GLE_AXIS_LOG_1;
			else kw("N1") xx[axis].lgset = GLE_AXIS_LOG_N1;
			else if (showerr) g_throw_parser_error("Expecting OFF, L25, L25B, L1, or N1, found '", tk[ct], "'");
		}
		else if (showerr) {
			g_throw_parser_error("Expecting LABELS sub command, found '", tk[ct], "'");
		}
		ct++;
	}
}

void do_side(int axis, bool showerr) throw (ParserError) {
	int ct = 2;
	while (ct <= ntk)  {
		if (*tk[ct]==' ') ct++;
		kw("OFF") {
			if (showerr) xx[axis].side_off = true;
		}
		else kw("ON") {
			if (showerr) xx[axis].side_off = false;
		}
		else kw("COLOR") xx[axis].side_color = next_color;
		else kw("LWIDTH") xx[axis].side_lwidth = next_exp;
		else kw("LSTYLE") next_str(xx[axis].side_lstyle);
		else if (showerr) {
			g_throw_parser_error("Expecting SIDE sub command, found '", tk[ct], "'");
		}
		ct++;
	}
}

void do_ticks(int axis, bool showerr) throw (ParserError) {
	int ct = 2;
	while (ct <= ntk)  {
		if (*tk[ct]==' ') ct++;
		kw("LENGTH") xx[axis].ticks_length = next_exp;
		else kw("OFF") {
			if (showerr) {
				xx[axis].ticks_off = true;
				xx[axis].subticks_off = true;
			}
		} else kw("ON") {
			if (showerr) {
				xx[axis].ticks_off = false;
				xx[axis].subticks_off = false;
			}
		} else kw("COLOR") {
			xx[axis].ticks_color = next_color;
			xx[axis].subticks_color = xx[axis].ticks_color;
		} else kw("LWIDTH") {
			xx[axis].ticks_lwidth = next_exp;
		} else kw("LSTYLE") {
			next_str(xx[axis].ticks_lstyle);
		} else if (showerr) {
			g_throw_parser_error("Expecting TICKS sub command, found '", tk[ct], "'");
		}
		ct++;
	}
}

void do_subticks(int axis, bool showerr) throw (ParserError) {
	int ct = 2;
	while (ct <= ntk)  {
		if (*tk[ct]==' ') ct++;
		kw("LENGTH") xx[axis].subticks_length = next_exp;
		else kw("OFF") {
			if (showerr) {
				xx[axis].subticks_off = true;
				xx[axis].has_subticks_onoff = true;
			}
		}
		else kw("ON") {
			if (showerr) {
				xx[axis].subticks_off = false;
				xx[axis].has_subticks_onoff = true;
			}
		}
		else kw("COLOR") xx[axis].subticks_color = next_color;
		else kw("LWIDTH") xx[axis].subticks_lwidth = next_exp;
		else kw("LSTYLE") next_str(xx[axis].subticks_lstyle);
		else {
			g_throw_parser_error("Expecting SUBTICKS sub command, found '", tk[ct], "'");
		}
		ct++;
	}
}

void do_axis_part(int axis, bool craxis, int xset) throw (ParserError) {
	// craxis = command is for current axis
	// showerr = passing options for axis command to labels/side/ticks commands
	switch (xset) {
	  case GLEG_CMD_AXIS:
		do_axis(axis, craxis);
		do_labels(axis, false);
		do_side(axis, false);
		do_ticks(axis, false);
		break;
	  case GLEG_CMD_LABELS:
		do_labels(axis, true);
		break;
	  case GLEG_CMD_SIDE:
		do_side(axis, true);
		break;
	  case GLEG_CMD_TICKS:
		do_ticks(axis, true);
		break;
	  case GLEG_CMD_SUBTICKS:
		do_subticks(axis, true);
		break;
	}
}

void do_axis_part_all(int xset) throw (ParserError) {
	int axis = axis_type(tk[1]);
	if (axis == GLE_AXIS_ALL) {
		do_axis_part(GLE_AXIS_X, false, xset);
		do_axis_part(GLE_AXIS_X0, false, xset);
		do_axis_part(GLE_AXIS_X2, false, xset);
		do_axis_part(GLE_AXIS_Y, false, xset);
		do_axis_part(GLE_AXIS_Y0, false, xset);
		do_axis_part(GLE_AXIS_Y2, false, xset);
	} else {
		do_axis_part(axis, true, xset);
	}
	if (axis == GLE_AXIS_X) {
		do_axis_part(GLE_AXIS_X2, false, xset);
		do_axis_part(GLE_AXIS_X0, false, xset);
		do_axis_part(GLE_AXIS_T, false, xset);
	}
	if (axis == GLE_AXIS_Y) {
		do_axis_part(GLE_AXIS_Y2, false, xset);
		do_axis_part(GLE_AXIS_Y0, false, xset);
	}
}

void do_discontinuity() {
	int ct = 2;
	while (ct <= ntk)  {
		kw("THRESHOLD") {
			g_discontinuityThreshold = next_exp;
		} else {
			g_throw_parser_error("Expecting discontinuity option, but found '", tk[ct], "'");
		}
		ct++;
	}
}

void ensure_fill_created(bar_struct* bar, int fi) {
	if (bar->fill[fi].isNull()) {
		bar->fill[fi] = new GLEColor();
		bar->fill[fi]->setTransparent(true);
	}
}

void update_key_fill(bar_struct* bar, int fi) {
	int dataset = bar->to[fi];
	if (dp[dataset] != NULL) {
		dp[dataset]->key_fill = bar->fill[fi];
	}
}

void do_set_bar_color(const char* tk, bar_struct* bar, int type) {
	/* Use tokenizer that jumps over () pairs to be able to parse CVTRGB(...) */
	int fi = 0;
	string tokstr = (const char*)tk;
	level_char_separator separator(",", "", "(", ")");
	tokenizer<level_char_separator> tokens(tokstr, separator);
	while (tokens.has_more()) {
		GLERC<GLEColor> color(pass_color_var(tokens.next_token().c_str()));
		switch (type) {
			case BAR_SET_COLOR:
				bar->color[fi] = color;
				break;
			case BAR_SET_FILL:
				ensure_fill_created(bar, fi);
				update_color_foreground_and_pattern(bar->fill[fi].get(), color.get());
				update_key_fill(bar, fi);
				break;
			case BAR_SET_TOP:
				bar->top[fi] = color;
				break;
			case BAR_SET_SIDE:
				bar->side[fi] = color;
				break;
			case BAR_SET_PATTERN:
				if (color->isFill() && color->getFill()->getFillType() == GLE_FILL_TYPE_PATTERN) {
					ensure_fill_created(bar, fi);
					update_color_fill_pattern(bar->fill[fi].get(), static_cast<GLEPatternFill*>(color->getFill()));
					update_key_fill(bar, fi);
				} else {
					g_throw_parser_error("expected fill pattern");
				}
				break;
			case BAR_SET_BACKGROUND:
				ensure_fill_created(bar, fi);
				update_color_fill_background(bar->fill[fi].get(), color.get());
				update_key_fill(bar, fi);
				break;
		}
		fi++;
	}
}

void do_set_bar_style(const char* tk, bar_struct* bar) {
	int fi = 0;
	string tokstr = (const char*)tk;
	level_char_separator separator(",", "", "(", ")");
	tokenizer<level_char_separator> tokens(tokstr, separator);
	while (tokens.has_more()) {
		pass_file_name(tokens.next_token().c_str(), bar->style[fi]);
		str_to_uppercase(bar->style[fi]);
		fi++;
	}
}

void set_sizelength() {
	double ox, oy;

	/* get the origin x,y */
	g_get_xy(&ox,&oy);
	if (g_hscale==0) g_hscale = .7;
	if (g_vscale==0) g_vscale = .7;

	xbl = ox + (g_xsize/2)-(g_xsize*g_hscale/2);
	ybl = oy + (g_ysize/2)-(g_ysize*g_vscale/2);
	xlength = g_xsize*g_hscale;
	ylength = g_ysize*g_vscale;

	// In more recent versions, the size of a graph font depends on the hei parameter
	if (g_get_compatibility() == GLE_COMPAT_35) {
		if (xlength<ylength) g_fontsz = xlength/23;
		else g_fontsz = ylength/23;
	}

	/* set graph globals */
	graph_x1 = xbl;
	graph_y1 = ybl;
	graph_x2 = xbl + xlength;
	graph_y2 = ybl + ylength;
	graph_xmin = xx[GLE_AXIS_X].getMin();
	graph_xmax = xx[GLE_AXIS_X].getMax();
	graph_ymin = xx[GLE_AXIS_Y].getMin();
	graph_ymax = xx[GLE_AXIS_Y].getMax();
}

void axis_init_length() {
	for (int i = 1; i <= GLE_AXIS_MAX; i++) {
		xx[i].type = i;
		if (xx[i].base==0) xx[i].base = g_fontsz;
		if (axis_horizontal(i)) xx[i].length = xlength;
		else xx[i].length = ylength;
	}
}

void axis_add_grid() {
	for (int i = 1; i <= 2; i++) {
		if (xxgrid[i]) {
			double len = axis_horizontal(i) ? ylength : xlength;
			if (!xx[i].hasGridOnTop()) xx[i].setGrid(true);
			xx[i].ticks_length = len;
			xx[i+2].ticks_off = true;
			if (xx[i].subticks_length == 0.0) {
				xx[i].subticks_length = len;
				xx[i+2].subticks_off = true;
			}
			if (!xx[i].has_subticks_onoff) {
				/* only change default behaviour if no explicit setting given */
				if (xx[i].log) {
					xx[i].subticks_off = false;
				} else {
					xx[i].subticks_off = true;
				}
			}
		}
	}
}

void axis_add_noticks() {
	// Disable ticks and places on some axis
	for (int i = 1; i < GLE_AXIS_MAX; i++) {
		if (!xx[i].off) {
			if (xx[i].has_offset) {
				// axis has offset:
				// - disable ticks and labels at each intersection
				for (int j = 0; j < 3; j++) {
					int orth = axis_get_orth(i, j);
					if (!xx[orth].off) {
						if (xx[orth].has_offset) {
							xx[i].insertNoTickOrLabel(xx[orth].offset);
						} else {
							if (axis_is_max(orth)) xx[i].insertNoTickOrLabel(xx[i].getMax());
							else xx[i].insertNoTickOrLabel(xx[i].getMin());
						}
					}
				}
			} else {
				// axis has no offset:
				// - disable inside ticks (ticks1) for each intersection
				for (int j = 0; j < 3; j++) {
					int orth = axis_get_orth(i, j);
					if (!xx[orth].off) {
						if (xx[orth].has_offset) {
							xx[i].insertNoTick1(xx[orth].offset);
						} else {
							if (axis_is_max(orth)) xx[i].insertNoTick1(xx[i].getMax());
							else xx[i].insertNoTick1(xx[i].getMin());
						}
					}
				}
			}
		}
	}
#ifdef AXIS_DEBUG
	cout << endl;
	for (int i = 1; i <= GLE_AXIS_MAX; i++) {
		cout << "AXIS: " << i << endl;
		xx[i].printNoTicks();
		cout << endl;
	}
#endif
}

void draw_axis_pos(int axis, double xpos, double ypos, bool xy, DrawAxisPart axisPart, GLERectangle* box) {
	if (xx[axis].has_offset) {
		if (xy) g_move(graph_xgraph(xx[axis].offset), ypos);
		else g_move(xpos, graph_ygraph(xx[axis].offset));
	} else {
		g_move(xpos,ypos);
	}
	draw_axis(&xx[axis], box, axisPart);
}

GLEGraphPartGrid::GLEGraphPartGrid() {
}

GLEGraphPartGrid::~GLEGraphPartGrid() {
}

std::set<int> GLEGraphPartGrid::getLayers() {
	std::set<int> result;
	result.insert(GLE_GRAPH_LAYER_GRID);
	return result;
}

void GLEGraphPartGrid::drawLayerPart(DrawAxisPart axisPart) {
	GLERectangle box;
	box.initRange();
	draw_axis_pos(GLE_AXIS_Y0, xbl, ybl, true, axisPart, &box);
	draw_axis_pos(GLE_AXIS_Y, xbl, ybl, true, axisPart, &box);
	draw_axis_pos(GLE_AXIS_Y2, xbl+xlength, ybl, true, axisPart, &box);
	draw_axis_pos(GLE_AXIS_X, xbl, ybl, false, axisPart, &box);
	draw_axis_pos(GLE_AXIS_X0, xbl, ybl, false, axisPart, &box);
	draw_axis_pos(GLE_AXIS_X2, xbl, ybl+ylength, false, axisPart, &box);
}

void GLEGraphPartGrid::drawLayer(int layer) {
   drawLayerPart(DRAW_AXIS_GRID_SUBTICKS);
   drawLayerPart(DRAW_AXIS_GRID_TICKS);
}

GLEGraphPartAxis::GLEGraphPartAxis():
	m_box(0)
{
}

GLEGraphPartAxis::~GLEGraphPartAxis() {
}

std::set<int> GLEGraphPartAxis::getLayers() {
	std::set<int> result;
	result.insert(GLE_GRAPH_LAYER_AXIS);
	return result;
}

void GLEGraphPartAxis::drawLayer(int layer) {
	g_init_bounds();
	// y-axis should *not* influence position of title!
	draw_axis_pos(GLE_AXIS_Y0, xbl, ybl, true, DRAW_AXIS_ALL, m_box);
	draw_axis_pos(GLE_AXIS_Y, xbl, ybl, true, DRAW_AXIS_ALL, m_box);
	draw_axis_pos(GLE_AXIS_Y2, xbl+xlength, ybl, true, DRAW_AXIS_ALL, m_box);
	GLEMeasureBox measure;
	measure.measureStart();
	draw_axis_pos(GLE_AXIS_X, xbl, ybl, false, DRAW_AXIS_ALL, m_box);
	draw_axis_pos(GLE_AXIS_X0, xbl, ybl, false, DRAW_AXIS_ALL, m_box);
	draw_axis_pos(GLE_AXIS_X2, xbl, ybl+ylength, false, DRAW_AXIS_ALL, m_box);
	g_update_bounds(xbl+xlength/2, ybl+ylength);
	measure.measureEnd();
	draw_axis_pos(GLE_AXIS_T, xbl, measure.getYMax(), true, DRAW_AXIS_ALL, m_box);
	// otherwise it does not work if all axis are disabled!
	g_update_bounds_box(m_box);
}

void GLEGraphPartAxis::setBox(GLERectangle* box) {
	m_box = box;
}

void prepare_graph_key_and_clip(double ox, double oy, KeyInfo* keyinfo) {
	if (!keyinfo->hasHei()) keyinfo->setHei(g_fontsz);
	measure_key(keyinfo);
	if (keyinfo->getNbEntries() > 0 && !keyinfo->isDisabled() && !keyinfo->getNoBox() && keyinfo->getBackgroundColor()->isTransparent()) {
		g_gsave();
		g_beginclip();
		g_set_path(true);
		g_newpath();
		GLERectangle fullFig;
		g_get_userbox_undev(&fullFig);
		g_box_stroke(&fullFig, true);
		g_box_stroke(keyinfo->getRect(), false);
		g_clip();
		g_set_path(false);
	}
}

void key_update_bounds(double ox, double oy, KeyInfo* keyinfo) {
	if (!keyinfo->hasHei()) keyinfo->setHei(g_fontsz);
	measure_key(keyinfo);
	if (keyinfo->getNbEntries() > 0 && !keyinfo->isDisabled()) {
		g_update_bounds(keyinfo->getRect());
	}
}

void draw_graph(KeyInfo* keyinfo, GLEGraphBlockInstance* graphBlock) throw (ParserError) {
	GLERectangle box;
	double ox,oy;

	do_bigfile_compatibility();
	g_get_bounds(&box);

	/* if no size given, take entire drawing */
	if (g_xsize*g_ysize==0) {
		g_xsize = 10; g_ysize = 10;
		g_get_usersize(&g_xsize,&g_ysize);
	}

	/* get scales from data sets */
	do_each_dataset_settings();
	set_bar_axis_places();
	get_dataset_ranges();

	/* this window set is there to initialize ranges based on data */
	/* in case there is no ranges given in let commands */
	window_set(false);

	/* update scale based on let commands if not explicit scale given */
	if (should_autorange_based_on_lets()) {
		/* if min max not set, do_let in approximate way first */
		for (unsigned int i = 0; i < g_letCmds.size(); i++) {
			doLet(g_letCmds[i], false);
		}
		get_dataset_ranges();
		for (int i = 1; i <= ndata; i++) {
			if (dp[i] != NULL) {
				dp[i]->restore();
			}
		}
	} else {
		/* otherwise window_set will round ranges found by previous window_set! */
		reset_axis_ranges();
	}

	window_set(true);
	store_window_bounds_to_vars();

	/* get the origin x,y */
	g_get_xy(&ox,&oy);
	g_gsave();
	set_sizelength();
	g_set_hei(g_fontsz);

	/* draw the box */
	if (!g_nobox) {
		g_box_stroke(ox, oy, ox+g_xsize, oy+g_ysize);
	}

	/* init axis things */
	vinit_title_axis();
	axis_add_noticks();
	axis_init_length();

	/* center? */
	if (g_center || g_auto_s_v || g_auto_s_h) {
		GLERectangle dummy;
		dummy.initRange();
		GLEMeasureBox measure;
		GLEDevice* old_device = g_set_dummy_device();
		measure.measureStart();
		graphBlock->getAxis()->setBox(&dummy);
		graphBlock->getAxis()->drawLayer(GLE_GRAPH_LAYER_UNDEFINED);
		key_update_bounds(ox, oy, keyinfo);
		measure.measureEnd();
		g_restore_device(old_device);
		if (g_auto_s_h) {
			double d1 = measure.getXMin() - ox - g_fontsz/5;
			double d2 = ox + g_xsize - measure.getXMax() - g_fontsz/5;
			double l1 = ox + g_xsize/2 - xlength/2 - measure.getXMin();
			double new_xlen = xlength + d1 + d2;
			g_hscale = new_xlen/g_xsize;
			ox += new_xlen/2 - g_xsize/2 + l1 + g_fontsz/5;
		} else if (g_center) {
			ox += ox+g_xsize/2.0 - measure.getXMid();
		}
		if (g_auto_s_v) {
			double d1 = measure.getYMin() - oy - g_fontsz/5;
			double d2 = oy + g_ysize - measure.getYMax() - g_fontsz/5;
			double l1 = oy + g_ysize/2 - ylength/2 - measure.getYMin();
			double new_ylen = ylength + d1 + d2;
			g_vscale = new_ylen/g_ysize;
			oy += new_ylen/2 - g_ysize/2 + l1 + g_fontsz/5;
		} else if (g_center) {
			oy += oy+g_ysize/2.0 - measure.getYMid();
		}
		g_move(ox,oy);
		set_sizelength();
		axis_init_length();
	}

	/* Measure key */
	g_move(ox,oy);
	prepare_graph_key_and_clip(ox, oy, keyinfo);

	/* must come after auto scale */
	axis_add_grid();

	/* do LETS now */
	for (unsigned int i = 0; i < g_letCmds.size(); i++) {
		doLet(g_letCmds[i], true);
	}

	/* Throw away missing values if NOMISS on datasets */
	gr_thrownomiss();

	/* Draw graph background */
	if (!g_graph_background->isTransparent()) {
		GLERC<GLEColor> old_fill(g_get_fill());
		g_set_fill(g_graph_background);
		g_box_fill(graph_x1, graph_y1, graph_x2, graph_y2);
		g_set_fill(old_fill);
	}

	if (g_colormap != NULL) {
		GLEToGraphView view(&xx[GLE_AXIS_X], &xx[GLE_AXIS_Y]);
		g_colormap->draw(&view, graph_x1, graph_y1, xlength, ylength);
		delete g_colormap;
		g_colormap = NULL;
	}

	graphBlock->getAxis()->setBox(&box);

	graphBlock->drawParts();

	/* Draw the key */
	if (keyinfo->getNbEntries() > 0 && !keyinfo->isDisabled() && !keyinfo->getNoBox() && keyinfo->getBackgroundColor()->isTransparent()) {
		g_endclip();
		g_grestore();
	}

	g_grestore();
	g_init_bounds();
	g_set_bounds(&box);

	draw_key_after_measure(keyinfo);
	g_move(ox,oy);
	return;
}

void g_graph_init() {
	for (int i = 0; i < MAX_NB_FILL; i++) {
		fd[i] = NULL;
	}
	for (int i = 0; i < MAX_NB_DATA; i++) {
		dp[i] = NULL;
	}
}

void graph_free() {
	for (int i = 0; i < MAX_NB_FILL; i++) {
		if (fd[i] != NULL) {
			delete fd[i];
			fd[i] = NULL;
		}
	}
	for (int i = 0; i < MAX_NB_DATA; i++) {
		if (dp[i] != NULL) {
			delete dp[i];
		}
		dp[i] = NULL;
	}
}

void graph_freebars() {
	for (int i = 1; i <= g_nbar; i++) {
		delete br[i];
		br[i] = NULL;
	}
	g_nbar = 0;
}

void graph_init() {
	g_graph_background = g_get_fill_clear();
	ndata = 0; nfd = 0; g_nbar = 0;
	xx[GLE_AXIS_X0].off = true;
	xx[GLE_AXIS_Y0].off = true;
	xx[GLE_AXIS_T].off = true;
	graph_freebars();
	graph_free();
}

void iffree(void *p, const char *s) {
	if (p!=NULL) myfrees(p,s);
}

int freedataset(int d) {
	int i,c=0;
	for (i=1;i<=ndata;i++)
	{
		if (dp[i] == NULL) c++;
		else if (dp[i]->undefined()) c++;
		if (c==d) return i;
	}
	return ndata + d - c;
}

void copy_default(int dn) {
	dp[dn]->copy(dp[0]);
	dp[dn]->key_name = "";
	dp[dn]->axisscale = false;
}

bool is_dataset_identifier(const char* ds) {
	int len = strlen(ds);
	if (len <= 1 || toupper(ds[0]) != 'D') {
		return false;
	}
	if (str_i_starts_with(ds, "d\\expr")) {
		return true;
	}
	if (str_i_equals(ds, "dn")) {
		return true;
	}
	if (len > 3 && ds[1] == '[' && ds[len - 1] == ']') {
		return true;
	}
	char* ptr = NULL;
	int result = strtol(ds+1, &ptr, 10);
	return ptr != NULL && *ptr == 0 && result >= 0;
}

int get_dataset_identifier(const std::string& ds, bool def) throw(ParserError) {
	int len = ds.size();
	if (len <= 1 || toupper(ds[0]) != 'D') {
		g_throw_parser_error("illegal data set identifier '", ds.c_str(), "'");
	}
	if (str_i_equals(ds, "dn")) {
		return 0;
	}
	if (len > 3 && ds[1] == '[' && ds[len - 1] == ']') {
		double id;
		string str(ds.c_str() + 2, len - 3);
		polish_eval((char*)str.c_str(), &id);
		int result = (int)floor(id + 0.5);
		if (result < 0 || result >= MAX_NB_DATA) {
			ostringstream err;
			err << "data set identifier out of range: '" << result << "'";
			g_throw_parser_error(err.str());
		}
		if (def && dp[result] == NULL) {
			g_throw_parser_error("data set '", ds.c_str(), "' not defined");
		}
		return result;
	} else {
		char* ptr = NULL;
		int result = strtol(ds.c_str() + 1, &ptr, 10);
		if (*ptr != 0) {
			g_throw_parser_error("illegal data set identifier '", ds.c_str(), "'");
		}
		if (result < 0 || result >= MAX_NB_DATA) {
			g_throw_parser_error("data set identifier out of range '", ds.c_str(), "'");
		}
		if (def && dp[result] == NULL) {
			g_throw_parser_error("data set '", ds.c_str(), "' not defined");
		}
		return result;
	}
}

int get_dataset_identifier(const string& ds, GLEParser* parser, bool def) throw(ParserError) {
	Tokenizer* tokens = parser->getTokens();
	if (str_i_equals(ds, "d")) {
		tokens->ensure_next_token("[");
		int result = (int)floor(parser->evalTokenToDouble() + 0.5);
		if (result < 0 || result >= MAX_NB_DATA) {
			ostringstream err;
			err << "data set identifier out of range: '" << result << "'";
			throw tokens->error(err.str());
		}
		tokens->ensure_next_token("]");
		if (def && dp[result] == NULL) {
			ostringstream err;
			err << "data set d" << result << " not defined";
			throw tokens->error(err.str());
		}
		return result;
	} else if (str_i_equals(ds, "dn")) {
		return 0;
	} else {
		if (ds.size() <= 1 || toupper(ds[0]) != 'D') {
			throw tokens->error("illegal data set identifier");
		}
		char* ptr = NULL;
		int result = strtol(ds.c_str() + 1, &ptr, 10);
		if (*ptr != 0) {
			throw tokens->error("data set identifier should be integer");
		}
		if (result < 0 || result >= MAX_NB_DATA) {
			throw tokens->error("data set identifier out of range");
		}
		if (def && dp[result] == NULL) {
			throw tokens->error("data set '", ds.c_str(), "' not defined");
		}
		return result;
	}
}

int get_column_number(GLEParser* parser) throw(ParserError) {
	Tokenizer* tokens = parser->getTokens();
	const string& token = tokens->next_token();
	if (str_i_equals(token, "c")) {
		tokens->ensure_next_token("[");
		int result = (int)floor(parser->evalTokenToDouble() + 0.5);
		if (result < 0) {
			ostringstream err;
			err << "column index out of range: '" << result << "'";
			throw tokens->error(err.str());
		}
		tokens->ensure_next_token("]");
		return result;
	} else {
		if (token.size() <= 1 || toupper(token[0]) != 'C') {
			throw tokens->error("illegal column index '", token.c_str(), "'");
		}
		char* ptr = NULL;
		int result = strtol(token.c_str()+1, &ptr, 10);
		if (*ptr != 0) {
			throw tokens->error("column index should be integer, not '", token.c_str(), "'");
		}
		if (result < 0) {
			throw tokens->error("column index out of range '", token.c_str(), "'");
		}
		return result;
	}
}

class GLEDataSetDescription {
public:
	GLEDataSetDescription();
	void setColumnIdx(unsigned int dimension, int column);
	int getColumnIdx(unsigned int dimension) const;
	unsigned int getNrDimensions() const;

	int ds;
	bool xygiven;
private:
	vector<int> m_columnIdx;
};

class GLEDataDescription {
public:
	GLEDataDescription();
	inline int getNbDataSets() { return m_description.size(); }
	inline GLEDataSetDescription* getDataSet(int i) { return &m_description[i]; }
	inline void addDataSet(const GLEDataSetDescription& description) { m_description.push_back(description); }

private:
	vector<GLEDataSetDescription> m_description;

public:
	string fileName;
	string comment;
	string delimiters;
	unsigned int ignore;
	bool nox;
};

GLEDataSetDescription::GLEDataSetDescription() {
	ds = 0;
	xygiven = false;
}

void GLEDataSetDescription::setColumnIdx(unsigned int dimension, int column) {
	m_columnIdx.resize(std::max<unsigned int>(m_columnIdx.size(), dimension + 1), -1);
	m_columnIdx[dimension] = column;
}

int GLEDataSetDescription::getColumnIdx(unsigned int dimension) const {
	return m_columnIdx[dimension];
}

unsigned int GLEDataSetDescription::getNrDimensions() const {
	return m_columnIdx.size();
}

GLEDataDescription::GLEDataDescription() :
    comment("!"),
    delimiters(" ,;\t"),
	ignore(0),
	nox(false)
{
}

/* data a.dat												  */
/* data a.dat d2 d5											*/
/* data a.dat d1=c1,c3 d2=c5,c1						  */
/* data a.dat IGNORE n d2 d5							  */
/* data a.dat IGNORE n d1=c1,c3 d2=c5,c1			  */
/* data a.dat COMMENT # d1=c1,c3 d2=c5,c1				*/
/* data a.dat COMMENT # IGNORE n d1=c1,c3 d2=c5,c1	*/

void read_data_description(GLEDataDescription* description, GLESourceLine& sline) {
	// Get data command
	string datacmd(sline.getCode());
	// Initialize parser for this line
	GLEParser* parser = get_global_parser();
	parser->setString(datacmd.c_str());
	Tokenizer* tokens = parser->getTokens();
	tokens->ensure_next_token_i("DATA");
	// Read file name
	parser->evalTokenToFileName(&description->fileName);
	// Parse rest of the line
	while (true) {
		// read next token from the data description
		const string& token = tokens->try_next_token();
		if (token == "") {
			// no further tokens
			break;
		}
		// check if it is one of the options
		if (str_i_equals(token, "IGNORE")) {
			description->ignore = tokens->next_integer();
		} else if (str_i_equals(token, "COMMENT")) {
			parser->evalTokenToFileName(&description->comment);
		} else if (str_i_equals(token, "DELIMITERS")) {
			parser->evalTokenToString(&description->delimiters);
		} else if(str_i_equals(token,"NOX")) {
			description->nox = true;
		} else {
			// check if it is a data set identifier
			GLEDataSetDescription datasetDescription;
			datasetDescription.ds = get_dataset_identifier(token, parser, false);
			if (tokens->is_next_token("=")) {
				datasetDescription.xygiven = true;
				datasetDescription.setColumnIdx(0, get_column_number(parser));
				tokens->ensure_next_token(",");
				datasetDescription.setColumnIdx(1, get_column_number(parser));
			}
			description->addDataSet(datasetDescription);
		}
	}
}

bool isMissingValue(const char* content, unsigned int size) {
	if (size == 0) {
		return true;
	} else if (size == 1) {
		char ch = content[0];
		return ch == '*' || ch == '?' || ch == '-' || ch == '.';
	} else {
		return false;
	}
}

void get_data_value(GLECSVData* csvData, int dn, GLEArrayImpl* array, int arrayIdx, int row, int col, unsigned int dimension) {
	unsigned int size;
	const char* buffer = csvData->getCell(row, col, &size);
	if (isMissingValue(buffer, size)) {
		array->setUnknown(arrayIdx);
	} else {
		char *ptr = NULL;
		string strValue(buffer, size);
		double doubleValue = strtod(strValue.c_str(), &ptr);
		if (ptr == NULL || *ptr != 0) {
			str_remove_quote(strValue);
			array->setObject(arrayIdx, new GLEString(strValue));
		} else {
			array->setDouble(arrayIdx, doubleValue);
		}
	}
}

void createDataSet(int dn) {
	if (dn < 0 || dn >= MAX_NB_DATA) {
		g_throw_parser_error("too many data sets");
	}
	if (dn > ndata) ndata = dn;
	if (dp[dn] == NULL) {
		dp[dn] = new GLEDataSet(dn);
		copy_default(dn);
	}
}

string dimension2String(unsigned int dimension) {
	if (dimension == 0) {
		return "x";
	} else if (dimension == 1) {
		return "y";
	} else if (dimension == 2) {
		return "z";
	} else {
		ostringstream dim;
		dim << (dimension + 1);
		return dim.str();
	}
}

bool isFloatMiss(GLECSVData* csvData, unsigned int row, unsigned int col) {
	unsigned int size;
	const char* buffer = csvData->getCell(row, col, &size);
	if (isMissingValue(buffer, size)) {
		return true;
	} else if (is_float(string(buffer, size))) {
		return true;
	} else {
		return false;
	}
}

bool auto_has_header(GLECSVData* csvData, unsigned int dataColumns) {
	if (csvData->getNbLines() == 0) {
		return false;
	} else {
		for (unsigned int col = 0; col < dataColumns; col++) {
			if (isFloatMiss(csvData, 0, col)) {
				return false;
			}
		}
		return true;
	}
}

bool auto_all_labels_column(GLECSVData* csvData, unsigned int first_row) {
	if (first_row >= csvData->getNbLines()) {
		return false;
	} else {
		for (unsigned int row = first_row; row < csvData->getNbLines(); row++) {
			if (isFloatMiss(csvData, row, 0)) {
				return false;
			}
		}
		return true;
	}
}

void data_command(GLESourceLine& sline) {
	// Read data set description
	GLEDataDescription description;
	read_data_description(&description, sline);
	// Open file
	string expandedName(GLEExpandEnvironmentVariables(description.fileName));
	validate_file_name(expandedName, true);
	GLECSVData csvData;
	csvData.setDelims(description.delimiters.c_str());
	csvData.setCommentIndicator(description.comment.c_str());
	csvData.setIgnoreHeader(description.ignore);
	csvData.read(expandedName);
	unsigned int dataColumns = csvData.validateIdenticalNumberOfColumns();
	GLECSVError* error = csvData.getError();
	if (error->errorCode != GLECSVErrorNone) {
		g_throw_parser_error(error->errorString);
	}
	// Auto-detect header
	bool has_header = auto_has_header(&csvData, dataColumns);
	unsigned int first_row = has_header ? 1 : 0;
	bool all_str_col = auto_all_labels_column(&csvData, first_row);
	bool nox = description.nox || dataColumns == 1 || all_str_col;
	// Auto-assign columns
	int cx = nox ? 0 : 1;
	int cyOffs = nox && !all_str_col ? 0 : 1;
	int nbDataSetsGiven = description.getNbDataSets();
	for (int i = 0; i < nbDataSetsGiven; ++i) {
		GLEDataSetDescription* dataset = description.getDataSet(i);
		if (!dataset->xygiven) {
			dataset->setColumnIdx(0, cx);
			dataset->setColumnIdx(1, i + cyOffs + 1);
		}
	}
	if (nbDataSetsGiven == 0) {
		int nbDataSetsMax = dataColumns - cyOffs;
		for (int i = 0; i < nbDataSetsMax; ++i) {
			GLEDataSetDescription dataset;
			dataset.ds = freedataset(i + 1);
			dataset.setColumnIdx(0, cx);
			dataset.setColumnIdx(1, i + cyOffs + 1);
			description.addDataSet(dataset);
		}
	}
	// Construct names data set
	if (all_str_col) {
		int ds = 0;
		GLEDataSetDescription dataset;
		dataset.ds = ds;
		dataset.setColumnIdx(0, 0);
		dataset.setColumnIdx(1, 1);
		description.addDataSet(dataset);
		xx[GLE_AXIS_X].setNamesDataSet(ds);
	}
	// Validate column indices
	for (int i = 0; i < description.getNbDataSets(); i++) {
		GLEDataSetDescription* dataset = description.getDataSet(i);
		if (dataset->getNrDimensions() <= 0) {
			ostringstream err;
			err << "no columns defined for d" << dataset->ds;
			g_throw_parser_error(err.str());
		}
		for (unsigned int dimension = 0; dimension < dataset->getNrDimensions(); dimension++) {
			int column = dataset->getColumnIdx(dimension);
			if (column < 0 || column > (int)dataColumns) {
				ostringstream err;
				err << "dimension " << dimension2String(dimension) <<  " column index out of range for d"
					<< dataset->ds << ": " << column << " not in [0,...," << dataColumns << "]";
				g_throw_parser_error(err.str());
			}
		}
	}
	// Read header and copy to key
	if (has_header && csvData.getNbLines() > 0) {
		for (int i = 0; i < description.getNbDataSets(); i++) {
			GLEDataSetDescription* dataset = description.getDataSet(i);
			int dn = dataset->ds;
			int targetColumn = dataset->getColumnIdx(dataset->getNrDimensions() - 1);
			if (targetColumn > 0) {
				createDataSet(dn);
				string tmp(csvData.getCellString(0, targetColumn - 1));
				str_remove_quote(tmp);
				dp[dn]->key_name = sdup(tmp.c_str());
			}
		}
	}
	// Copy all data to the data sets
	for (int i = 0; i < description.getNbDataSets(); i++) {
		GLEDataSetDescription* datasetDescr = description.getDataSet(i);
		int dn = datasetDescr->ds;
		createDataSet(dn);
		GLEDataSet* dataset = dp[dn];
		unsigned int np = csvData.getNbLines() - first_row;
		dataset->clearAll();
		dataset->np = np;
		GLEArrayImpl* dataDimensions = dataset->getData();
		dataDimensions->ensure(datasetDescr->getNrDimensions());
		for (unsigned int dimension = 0; dimension < datasetDescr->getNrDimensions(); dimension++) {
			int column = datasetDescr->getColumnIdx(dimension);
			GLEArrayImpl* dataColumn = new GLEArrayImpl();
			dataDimensions->setObject(dimension, dataColumn);
			dataColumn->ensure(np);
			for (unsigned int j = 0; j < np ; j++) {
				if (column == 0) {
					dataColumn->setDouble(j, j + 1);
				} else {
					unsigned int row = first_row + j;
					get_data_value(&csvData, dn, dataColumn, j, row, column - 1, dimension);
				}
			}
		}
	}
}
