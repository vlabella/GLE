#ifndef GLE_BLOCK_H
#define GLE_BLOCK_H

class GLEBlockBase;
class GLESourceLine;

#include <vector>
#include <map>
#include <set>

class GLEBlockInstance {
public:
	GLEBlockInstance(GLEBlockBase* parent);
	virtual ~GLEBlockInstance();

	virtual void executeLine(GLESourceLine& sline) = 0;
	virtual void endExecuteBlock() = 0;

	GLEBlockBase* getParent();

private:
	GLEBlockBase* m_parent;
};

class GLEBlockBase {
public:
	GLEBlockBase(const std::string& blockName,
			     bool allowRecursiveBlocks);
	virtual ~GLEBlockBase();

	virtual GLEBlockInstance* beginExecuteBlockImpl(GLESourceLine& sline, int *pcode, int *cp) = 0;
	virtual bool checkLine(GLESourceLine& sline) = 0;

	void beginExecuteBlock(GLESourceLine& sline, int *pcode, int *cp);
	void executeLine(GLESourceLine& sline);
	void endExecuteBlock();

	bool allowRecursiveBlocks() const;
	const std::string getBlockName() const;

private:
	std::string m_blockName;
	bool m_allowRecursiveBlocks;
	std::vector<GLEBlockInstance*> m_blockStack;
};

class GLEBlockWithSimpleKeywords : public GLEBlockBase {
public:
	GLEBlockWithSimpleKeywords(const std::string& blockName,
			                   bool allowRecursiveBlocks);
	virtual ~GLEBlockWithSimpleKeywords();

	virtual bool checkLine(GLESourceLine& sline);

	void addKeyWord(const char* keyword);
	void addKeyWord(const std::string& keyword);

private:
	std::set<std::string, str_i_less> m_keyWords;
};

class GLEBlocks {
public:
	GLEBlocks();
	~GLEBlocks();

	void addBlock(int blockType, GLEBlockBase* block);
	GLEBlockBase* getBlock(int blockType);
	GLEBlockBase* getBlockIfExists(int blockType);

private:
	std::map<int, GLEBlockBase*> m_blocks;
};

#endif
