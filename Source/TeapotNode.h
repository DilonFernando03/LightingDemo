#pragma once
#include "SceneNode.h"
#include "DirectXFramework.h"
#include "GeometricObject.h"
class TeapotNode : public SceneNode
{
public:
	TeapotNode(wstring name) : TeapotNode(name, Vector4(0.25f, 0.25f, 0.25f, 1.0f)) {};
	TeapotNode(wstring name, const Vector4& materialColor) : SceneNode(name), _materialColor(materialColor){};
	~TeapotNode(void) {};
	virtual bool Initialise();
	virtual void Render();
	virtual void Shutdown();
private:
	ComPtr<ID3D11Buffer> _vertexBuffer;
	ComPtr<ID3D11Buffer> _indexBuffer;
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _deviceContext;
	ComPtr<ID3D11InputLayout> _inputLayout;
	ComPtr<ID3D11VertexShader> _vertexShader;
	ComPtr<ID3D11PixelShader> _pixelShader;
	ComPtr<ID3D11Buffer> _constantBuffer;
	ComPtr<ID3D11InputLayout> _layout;
	ComPtr<ID3D10Blob> _pixelShaderByteCode = nullptr;
	ComPtr<ID3D10Blob> _vertexShaderByteCode = nullptr;
	Vector4 _materialColor;
	std::vector<ObjectVertexStruct> vertices;
	std::vector<UINT>				indices;
	const Vector3 size;
	Vector2 _objTexture;
	Vector3	vectorA;
	Vector3	vectorB;
	Vector3	NormalCalc;
	void BuildGeometryBuffers();
	void BuildShaders();
	void BuildVertexLayout();
	void BuildConstantBuffer();
	void ComputeNormals();
};

