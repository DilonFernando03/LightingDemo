#pragma once
#include "core.h"
#include "DirectXCore.h"


using namespace std;

// Abstract base class for all nodes of the scene graph.  
// This scene graph implements the Composite Design Pattern

class SceneNode;

typedef shared_ptr<SceneNode>	SceneNodePointer;

class SceneNode : public enable_shared_from_this<SceneNode>
{
public:
	SceneNode(wstring name) {_name = name;};
	~SceneNode(void) {};

	// Core methods
	virtual bool Initialise() = 0;
	virtual void Update(const Matrix& worldTransformation) { _cumulativeWorldTransformation = _thisWorldTransformation * worldTransformation; }	
	virtual void Render() = 0;
	virtual void Shutdown() {}

	void SetWorldTransform(const Matrix& worldTransformation) { _thisWorldTransformation = worldTransformation; }
		
	// Although only required in the composite class, these are provided
	// in order to simplify the code base for recursive operations

	virtual void Add(SceneNodePointer node) {}
	virtual void Remove(SceneNodePointer node) {};
	virtual	SceneNodePointer Find(wstring name) { return (_name == name) ? shared_from_this() : nullptr; }

protected:
	Matrix				_thisWorldTransformation;
	Matrix				_cumulativeWorldTransformation;
	wstring				_name;
};

