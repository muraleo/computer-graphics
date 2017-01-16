#include <algorithm>

#include "scenegraph.h"

using namespace std;
// using namespace std::tr1;

bool SgTransformNode::accept(SgNodeVisitor& visitor) 
{
	if (!visitor.visit(*this))
		return false;
	for (int i = 0, n = children_.size(); i < n; ++i) 
	{
		if (!children_[i]->accept(visitor))
			return false;
	}
	return visitor.postVisit(*this);
}

void SgTransformNode::addChild(shared_ptr<SgNode> child) 
{
	children_.push_back(child);
}

void SgTransformNode::removeChild(shared_ptr<SgNode> child) 
{
	children_.erase(find(children_.begin(), children_.end(), child));
}

bool SgShapeNode::accept(SgNodeVisitor& visitor) 
{
	if (!visitor.visit(*this))
		return false;
	return visitor.postVisit(*this);
}

class RbtAccumVisitor : public SgNodeVisitor 
{
protected:
	vector<RigTForm> rbtStack_;
	SgTransformNode& target_;
	bool found_;
public:
	RbtAccumVisitor(SgTransformNode& target)
		: target_(target)
		, found_(false) {}

	const RigTForm getAccumulatedRbt(int offsetFromStackTop = 0) 
	{
		// TODO
		int target_rbt_number = rbtStack_.size() - 1- offsetFromStackTop;
		return rbtStack_[target_rbt_number];  // get the target Rbt
	}

	virtual bool visit(SgTransformNode& node) 
	{
		// TODO
		if (rbtStack_.empty()) // when stack is empty, push the node's rbt
		{
			rbtStack_.push_back(node.getRbt()); 
		} 
		else  // just like drawer
		{
			rbtStack_.push_back(rbtStack_.back() * node.getRbt());
		}
		if(target_ == node)
		{
			return false; // get the target and terminate the visit
		}
		else
		{
			return true; // not get the target, so continue visit
		}
	}

	virtual bool postVisit(SgTransformNode& node) 
	{
		// TODO
		rbtStack_.pop_back();  // pop the last node
		return true;
	}
};

RigTForm getPathAccumRbt(
	shared_ptr<SgTransformNode> source,
	shared_ptr<SgTransformNode> destination,
	int offsetFromDestination) 
{
	RbtAccumVisitor accum(*destination);
	source->accept(accum);
	return accum.getAccumulatedRbt(offsetFromDestination);
}
