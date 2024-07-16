#include "DirectXApp.h"
#include "TeapotNode.h"
#include "TexturedCubeNode.h"
#include "CubeNode.h"
using namespace std;
DirectXApp app;
shared_ptr<TexturedCubeNode> steve;
shared_ptr<TeapotNode> teapot;
shared_ptr<CubeNode> body;
DirectXApp::DirectXApp() : DirectXFramework(800, 600)
{
	// Initialise vectors used to create camera.  We will look
	// at this in detail later
	DirectXFramework::SetCameraPosition(Vector3(0.0f, 0, -20.0f));
	DirectXFramework::SetCameraFocalPoint(Vector3(0.0f, 0, 0.0f));
}



void DirectXApp::CreateSceneGraph()
{
    SceneGraphPointer sceneGraph = GetSceneGraph();
    // Teapot
	teapot = make_shared<TeapotNode>(L"Teapot", Vector4(1.0f, 0, 1.0f, 0.5f));
	sceneGraph->Add(teapot);

    // Body
    steve = make_shared<TexturedCubeNode>(L"Body", Vector4(1.0f, 1.0f, 1.0f, 1.0f), L"steve t-shirt.jpg");
    steve->SetWorldTransform(Matrix::CreateScale(Vector3(2.0f, 3.0f, 1.0f)) * Matrix::CreateTranslation(Vector3(0.0f, 0.0f, 0.0f)));
    sceneGraph->Add(steve);

    // Left Leg
    body = make_shared<CubeNode>(L"LeftLeg", Vector4(73.0 / 255.0, 70.0 / 255.0, 151.0 / 255.0, 1.0));
    body->SetWorldTransform(Matrix::CreateScale(Vector3(1.5f, 5.5f, 1.0f)) * Matrix::CreateTranslation(Vector3(-1.0f, -5.8f, 0.0f)));
    sceneGraph->Add(body);

    // Right Leg
    body = make_shared<CubeNode>(L"RightLeg", Vector4(73.0 / 255.0, 70.0 / 255.0, 151.0 / 255.0, 1.0));
    body->SetWorldTransform(Matrix::CreateScale(Vector3(1.5f, 5.5f, 1.0f)) * Matrix::CreateTranslation(Vector3(1.0f, -5.8f, 0.0f)));
    sceneGraph->Add(body);

    // Head
    steve = make_shared<TexturedCubeNode>(L"Head", Vector4(1.0f, 1.0f, 1.0f, 1.0f), L"steve.png");
    steve->SetWorldTransform(Matrix::CreateScale(Vector3(1.5f, 2.0f, 1.0f)) * Matrix::CreateTranslation(Vector3(0.0f, 5.0f, 0.0f)));
    sceneGraph->Add(steve);

    // creating sub scene graph for arms of robot
    SceneGraphPointer arms = make_shared<SceneGraph>(L"Arms");
    sceneGraph->Add(arms);

    // Left Arm
    body = make_shared<CubeNode>(L"LeftArm", Vector4(169.0 / 255.0, 125.0 / 255.0, 100.0 / 255.0, 1.0));
    body->SetWorldTransform(Matrix::CreateScale(Vector3(1.0f, 5.5f, 1.0f)) * Matrix::CreateTranslation(Vector3(2.5f, -1.0f, 0.0f)));
    arms->Add(body);

    // Right Arm
    body = make_shared<CubeNode>(L"RightArm", Vector4(169.0 / 255.0, 125.0 / 255.0, 100.0 / 255.0, 1.0));
    body->SetWorldTransform(Matrix::CreateScale(Vector3(1.0f, 5.5f, 1.0f)) * Matrix::CreateTranslation(Vector3(-2.5f, -1.0f, 0.0f)));
    arms->Add(body);

    _rotationAngle = 0;
    _yOffset = 0.0f;
    _isGoingUp = false;
	_rotationAngle = 0;
}

void DirectXApp::UpdateSceneGraph()
{
    SceneGraphPointer sceneGraph = GetSceneGraph();
    SceneNodePointer teapotupdate = sceneGraph->Find(L"Teapot");
    SceneNodePointer arms = sceneGraph->Find(L"Arms");
    // Apply rotation to the teapot
    _rotationAngle += 0.5f;
    Matrix rotationMatrix = Matrix::CreateRotationY(_rotationAngle * XM_PI / 180.0f);
    teapotupdate->SetWorldTransform(rotationMatrix * Matrix::CreateTranslation(-7.0f, 0, 0));

    // Rotation for robot arms to swing
    float leftArmRotation = sin(_rotationAngle * XM_PI / 180.0f) * 45.0f;  // Swinging left arm
    float rightArmRotation = -sin(_rotationAngle * XM_PI / 180.0f) * 45.0f;  // Swinging right arm
    arms->SetWorldTransform(Matrix::CreateRotationX(rightArmRotation * XM_PI / 180.0f));
}
