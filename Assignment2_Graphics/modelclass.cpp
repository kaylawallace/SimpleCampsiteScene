////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "modelclass.h"

using namespace DirectX;

ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;

}
ModelClass::~ModelClass()
{
}


bool ModelClass::InitializeModel(ID3D11Device *device, char* filename)
{
	LoadModel(filename);
	InitializeBuffers(device);
	return false;
}

bool ModelClass::InitializeTeapot(ID3D11Device* device)
{
	GeometricPrimitive::CreateTeapot(preFabVertices, preFabIndices, 1, 8, false);
	m_vertexCount	= preFabVertices.size();
	m_indexCount	= preFabIndices.size();

	bool result;
	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}
	return true;
}

bool ModelClass::InitializeSphere(ID3D11Device *device)
{
	GeometricPrimitive::CreateSphere(preFabVertices, preFabIndices, 1, 8, false);
	m_vertexCount = preFabVertices.size();
	m_indexCount = preFabIndices.size();

	bool result;
	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
	return true;
}

bool ModelClass::InitializeBox(ID3D11Device * device, float xwidth, float yheight, float zdepth)
{
	GeometricPrimitive::CreateBox(preFabVertices, preFabIndices,
		DirectX::SimpleMath::Vector3(xwidth, yheight, zdepth),false);
	m_vertexCount = preFabVertices.size();
	m_indexCount = preFabIndices.size();

	bool result;
	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
	return true;
}

void ModelClass::Shutdown()
{

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	// Release the model data.
	ReleaseModel();

	return;
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	return;
}

//void ModelClass::RenderWithTransformations(ID3D11DeviceContext* context, SimpleMath::Matrix m_world)

//void ModelClass::RenderSkybox(ID3D11DeviceContext* deviceContext) {
//	deviceContext->UpdateSubresource()
//}

// 
// Custom Geometry 
//
// Prism creation and normals calculation 
bool ModelClass::InitializePrism(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	
	m_vertexCount = 8;
	m_indexCount = 36;

	vertices = new VertexType[m_vertexCount];
	indices = new unsigned long[m_indexCount];

	// Set prism vertex positions
	vertices[0].position = DirectX::SimpleMath::Vector3(0.f, 0.f, 0.f);		//a
	vertices[1].position = DirectX::SimpleMath::Vector3(-0.5f, 1.f, 0.f);	//b
	vertices[2].position = DirectX::SimpleMath::Vector3(-1.f, 0.f, 0.f);	//c
	vertices[3].position = DirectX::SimpleMath::Vector3(0.f, 0.f, 2.f);		//d
	vertices[4].position = DirectX::SimpleMath::Vector3(-0.5f, 1.f, 2.f);	//e
	vertices[5].position = DirectX::SimpleMath::Vector3(-1.f, 0.f, 2.f);	//f

	// Set indices (grouped by faces - f_)
	// f1 
	indices[0] = 0;
	indices[1] = 2;
	indices[2] = 1;

	// f2
	indices[3] = 2;
	indices[4] = 4;
	indices[5] = 1;

	// f3
	indices[6] = 2;
	indices[7] = 5;
	indices[8] = 4;

	// f4
	indices[9] = 0;
	indices[10] = 1;
	indices[11] = 4;

	// f5
	indices[12] = 0;
	indices[13] = 4;
	indices[14] = 3;

	// f6
	indices[15] = 3;
	indices[16] = 4;
	indices[17] = 5;

	// f7
	indices[18] = 0;
	indices[19] = 3;
	indices[20] = 5;

	// f8
	indices[21] = 0;
	indices[22] = 5;
	indices[23] = 2;

	// Normals for f1
	// Calculate direction vectors for ab and ac (b-a and c-a)
	SimpleMath::Vector3 ab = vertices[1].position - vertices[0].position;
	SimpleMath::Vector3 ac = vertices[2].position - vertices[0].position;
	// Calculate cross product of ac and ab and normalize for normal to face 1 
	SimpleMath::Vector3 f1_normal = ac.Cross(ab);
	f1_normal.Normalize();
	
	// Normals for f2
	SimpleMath::Vector3 bc = vertices[2].position - vertices[1].position;
	SimpleMath::Vector3 be = vertices[4].position - vertices[1].position;
	SimpleMath::Vector3 f2_normal = be.Cross(bc);
	f2_normal.Normalize();

	// Normals for f3
	SimpleMath::Vector3 ce = vertices[4].position - vertices[2].position;
	SimpleMath::Vector3 cf = vertices[5].position - vertices[2].position;
	SimpleMath::Vector3 f3_normal = cf.Cross(ce);
	f3_normal.Normalize();

	// Normals for f4
	SimpleMath::Vector3 ba = vertices[1].position - vertices[4].position;
	SimpleMath::Vector3 f4_normal = ce.Cross(ba);
	f4_normal.Normalize();

	// Normals for f5
	SimpleMath::Vector3 da = vertices[0].position - vertices[3].position;
	SimpleMath::Vector3 de = vertices[4].position - vertices[3].position;
	SimpleMath::Vector3 f5_normal = de.Cross(da);
	f5_normal.Normalize();

	// Normals for f6
	SimpleMath::Vector3 ed = vertices[3].position - vertices[4].position;
	SimpleMath::Vector3 ef = vertices[5].position - vertices[4].position;
	SimpleMath::Vector3 f6_normal = ef.Cross(ed);
	f6_normal.Normalize();

	// Normals for f7
	SimpleMath::Vector3 df = vertices[5].position - vertices[3].position;
	SimpleMath::Vector3 f7_normal = df.Cross(da);
	f7_normal.Normalize();

	// Normals for f8
	SimpleMath::Vector3 fa = vertices[0].position - vertices[5].position;
	SimpleMath::Vector3 fc = vertices[2].position - vertices[5].position;
	SimpleMath::Vector3 f8_normal = fc.Cross(fa);
	f8_normal.Normalize();

	// Vertex 0 (point a) normal 
	// All vertex normals calculated by taking the average of all adjacent face normals 
	SimpleMath::Vector3 a_normal = f1_normal + f4_normal + f5_normal + f7_normal + f8_normal;
	a_normal = a_normal / (a_normal.Length());

	// Vertex 1 (point b) normal
	SimpleMath::Vector3 b_normal = f1_normal + f2_normal + f4_normal;
	b_normal = b_normal / (b_normal.Length());

	// Vertex 2 (point c) normal
	SimpleMath::Vector3 c_normal = f1_normal + f2_normal + f3_normal + f8_normal;
	c_normal = c_normal / (c_normal.Length());

	// Vertex 3 (point d) normal
	SimpleMath::Vector3 d_normal = f5_normal + f6_normal + f7_normal;
	d_normal = d_normal / (d_normal.Length());

	// Vertex 4 (point e) normal
	SimpleMath::Vector3 e_normal = f2_normal + f3_normal + f4_normal + f5_normal + f6_normal;
	e_normal = e_normal / (e_normal.Length());

	// Vertex 5 (point f) normal
	SimpleMath::Vector3 f_normal = f6_normal + f3_normal + f7_normal + f8_normal;
	f_normal = f_normal / (f_normal.Length());

	// Assign normal values to the vertices array 
	vertices[0].normal = a_normal;
	vertices[1].normal = b_normal;
	vertices[2].normal = c_normal;
	vertices[3].normal = d_normal;
	vertices[4].normal = e_normal;
	vertices[5].normal = f_normal;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}


int ModelClass::GetIndexCount()
{
	return m_indexCount;
}


bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}
	
	// Load the vertex array and index array with data from the pre-fab
	for (i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position	= DirectX::SimpleMath::Vector3(preFabVertices[i].position.x, preFabVertices[i].position.y, preFabVertices[i].position.z);
		vertices[i].texture		= DirectX::SimpleMath::Vector2(preFabVertices[i].textureCoordinate.x, preFabVertices[i].textureCoordinate.y);
		vertices[i].normal		= DirectX::SimpleMath::Vector3(preFabVertices[i].normal.x, preFabVertices[i].normal.y, preFabVertices[i].normal.z);
	}
	for (i = 0; i < m_indexCount; i++)
	{
		indices[i] = preFabIndices[i];
	}

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}


void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


bool ModelClass::LoadModel(char* filename)
{
	std::vector<XMFLOAT3> verts;
	std::vector<XMFLOAT3> norms;
	std::vector<XMFLOAT2> texCs;
	std::vector<unsigned int> faces;

	FILE* file;// = fopen(filename, "r");
	errno_t err;
	err = fopen_s(&file, filename, "r");
	if (err != 0)
		//if (file == NULL)
	{
		return false;
	}

	while (true)
	{
		char lineHeader[128];

		// Read first word of the line
		int res = fscanf_s(file, "%s", lineHeader, sizeof(lineHeader));
		if (res == EOF)
		{
			break; // exit loop
		}
		else // Parse
		{
			if (strcmp(lineHeader, "v") == 0) // Vertex
			{
				XMFLOAT3 vertex;
				fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				verts.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0) // Tex Coord
			{
				XMFLOAT2 uv;
				fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
				texCs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0) // Normal
			{
				XMFLOAT3 normal;
				fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				norms.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0) // Face
			{
				unsigned int face[9];
				int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &face[0], &face[1], &face[2],
					&face[3], &face[4], &face[5],
					&face[6], &face[7], &face[8]);
				if (matches != 9)
				{
					// Parser error, or not triangle faces
					return false;
				}

				for (int i = 0; i < 9; i++)
				{
					faces.push_back(face[i]);
				}


			}
		}
	}

	int vIndex = 0, nIndex = 0, tIndex = 0;
	int numFaces = (int)faces.size() / 9;

	//// Create the model using the vertex count that was read in.
	m_vertexCount = numFaces * 3;
//	model = new ModelType[vertexCount];

	// "Unroll" the loaded obj information into a list of triangles.
	for (int f = 0; f < (int)faces.size(); f += 3)
	{
		VertexPositionNormalTexture tempVertex;
		tempVertex.position.x = verts[(faces[f + 0] - 1)].x;
		tempVertex.position.y = verts[(faces[f + 0] - 1)].y;
		tempVertex.position.z = verts[(faces[f + 0] - 1)].z;

		tempVertex.textureCoordinate.x = texCs[(faces[f + 1] - 1)].x;
		tempVertex.textureCoordinate.y = texCs[(faces[f + 1] - 1)].y;
			
		tempVertex.normal.x = norms[(faces[f + 2] - 1)].x;
		tempVertex.normal.y = norms[(faces[f + 2] - 1)].y;
		tempVertex.normal.z = norms[(faces[f + 2] - 1)].z;

		//increase index count
		preFabVertices.push_back(tempVertex);
		
		int tempIndex;
		tempIndex = vIndex;
		preFabIndices.push_back(tempIndex);
		vIndex++;
	}

	m_indexCount = vIndex;

	verts.clear();
	norms.clear();
	texCs.clear();
	faces.clear();
	return true;
}


void ModelClass::ReleaseModel()
{
	return;
}