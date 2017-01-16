#ifndef DRAWER_H
#define DRAWER_H

#include <vector>

#include "scenegraph.h"
#include "asstcommon.h"

class Drawer : public SgNodeVisitor {
protected:
	std::vector<RigTForm> rbtStack_;
	const ShaderState& curSS_;
public:
	Drawer(const RigTForm& initialRbt, const ShaderState& curSS) //initialization
		: rbtStack_(1, initialRbt)
		, curSS_(curSS) {}

	virtual bool visit(SgTransformNode& node) 
	{
		rbtStack_.push_back(rbtStack_.back() * node.getRbt()); // push last element * Rbt in the stack
		return true;
	}

	virtual bool postVisit(SgTransformNode& node) 
	{
		rbtStack_.pop_back(); // delete the last element from stack
		return true;
	}

	virtual bool visit(SgShapeNode& shapeNode)    // not push it into stack, just draw it
	{
		const Matrix4 MVM = rigTFormToMatrix(rbtStack_.back()) * shapeNode.getAffineMatrix();
		sendModelViewNormalMatrix(curSS_, MVM, normalMatrix(MVM));
		shapeNode.draw(curSS_);
		return true;
	}

	virtual bool postVisit(SgShapeNode& shapeNode) 
	{
		return true;
	}

	const ShaderState& getCurSS() const 
	{
		return curSS_;
	}
};

#endif



