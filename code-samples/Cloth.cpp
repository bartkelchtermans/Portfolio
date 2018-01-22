#include "Cloth.h"
//#include "stdafx.h"

Cloth::Cloth(float width, float length, int resolution, XMFLOAT4 color, float externalForce)
	:m_Width(width)
	, m_Length(length)
	, m_Resolution(resolution)
	, m_ExternalAcceleration(externalForce, externalForce, externalForce)
	, m_Color(color)
{
}

Cloth::~Cloth(void) 
{
}

void Cloth::Initialize(const GameContext& gameContext) 
{	
	UNREFERENCED_PARAMETER(gameContext);

	// Get physics & scene
	auto &physX = PxGetPhysics();
	auto gameScene = GetScene();

	// Amount of vertices on one edge
	int vertexRes = m_Resolution + 2;
	float halfWidth = m_Width / 2;

	for (int y = 0; y < vertexRes; ++y) 
	{
		for (int x = 0; x < vertexRes; x++) 
		{
			PxClothParticle particle(PxVec3(-halfWidth + x * (m_Width / (vertexRes - 1)), -y * (m_Length / vertexRes), 0.0f), (rand() % 1 + 100) / 100.0f);
			m_Particles.push_back(particle);
		}
	}

	// Set invWeight to 0
	for (int i = 0; i < vertexRes; ++i) m_Particles[i].invWeight = 0.f;

	// Store indices
	for (int y = 0; y < vertexRes - 1; y++) 
	{
		for (int x = 0; x < vertexRes - 1; x++) 
		{
			m_Primitives.push_back((y * vertexRes) + x);
			m_Primitives.push_back((y * vertexRes) + x + 1);
			m_Primitives.push_back(((y + 1) * vertexRes) + x + 1);
			m_Primitives.push_back(((y + 1) * vertexRes) + x);
		}
	}

	m_VertexCount = (vertexRes) * (vertexRes);
	m_IndexCount = (vertexRes - 1) * (vertexRes - 1) * 4;

	// Cloth mesh descriptor
	PxClothMeshDesc meshDesc;
	meshDesc.points.data = m_Particles.data();
	meshDesc.points.count = m_VertexCount;
	meshDesc.points.stride = sizeof(PxClothParticle);

	meshDesc.invMasses.data = &m_Particles.data()->invWeight;
	meshDesc.invMasses.count = m_VertexCount;
	meshDesc.invMasses.stride = sizeof(PxClothParticle);

	meshDesc.quads.data = m_Primitives.data();
	meshDesc.quads.count = m_IndexCount / 4;
	meshDesc.quads.stride = sizeof(PxU32) * 4;

	// Create fabric & pose
	PxClothFabric* fabric = PxClothFabricCreate(physX, meshDesc, PxVec3(0, -1, 0));
	PxTransform pose = PxTransform(ToPxVec3(GetTransform()->GetWorldPosition()));

	// Create cloth
	m_pCloth = physX.createCloth(pose, *fabric, m_Particles.data(), PxClothFlags());
	m_pCloth->setSolverFrequency(240.0f);

	// Add cloth actor
	gameScene->GetPhysxProxy()->GetPhysxScene()->addActor(*m_pCloth);
	gameScene->GetPhysxProxy()->GetPhysxScene()->setVisualizationParameter(PxVisualizationParameter::eCLOTH_HORIZONTAL, 1);
	gameScene->GetPhysxProxy()->GetPhysxScene()->setVisualizationParameter(PxVisualizationParameter::eCLOTH_VERTICAL, 1);
	gameScene->GetPhysxProxy()->GetPhysxScene()->setVisualizationParameter(PxVisualizationParameter::eCLOTH_BENDING, 1);

	// Cloth settings
	m_pCloth->setInertiaScale(0.5f);
	m_pCloth->setClothFlag(PxClothFlag::eSCENE_COLLISION, true);
	m_pCloth->setClothFlag(PxClothFlag::eSWEPT_CONTACT, true);
	m_pCloth->setStretchConfig(PxClothFabricPhaseType::eVERTICAL, PxClothStretchConfig(1.0f));
	m_pCloth->setStretchConfig(PxClothFabricPhaseType::eHORIZONTAL, PxClothStretchConfig(0.9f));
	m_pCloth->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.75f));
	m_pCloth->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig(0.5f));


	m_pCloth->addCollisionPlane(PxClothCollisionPlane(PxVec3(0.0f, -1.0f, 0.0f), 0.0f));
	m_pCloth->addCollisionConvex(1 << 0); // Convex references the first plane

	m_pCloth->setExternalAcceleration(m_ExternalAcceleration);
	m_pCloth->setSelfCollisionDistance(0.02f);
	m_pCloth->setDragCoefficient(0.2f);

	// Normals
	CalculateNormals(3);

	for (size_t i = 0; i < UINT(m_VertexCount); i++) 
	{
		m_Normals[i] = m_Normals[i].getNormalized();
	}

	// Mesh object
	m_pMeshComp = new MeshDrawComponent(m_IndexCount / 2);
	for (size_t i = 0; i < UINT(m_IndexCount); i += 4) 
	{
		m_pMeshComp->AddQuad(
			VertexPosNormCol(ToXMFLOAT3(m_Particles[m_Primitives[i]].pos), ToXMFLOAT3(m_Normals[m_Primitives[i]]), m_Color),
			VertexPosNormCol(ToXMFLOAT3(m_Particles[m_Primitives[i + 1]].pos), ToXMFLOAT3(m_Normals[m_Primitives[i + 1]]), m_Color),
			VertexPosNormCol(ToXMFLOAT3(m_Particles[m_Primitives[i + 2]].pos), ToXMFLOAT3(m_Normals[m_Primitives[i + 2]]), m_Color),
			VertexPosNormCol(ToXMFLOAT3(m_Particles[m_Primitives[i + 3]].pos), ToXMFLOAT3(m_Normals[m_Primitives[i + 3]]), m_Color));
	}
	AddComponent(m_pMeshComp);

	// Release fabric
	fabric->release();
}

void Cloth::Update(const GameContext& gameContext) {

	UNREFERENCED_PARAMETER(gameContext);

	// Set pose
	XMFLOAT4 rotation = GetTransform()->GetWorldRotation();
	m_pCloth->setTargetPose(PxTransform(ToPxVec3(GetTransform()->GetWorldPosition())));// , PxQuat(rotation.x, rotation.y + 90, rotation.z + 0.1f, rotation.w)));

	// Get updated cloth data
	PxClothParticleData* pData = m_pCloth->lockParticleData();
	PxClothParticle* pVertices = pData->particles;

	// Calculate new normals for mesh
	CalculateNormals(1);

	m_pMeshComp->RemoveTriangles();
	for (size_t i = 0; i < UINT(m_IndexCount); i += 4) 
	{
		m_pMeshComp->AddQuad(
			VertexPosNormCol(ToXMFLOAT3(pVertices[m_Primitives[i]].pos), ToXMFLOAT3(m_Normals[m_Primitives[i]]), m_Color),
			VertexPosNormCol(ToXMFLOAT3(pVertices[m_Primitives[i + 1]].pos), ToXMFLOAT3(m_Normals[m_Primitives[i + 1]]), m_Color),
			VertexPosNormCol(ToXMFLOAT3(pVertices[m_Primitives[i + 2]].pos), ToXMFLOAT3(m_Normals[m_Primitives[i + 2]]), m_Color),
			VertexPosNormCol(ToXMFLOAT3(pVertices[m_Primitives[i + 3]].pos), ToXMFLOAT3(m_Normals[m_Primitives[i + 3]]), m_Color)
		);
	}

	m_pMeshComp->UpdateBuffer();
	pData->unlock();
}

void Cloth::SetExternalAcceleration(float x, float y, float z) 
{
	m_ExternalAcceleration.x = x;
	m_ExternalAcceleration.y = y;
	m_ExternalAcceleration.z = z;
	m_pCloth->setExternalAcceleration(m_ExternalAcceleration);
}

void Cloth::SetExternalAcceleration(float force)
{
	m_ExternalAcceleration = { force, force, force };
	m_pCloth->setExternalAcceleration(m_ExternalAcceleration);
}

void Cloth::CalculateNormals(float normalizer)
{
	m_Normals.resize(m_VertexCount);

	for (size_t i = 0; i < UINT(m_IndexCount); i += 4) 
	{
		PxVec3 p1 = m_Particles[m_Primitives[i]].pos;
		PxVec3 p2 = m_Particles[m_Primitives[i + 1]].pos;
		PxVec3 p3 = m_Particles[m_Primitives[i + 2]].pos;
		PxVec3 n = (p2 - p1).cross(p3 - p1);

		m_Normals[m_Primitives[i]] += n / normalizer;
		m_Normals[m_Primitives[i + 1]] += n / normalizer;
		m_Normals[m_Primitives[i + 2]] += n / normalizer;
		m_Normals[m_Primitives[i + 3]] += n / normalizer;
	}
}
