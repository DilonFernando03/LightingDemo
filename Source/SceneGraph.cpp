#include "SceneGraph.h"
bool SceneGraph::Initialise()
{
	for (SceneNodePointer child : _children)
	{
		if (!child->Initialise())
		{
			return false;
		}
	}
	return true;
}

void SceneGraph::Update(const Matrix& worldTrans)
{
	SceneNode::Update(worldTrans);
	for (SceneNodePointer child : _children)
	{
		child->Update(_cumulativeWorldTransformation);
	}

}

void SceneGraph::Render()
{
	for (SceneNodePointer child : _children)
	{
		child->Render();
	}
}

void SceneGraph::Shutdown()
{
	for (SceneNodePointer child : _children)
	{
		child->Shutdown();
	}
}

void SceneGraph::Add(SceneNodePointer node)
{
	_children.push_back(node);
}

void SceneGraph::Remove(SceneNodePointer node)
{
	_children.remove(node);
}

SceneNodePointer SceneGraph::Find(wstring name)
{
	/*Node being searched for is itself*/
	if (_name == name)
	{
		return shared_from_this();
	}

	/*Cycling through nodes till it's found (or else a null ptr returned)*/
	for (SceneNodePointer child : _children)
	{
		SceneNodePointer foundNode = child->Find(name);
		if (foundNode)
		{
			return foundNode;
		}
	}
	return nullptr;
}