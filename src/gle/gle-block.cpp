
#include "basicconf.h"
#include "cutils.h"
#include "gle-block.h"
#include "tokens/Tokenizer.h"
#include "gle-interface/gle-interface.h"

void GLEParserInitTokenizer(Tokenizer* tokens);

GLEBlockInstance::GLEBlockInstance(GLEBlockBase* parent):
	m_parent(parent)
{
}

GLEBlockInstance::~GLEBlockInstance() {
}

GLEBlockBase* GLEBlockInstance::getParent() {
	return m_parent;
}

GLEBlockBase::GLEBlockBase(const std::string& blockName,
			               bool allowRecursiveBlocks):
    m_blockName(blockName),
    m_allowRecursiveBlocks(allowRecursiveBlocks)
{
}

GLEBlockBase::~GLEBlockBase() {
	for (std::vector<GLEBlockInstance*>::iterator i(m_blockStack.begin()); i != m_blockStack.end(); ++i) {
		delete *i;
	}
}

void GLEBlockBase::beginExecuteBlock(GLESourceLine& sline, int *pcode, int *cp) {
	if (!allowRecursiveBlocks() && !m_blockStack.empty()) {
		g_throw_parser_error("recursive calls to '", getBlockName().c_str(), "' blocks not allowed");
	}
	GLEBlockInstance* block = beginExecuteBlockImpl(sline, pcode, cp);
	m_blockStack.push_back(block);
}

void GLEBlockBase::executeLine(GLESourceLine& sline) {
	if (m_blockStack.empty()) {
		g_throw_parser_error("not in block '", getBlockName().c_str(), "'");
	} else {
		m_blockStack.back()->executeLine(sline);
	}
}

void GLEBlockBase::endExecuteBlock() {
	if (m_blockStack.empty()) {
		g_throw_parser_error("not in block '", getBlockName().c_str(), "'");
	} else {
		GLEBlockInstance* block = m_blockStack.back();
		block->endExecuteBlock();
		delete block;
		m_blockStack.pop_back();
	}
}

bool GLEBlockBase::allowRecursiveBlocks() const {
	return m_allowRecursiveBlocks;
}

const std::string GLEBlockBase::getBlockName() const {
	return m_blockName;
}

GLEBlockWithSimpleKeywords::GLEBlockWithSimpleKeywords(const std::string& blockName, bool allowRecursiveBlocks):
	GLEBlockBase(blockName, allowRecursiveBlocks)
{
}

GLEBlockWithSimpleKeywords::~GLEBlockWithSimpleKeywords() {
}

bool GLEBlockWithSimpleKeywords::checkLine(GLESourceLine& sline) {
	StringTokenizer tokens(sline.getCodeCStr());
	GLEParserInitTokenizer(&tokens);
	if (tokens.has_more_tokens()) {
		const std::string token = tokens.next_token();
		return m_keyWords.find(token) != m_keyWords.end();
	} else {
		return false;
	}
}

void GLEBlockWithSimpleKeywords::addKeyWord(const char* keyword) {
	m_keyWords.insert(keyword);
}

void GLEBlockWithSimpleKeywords::addKeyWord(const std::string& keyword) {
	m_keyWords.insert(keyword);
}

GLEBlocks::GLEBlocks() {
}

GLEBlocks::~GLEBlocks() {
	for (std::map<int, GLEBlockBase*>::iterator i(m_blocks.begin()); i != m_blocks.end(); ++i) {
		delete i->second;
	}
}

void GLEBlocks::addBlock(int blockType, GLEBlockBase* block) {
	std::map<int, GLEBlockBase*>::iterator i(m_blocks.find(blockType));
	CUtilsAssert(i == m_blocks.end());
	m_blocks.insert(std::make_pair(blockType, block));
}

GLEBlockBase* GLEBlocks::getBlock(int blockType) {
	std::map<int, GLEBlockBase*>::iterator i(m_blocks.find(blockType));
	CUtilsAssert(i != m_blocks.end());
	return i->second;
}

GLEBlockBase* GLEBlocks::getBlockIfExists(int blockType) {
	std::map<int, GLEBlockBase*>::iterator i(m_blocks.find(blockType));
	if (i != m_blocks.end()) {
		return i->second;
	} else {
		return 0;
	}
}
