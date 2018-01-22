#pragma once
#include "stdafx.h"
#include "Cloth.h"

struct GameContext;
class CameraComponent;
class RigidBodyComponent;
class CubePrefab;
class SpherePrefab;
class ParticleEmitterComponent;
class PhysxProxy;

class ShipPrefab : public GameObject
{
public:
	// Constructor & Destructor
	ShipPrefab::ShipPrefab(bool direction);
	virtual ~ShipPrefab(void);

	virtual void Update(const GameContext& gameContext);
	virtual void Initialize(const GameContext& gameContext);
	virtual void PostInitialize(const GameContext& gameContext);

	GameObject* GetController() { return m_pGalleon; }
	bool CanBeControlled() { return m_CanBeControlled; }
	bool GetDirection() { return m_Direction; }
	std::vector<SpherePrefab*> GetCannonballs() { return m_Cannonballs; }
	void DoDamage(int part, float damage);

	// Return health
	int GetTotalHealth();
	float GetHullHealth() { return m_HullHealth / 1.5f; };
	float GetBalloonHealth(byte balloon);

	void SetAIActive(bool active, ShipPrefab* player1, ShipPrefab* player2);
	void PassPhysxProxy(PhysxProxy* proxy) { physxProxy = proxy; };
	void Shoot();
	float GetReloadTime() { return m_CannonTimer; };

private:

	// Hull
	GameObject* m_pGalleon;
	RigidBodyComponent* m_pGalleonRigidBody;

	// Balloons
	SpherePrefab* m_pBalloonBig;
	RigidBodyComponent* m_pBalloonBRigidBody;
	SpherePrefab* m_pBalloonMid;
	RigidBodyComponent* m_pBalloonMRigidBody;
	SpherePrefab* m_pBalloonSmall;
	RigidBodyComponent* m_pBalloonSRigidBody;

	// Joints
	PxFixedJoint* m_pJointBalloonBig;
	PxFixedJoint* m_pJointBalloonMid;
	PxFixedJoint* m_pJointBalloonSmall;
	PxFixedJoint* m_pJointTrigger;

	// Trigger
	RigidBodyComponent* m_pTriggerRigidBody;
	CubePrefab* m_pShipTrigger;

	// Cannon
	CubePrefab* m_CannonBottom;
	CubePrefab* m_CannonTop;
	RigidBodyComponent* m_CannonballRigidbody;

	// Cannonballs
	std::vector<SpherePrefab*> m_Cannonballs;

	// Physx
	PhysxProxy* physxProxy;

	// AI
	ShipPrefab* m_Target;
	ShipPrefab* m_Player1;
	ShipPrefab* m_Player2;

	// Ship variables
	bool m_CanBeControlled = false;
	bool m_Direction = true;
	bool m_AI = false;
		// Movement
	float horMomentum;
	float verMomentum;
	float momentumCap{ 0.1f };
		// Floating effect
	float m_FloatingCounterSign = 0.005f;
	float m_FloatingCounter = 1;
		// Health
	float m_HullHealth = 150;
	float m_BalloonBHealth = 50;
	float m_BalloonMHealth = 35;
	float m_BalloonSHealth = 15;
		// Status
	bool m_JointBig = true;
	bool m_JointMid = true;
	bool m_JointSmall = true;
	bool m_CannonShot = false;
	bool m_IsControlled = false;
		// Cannons
	float m_CannonballDamage = 4;
	float m_CannonPower = 400;
	float m_CannonTimer = 3.5f;
	float m_AICannonTimer = 3.5f;

	float m_ShootVector;

	bool m_ParticlesVisible = false;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ShipPrefab(const ShipPrefab& yRef);									
	ShipPrefab& operator=(const ShipPrefab& yRef);
};

