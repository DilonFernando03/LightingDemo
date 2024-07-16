#include "TexturedCubeNode.h"
#include "Geometry.h"
#include "GeometricObject.h"
#include "WICTextureLoader.h"

bool TexturedCubeNode::Initialise()
{
	_device = DirectXFramework::GetDXFramework()->GetDevice();
	_deviceContext = DirectXFramework::GetDXFramework()->GetDeviceContext();

	if (_device.Get() == nullptr || _deviceContext.Get() == nullptr)
	{
		return false;
	}
	if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
	{
		return false;
	}
	ComputeBox(vertices, indices, Vector3(2.0f, 2.0f, 2.0f));
	ComputeNormals();
	BuildGeometryBuffers();
	BuildShaders();
	BuildVertexLayout();
	BuildConstantBuffer();
	BuildTexture();
	return true;
}

void TexturedCubeNode::ComputeNormals() {
	const int vertexCount = vertices.size();
	const int indexCount = indices.size();
	std::vector<int> vertexcontributingCount(vertexCount, 0); // No of contributions per vertex

	for (int j = 0; j < indexCount; j += 3)
	{
		//Taking the three vertices associated with the polygon
		ObjectVertexStruct& v0 = vertices[indices[j]];
		ObjectVertexStruct& v1 = vertices[indices[j + 1]];
		ObjectVertexStruct& v2 = vertices[indices[j + 2]];

		// Constructing vectorA and vectorB
		vectorA = v1.Position - v0.Position;
		vectorB = v2.Position - v0.Position;

		// Calculating cross product of vector A and B
		NormalCalc = vectorA.Cross(vectorB);

		// Adding the polygon normal to the 3 verties associated with it
		v0.Normal += NormalCalc;
		v1.Normal += NormalCalc;
		v2.Normal += NormalCalc;

		// incrementing the associated vertices contribution count
		vertexcontributingCount[indices[j]]++;
		vertexcontributingCount[indices[j + 1]]++;
		vertexcontributingCount[indices[j + 2]]++;
	}

	for (int k = 0; k < vertexCount; k++)
	{
		if (vertexcontributingCount[k] > 0)
		{
			// Divide summed vertex normals by no.of contributions
			vertices[k].Normal /= vertexcontributingCount[k];

			// normalize resulting vector
			vertices[k].Normal.Normalize();
		}

	}
}

void TexturedCubeNode::Render()
{
	//Calculating _completeTransformation using World Trans * View Trans * Projection Trans
	Matrix _viewTransformation = DirectXFramework::GetDXFramework()->GetViewTransformation();
	Matrix _projectionTransformation = DirectXFramework::GetDXFramework()->GetProjectionTransformation();

	Matrix _completeTransformation = _cumulativeWorldTransformation * _viewTransformation * _projectionTransformation;

	CBuffer constantBuffer;
	constantBuffer.WorldViewProjection = _completeTransformation;
	constantBuffer.World = _cumulativeWorldTransformation; // Added the world transformation matrix
	constantBuffer.MaterialColour = _materialColor;
	constantBuffer.diffuseColour = Vector4(Colors::White);
	constantBuffer.AmbientLightColour = Vector4(Colors::Yellow);
	constantBuffer.DirectionalLightVector = Vector4(-1.0f, -1.0f, 1.0f, 0.0f); // Directional light vector
	constantBuffer.DirectionalLightColour = Vector4(Colors::White); // Directional light colour
	constantBuffer.EyePosition = Vector3(4.0f, 4.0f, -4.0f);
	constantBuffer.specularPower = 10;
	constantBuffer.specularPassedColour = Vector4(Colors::Turquoise);
	constantBuffer.pointLightColour = Vector4(Colors::White);

	// Update the constant buffer. Note the layout of the constant buffer must match that in the shader
	_deviceContext->VSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
	_deviceContext->PSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());//Added to Allows Pixel Shader to access the CBuffer
	_deviceContext->UpdateSubresource(_constantBuffer.Get(), 0, 0, &constantBuffer, 0, 0);

	// Update the constant buffer. Note the layout of the constant buffer must match that in the shader
	_deviceContext->VSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
	_deviceContext->UpdateSubresource(_constantBuffer.Get(), 0, 0, &constantBuffer, 0, 0);

	_deviceContext->PSSetShaderResources(0, 1, _texture.GetAddressOf());
	//rendering object
	UINT stride = sizeof(ObjectVertexStruct);
	UINT offset = 0;

	//Set vertex and index buffer
	_deviceContext->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
	_deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//Specifying the layout of the polygons
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Specify the layout of the input vertices
	_deviceContext->IASetInputLayout(_layout.Get());

	//Specify vertex and pixel shaders to be used
	_deviceContext->VSSetShader(_vertexShader.Get(), 0, 0);
	_deviceContext->PSSetShader(_pixelShader.Get(), 0, 0);

	//Drawing the cube
	_deviceContext->DrawIndexed(indices.size(), 0, 0);


}

void TexturedCubeNode::BuildGeometryBuffers()
{
	// This method uses the arrays defined in Geometry.h
	// 
	// Setup the structure that specifies how big the vertex 
	// buffer should be

	D3D11_BUFFER_DESC vertexBufferDescriptor = { 0 };
	vertexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescriptor.ByteWidth = sizeof(ObjectVertexStruct) * vertices.size();
	vertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescriptor.CPUAccessFlags = 0;
	vertexBufferDescriptor.MiscFlags = 0;
	vertexBufferDescriptor.StructureByteStride = 0;

	// Now set up a structure that tells DirectX where to get the
	// data for the vertices from
	D3D11_SUBRESOURCE_DATA vertexInitialisationData = { 0 };
	vertexInitialisationData.pSysMem = vertices.data();

	// and create the vertex buffer
	ThrowIfFailed(_device->CreateBuffer(&vertexBufferDescriptor, &vertexInitialisationData, _vertexBuffer.GetAddressOf()));

	// Setup the structure that specifies how big the index 
	// buffer should be
	D3D11_BUFFER_DESC indexBufferDescriptor = { 0 };
	indexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescriptor.ByteWidth = sizeof(UINT) * indices.size();
	indexBufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescriptor.CPUAccessFlags = 0;
	indexBufferDescriptor.MiscFlags = 0;
	indexBufferDescriptor.StructureByteStride = 0;

	// Now set up a structure that tells DirectX where to get the
	// data for the indices from
	D3D11_SUBRESOURCE_DATA indexInitialisationData;
	indexInitialisationData.pSysMem = data(indices); //self added

	// and create the index buffer
	ThrowIfFailed(_device->CreateBuffer(&indexBufferDescriptor, &indexInitialisationData, _indexBuffer.GetAddressOf()));
}

void TexturedCubeNode::BuildShaders()
{
	DWORD shaderCompileFlags = 0;
#if defined( _DEBUG )
	shaderCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compilationMessages = nullptr;

	//Compile vertex shader
	HRESULT hr = D3DCompileFromFile(TexturedShaderFileName,
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		VertexShaderName, "vs_5_0",
		shaderCompileFlags, 0,
		_vertexShaderByteCode.GetAddressOf(),
		compilationMessages.GetAddressOf());

	if (compilationMessages.Get() != nullptr)
	{
		// If there were any compilation messages, display them
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}
	// Even if there are no compiler messages, check to make sure there were no other errors.
	ThrowIfFailed(hr);
	ThrowIfFailed(_device->CreateVertexShader(_vertexShaderByteCode->GetBufferPointer(), _vertexShaderByteCode->GetBufferSize(), NULL, _vertexShader.GetAddressOf()));

	// Compile pixel shader
	hr = D3DCompileFromFile(TexturedShaderFileName,
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		PixelShaderName, "ps_5_0",
		shaderCompileFlags, 0,
		_pixelShaderByteCode.GetAddressOf(),
		compilationMessages.GetAddressOf());

	if (compilationMessages.Get() != nullptr)
	{
		// If there were any compilation messages, display them
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}
	ThrowIfFailed(hr);
	ThrowIfFailed(_device->CreatePixelShader(_pixelShaderByteCode->GetBufferPointer(), _pixelShaderByteCode->GetBufferSize(), NULL, _pixelShader.GetAddressOf()));
}

void TexturedCubeNode::BuildVertexLayout()
{
	// Create the vertex input layout. This tells DirectX the format
	// of each of the vertices we are sending to it. The vertexDesc array is
	// defined in Geometry.h

	ThrowIfFailed(_device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), _vertexShaderByteCode->GetBufferPointer(), _vertexShaderByteCode->GetBufferSize(), _layout.GetAddressOf()));
}

void TexturedCubeNode::BuildConstantBuffer()
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(CBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	ThrowIfFailed(_device->CreateBuffer(&bufferDesc, NULL, _constantBuffer.GetAddressOf()));
}

void TexturedCubeNode::Shutdown()
{
	CoUninitialize();
}

void TexturedCubeNode::BuildTexture()
{
	// Note that in order to use CreateWICTextureFromFile, we 
	// need to ensure we make a call to CoInitializeEx in our 
	// Initialise method (and make the corresponding call to 
	// CoUninitialize in the Shutdown method).  Otherwise, 
	// the following call will throw an exception
	ThrowIfFailed(CreateWICTextureFromFile(_device.Get(),
		_deviceContext.Get(),
		TextureName.c_str(),
		nullptr,
		_texture.GetAddressOf()
	));
}
