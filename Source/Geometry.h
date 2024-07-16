#pragma once
//#include"SimpleMath.h"
//#include"SimpleMath.cpp"

#define TexturedShaderFileName		L"texturedshader.hlsl"
#define ShaderFileName		L"shader.hlsl"
#define VertexShaderName	"VS"
#define PixelShaderName		"PS"

// Format of the constant buffer. This must match the format of the
// cbuffer structure in the shader

struct CBuffer
{
	Matrix WorldViewProjection;
	Matrix World; // self added
	Vector4 MaterialColour;
	Vector4 diffuseColour;
	Vector4 AmbientLightColour;
	Vector4 DirectionalLightColour; // self added
	Vector4 DirectionalLightVector; // self added
	Vector3 EyePosition;
	float specularPower;
	Vector4 specularPassedColour;
	/*
	float pointAttenuationA;
	float pointAttenuationB;
	float pointAttenuationC;
	*/
	Vector4 pointLightColour;
};



// The description of the vertex that is passed to CreateInputLayout.  This must
// match the format of the vertex above and the format of the input vertex in the shader

static D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{"TEXTURECOORDINATE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
 


