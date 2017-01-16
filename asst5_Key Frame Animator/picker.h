#ifndef PICKER_H
#define PICKER_H

#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
// #if __GNUG__
// #   include <tr1/memory>
// #endif

#include "cvec.h"
#include "scenegraph.h"
#include "asstcommon.h"
#include "ppm.h"
#include "drawer.h"

class Picker : public SgNodeVisitor 
{
	std::vector<std::shared_ptr<SgNode> > nodeStack_;

	typedef std::map<int, std::shared_ptr<SgRbtNode> > IdToRbtNodeMap;
	IdToRbtNodeMap idToRbtNode_;    //use to map the id and Rbtnode

	int idCounter_;
	bool srgbFrameBuffer_;

	Drawer drawer_;

	// can not call in asst4.cpp
	void addToMap(int id, std::shared_ptr<SgRbtNode> node);  
	std::shared_ptr<SgRbtNode> find(int id);
	Cvec3 idToColor(int id);
	int colorToId(const PackedPixel& p);

// can be called in asst4.cpp
public:
	Picker(const RigTForm& initialRbt, const ShaderState& curSS);

	virtual bool visit(SgTransformNode& node);
	virtual bool postVisit(SgTransformNode& node);
	virtual bool visit(SgShapeNode& node);
	virtual bool postVisit(SgShapeNode& node);

	std::shared_ptr<SgRbtNode> getRbtNodeAtXY(int x, int y);
};


#endif
