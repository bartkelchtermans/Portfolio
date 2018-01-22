#pragma once
// #include "stdafx.h"

class Cloth: public GameObject
{
public:
	Cloth(float width, float length, int resolution, XMFLOAT4 color, float externalForce);
	~Cloth();	

	void SetExternalAcceleration(float x, float y, float z);
	void SetExternalAcceleration(float force);

protected:
	void Initialize(const GameContext& gameContext);
	void Update(const GameContext& gameContext);

private:
	// Methods
	void CalculateNormals(float n);

	// Variables
	PxCloth* m_pCloth = nullptr;
	MeshDrawComponent* m_pMeshComp = nullptr;

	float m_Width;
	float m_Length;

	int m_Resolution;
	int m_VertexCount;
	int m_IndexCount;

	XMFLOAT4 m_Color;
	PxVec3 m_ExternalAcceleration;

	std::vector<PxClothParticle> m_Particles;
	std::vector<PxVec3> m_Normals;
	std::vector<int> m_Primitives;
};
