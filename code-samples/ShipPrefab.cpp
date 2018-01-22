#include "ShipPrefab.h"

ShipPrefab::ShipPrefab(bool direction):
	m_pGalleon(nullptr),
	m_pGalleonRigidBody(nullptr), 
	m_pBalloonBig(nullptr),
	m_pBalloonBRigidBody(nullptr),
	m_pBalloonMid(nullptr),
	m_pBalloonMRigidBody(nullptr),
	m_pBalloonSmall(nullptr),
	m_pBalloonSRigidBody(nullptr),
	m_pJointBalloonBig(nullptr),
	m_pJointBalloonMid(nullptr),
	m_pJointBalloonSmall(nullptr),
	m_pJointTrigger(nullptr),
	m_pTriggerRigidBody(nullptr),
	m_pShipTrigger(nullptr),
	m_CannonBottom(nullptr),
	m_CannonTop(nullptr),
	m_Direction(direction)
{
}

ShipPrefab::~ShipPrefab(void)
{
	if (m_pJointBalloonBig != nullptr && m_JointBig && m_pJointBalloonBig->isReleasable()) m_pJointBalloonBig->release();
	if (m_pJointBalloonMid != nullptr && m_JointMid && m_pJointBalloonMid->isReleasable()) m_pJointBalloonMid->release();
	if (m_pJointBalloonSmall != nullptr && m_JointSmall && m_pJointBalloonSmall->isReleasable()) m_pJointBalloonSmall->release();
	if (m_pJointTrigger != nullptr && m_pJointTrigger->isReleasable()) m_pJointTrigger->release();
}

void ShipPrefab::Initialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);
	// PhysX
	auto physX = PhysxManager::GetInstance()->GetPhysics();
	auto defaultMaterial = physX->createMaterial(.5f, .5f, .1f);

	// Hull
	m_pGalleon = new GameObject();
	m_pGalleon->AddComponent(new ModelComponent(L"Resources/Meshes/Galleon.ovm"));
	m_pGalleon->GetComponent<ModelComponent>()->SetMaterial(0);

	m_pGalleonRigidBody = new RigidBodyComponent();
	m_pGalleonRigidBody->SetKinematic(true);
	m_pGalleon->AddComponent(m_pGalleonRigidBody);

	auto triangleMesh = ContentManager::Load<PxTriangleMesh>(L"Resources/Meshes/Galleon.ovpt");
	std::shared_ptr<PxGeometry>triangleGeom(new PxTriangleMeshGeometry(triangleMesh));
	ColliderComponent* collider = new ColliderComponent(triangleGeom, *defaultMaterial, PxTransform(PxIdentity));

	m_pGalleon->AddComponent(collider);

	m_pGalleon->GetTransform()->Translate(XMFLOAT3(0, -5, 0));
	m_pGalleon->SetTag(L"HULL");
	AddChild(m_pGalleon);

	// Balloon: Big
	float colorValue = (rand() % 30 + 190.f) / 256.f;
	m_pBalloonBig = new SpherePrefab(30, 20, XMFLOAT4(colorValue, colorValue, colorValue, 1));

	m_pBalloonBRigidBody = new RigidBodyComponent();
	m_pBalloonBig->AddComponent(m_pBalloonBRigidBody);

	std::shared_ptr<PxGeometry> spheregeom(new PxSphereGeometry(30));
	m_pBalloonBig->AddComponent(new ColliderComponent(spheregeom, *defaultMaterial, PxTransform(PxQuat(XM_PIDIV2, PxVec3(0, 0, 1)))));
	m_pBalloonBig->SetTag(L"BALLOONB");
	AddChild(m_pBalloonBig);

	// Balloon: Mid
	colorValue = (rand() % 30 + 190.f) / 256.f;
	m_pBalloonMid = new SpherePrefab(18, 20, XMFLOAT4(colorValue, colorValue, colorValue, 1));

	m_pBalloonMRigidBody = new RigidBodyComponent();
	m_pBalloonMid->AddComponent(m_pBalloonMRigidBody);

	std::shared_ptr<PxGeometry> spheregeom2(new PxSphereGeometry(18));
	m_pBalloonMid->AddComponent(new ColliderComponent(spheregeom2, *defaultMaterial, PxTransform(PxQuat(XM_PIDIV2, PxVec3(0, 0, 1)))));
	m_pBalloonMid->SetTag(L"BALLOONM");
	AddChild(m_pBalloonMid);

	// Balloon: Small
	colorValue = (rand() % 30 + 190.f) / 256.f;
	m_pBalloonSmall = new SpherePrefab(10, 20, XMFLOAT4(colorValue, colorValue, colorValue, 1));

	m_pBalloonSRigidBody = new RigidBodyComponent();
	m_pBalloonSmall->AddComponent(m_pBalloonSRigidBody);

	std::shared_ptr<PxGeometry> spheregeom3(new PxSphereGeometry(10));
	m_pBalloonSmall->AddComponent(new ColliderComponent(spheregeom3, *defaultMaterial, PxTransform(PxQuat(XM_PIDIV2, PxVec3(0, 0, 1)))));
	m_pBalloonSmall->SetTag(L"BALLOONS");
	AddChild(m_pBalloonSmall);

	// Trigger to see if it can be controlled
	m_pShipTrigger = new CubePrefab(0, 0, 0);
	m_pShipTrigger->GetTransform()->Translate(XMFLOAT3(this->GetTransform()->GetPosition().x, this->GetTransform()->GetPosition().y - 5, this->GetTransform()->GetPosition().z));

	m_pTriggerRigidBody = new RigidBodyComponent();
	m_pShipTrigger->AddComponent(m_pTriggerRigidBody);

	std::shared_ptr<PxGeometry> cubegeom(new PxBoxGeometry(8, 45, 5));
	m_pShipTrigger->AddComponent(new ColliderComponent(cubegeom, *defaultMaterial, PxTransform(PxQuat(XM_PIDIV2, PxVec3(0, 0, 1)))));
	m_pShipTrigger->GetComponent<ColliderComponent>()->EnableTrigger(true);

	m_pShipTrigger->SetOnTriggerCallBack([this](GameObject *trigger, GameObject *receiver, GameObject::TriggerAction action)
	{
		UNREFERENCED_PARAMETER(trigger);

		if (action == TriggerAction::ENTER)
		{
			if (receiver->GetTag() == L"Player")
			{
				m_CanBeControlled = true;
			}
		}
		if (action == TriggerAction::LEAVE)
		{
			if (receiver->GetTag() == L"Player")
			{
				m_CanBeControlled = false;
			}
		}
	});

	AddChild(m_pShipTrigger);

	// Cannon
	m_CannonBottom = new CubePrefab(3, 8, 3, XMFLOAT4(0.2f, 0.2f, 0.2f, 1));
	m_CannonBottom->GetTransform()->Translate(XMFLOAT3(-36, 11, 0));

	m_CannonTop = new CubePrefab(10, 3, 3, XMFLOAT4(0.2f, 0.2f, 0.2f, 1));
	m_CannonTop->GetTransform()->Translate(XMFLOAT3(-36, 11, 0));
	m_CannonTop->AddComponent(new RigidBodyComponent());
	m_CannonTop->GetComponent<RigidBodyComponent>()->SetConstraint(RigidBodyConstraintFlag::RotX, true);
	m_CannonTop->GetComponent<RigidBodyComponent>()->SetConstraint(RigidBodyConstraintFlag::RotY, true);
	std::shared_ptr<PxGeometry> cannoncubegeom(new PxBoxGeometry(1.5f, 5, 1.5f));
	m_CannonTop->AddComponent(new ColliderComponent(cannoncubegeom, *defaultMaterial, PxTransform(PxQuat(XM_PIDIV2, PxVec3(0, 0, 1)))));

	AddChild(m_CannonBottom);
	AddChild(m_CannonTop);

	// Cloth
	AddChild(new Cloth(8.f, 12.f, 10, (m_Direction) ? XMFLOAT4(0.2f, 0.55f, 0.2f, 1.f) : XMFLOAT4(0.55f, 0.2f, 0.2f, 1.f), 0.f));

	// Floating
	srand(rand());
	int intingCounter = rand() % 3;
	m_FloatingCounter = (float)intingCounter;
}

void ShipPrefab::PostInitialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);

	// Joints
	m_pJointBalloonBig = PxFixedJointCreate(*PhysxManager::GetInstance()->GetPhysics(), m_pGalleonRigidBody->GetPxRigidActor(), PxTransform(PxVec3(25.f, 57.f, 0.f)), m_pBalloonBRigidBody->GetPxRigidActor(), PxTransform(PxVec3(0.f, 0.f, 0.f)));
	m_pJointBalloonMid = PxFixedJointCreate(*PhysxManager::GetInstance()->GetPhysics(), m_pGalleonRigidBody->GetPxRigidActor(), PxTransform(PxVec3(-30.f, 39.f, 0.f)), m_pBalloonMRigidBody->GetPxRigidActor(), PxTransform(PxVec3(0.f, 0.f, 0.f)));
	m_pJointBalloonSmall = PxFixedJointCreate(*PhysxManager::GetInstance()->GetPhysics(), m_pGalleonRigidBody->GetPxRigidActor(), PxTransform(PxVec3(-22.f, 71.f, 0.f)), m_pBalloonSRigidBody->GetPxRigidActor(), PxTransform(PxVec3(0.f, 0.f, 0.f)));
	m_pJointTrigger = PxFixedJointCreate(*PhysxManager::GetInstance()->GetPhysics(), m_pGalleonRigidBody->GetPxRigidActor(), PxTransform(PxVec3(2, 10, 0.f)), m_pTriggerRigidBody->GetPxRigidActor(), PxTransform(PxVec3(0.f, 0.f, 0.f)));
}

void ShipPrefab::Update(const GameContext& gameContext)
{
	float fi = gameContext.pGameTime->GetFPS() * gameContext.pGameTime->GetElapsed();

	XMFLOAT3 galleonPos = m_pGalleon->GetTransform()->GetPosition();

	// Calculating upwards force of the balloons
	float gravitySum = 0.3f;
	if (m_JointBig) gravitySum -= 0.15f;
	if (m_JointMid) gravitySum -= 0.1f;
	if (m_JointSmall) gravitySum -= 0.05f;

	// Making the ship float with sinusoidal movement
	float floatingValue = 7 * std::sin(m_FloatingCounter) * m_FloatingCounterSign * fi;

	m_FloatingCounter += m_FloatingCounterSign;

	if (m_FloatingCounter > XM_PI || m_FloatingCounter < 0)
	{
		m_FloatingCounterSign *= -1;
	}
	
	// Applying elevation forces on the balloons
	m_pBalloonBRigidBody->AddForce(PxVec3(0, 400, 0));
	m_pBalloonMRigidBody->AddForce(PxVec3(0, 300, 0));
	m_pBalloonSRigidBody->AddForce(PxVec3(0, 150, 0));

	// Player control
	if (gameContext.pInput->IsActionTriggered(5))
	{
		if (!m_IsControlled && m_CanBeControlled)
		{
			m_IsControlled = true;
		}
		else if (m_IsControlled)
		{
			m_IsControlled = false;
		}
	}

	// Handle input
	if (m_IsControlled)
	{
		// Movement
		XMFLOAT3 newPos = { 0,0,0 };

		if (gameContext.pInput->IsActionTriggered(1)) if (verMomentum < momentumCap) verMomentum += 0.01f * fi;
		if (gameContext.pInput->IsActionTriggered(2)) if (verMomentum > -momentumCap)verMomentum -= 0.01f * fi;
		if (gameContext.pInput->IsActionTriggered(3)) if (horMomentum > -momentumCap) horMomentum -= 0.01f * fi;
		if (gameContext.pInput->IsActionTriggered(4)) if (horMomentum < momentumCap) horMomentum += 0.01f * fi;

		newPos.y += verMomentum;
		newPos.x += horMomentum;

		if (newPos.x > 10) newPos.x = 50;

		if (!gameContext.pInput->IsActionTriggered(1) || !gameContext.pInput->IsActionTriggered(2) || !gameContext.pInput->IsActionTriggered(3) || !gameContext.pInput->IsActionTriggered(4))
		{
			(verMomentum > 0) ? verMomentum -= 0.0005f * fi : verMomentum += 0.0005f * fi;
			(horMomentum > 0) ? horMomentum -= 0.0005f * fi : horMomentum += 0.0005f * fi;
		}

		m_pGalleon->GetTransform()->Translate(galleonPos.x + newPos.x, galleonPos.y + newPos.y + floatingValue - gravitySum, galleonPos.z + newPos.z);

		// Calculate mouse position in NDC
		XMFLOAT2 MouseNDC;
		float halfWidth = OverlordGame::GetGameSettings().Window.Width / 2.f;
		float halfHeight = OverlordGame::GetGameSettings().Window.Height / 2.f;

		MouseNDC.x = (gameContext.pInput->GetMousePosition().x - halfWidth) / halfWidth;
		MouseNDC.y = (halfHeight - gameContext.pInput->GetMousePosition().y) / halfHeight;

		// Calculate cannon position in NDC
		XMFLOAT4 CannonNDC = XMFLOAT4(m_CannonTop->GetTransform()->GetPosition().x, m_CannonTop->GetTransform()->GetPosition().y, 1, 0);
		XMFLOAT4X4 viewProj = gameContext.pCamera->GetViewProjection();

		XMVECTOR vecCannonPos = XMVector3TransformCoord(XMLoadFloat4(&CannonNDC), XMLoadFloat4x4(&viewProj));
		XMStoreFloat4(&CannonNDC, vecCannonPos);

		// Math to calculate angle between mouse NDC and cannon NDC
		float deltaY = CannonNDC.y - MouseNDC.y;
		float deltaX = CannonNDC.x - MouseNDC.x;
		float angle = atan2(deltaY, deltaX) * 180 / XM_PI;

		if (angle < 22 && angle > -45) m_CannonTop->GetTransform()->Rotate(0, 0, angle, true);
		else
		{
		if (angle > 22) m_CannonTop->GetTransform()->Rotate(0, 0, 22, true);
		if (angle < -45) m_CannonTop->GetTransform()->Rotate(0, 0, -45, true);
		}

		// Shooting
		if (gameContext.pInput->IsMouseButtonDown(1))
		{
			if ((!m_CannonShot) && (m_CannonTimer >= 3.5f))
			{
				Shoot();
			}
		}
		else
		{
		m_CannonShot = false;
		}

		if (!m_CannonShot)
		{
			m_CannonTimer += 0.01f;
		}
	}
	else
	{
		if (m_AI)
		{
			// AI logic
				// Recheck target
			XMFLOAT3 player1Pos = m_Player1->GetController()->GetTransform()->GetPosition();
			XMFLOAT3 player2Pos = m_Player2->GetController()->GetTransform()->GetPosition();

			float pythagorean1 = sqrt(abs(abs(galleonPos.x * galleonPos.x) - abs(player1Pos.x * player1Pos.x)) + abs(abs(galleonPos.y * galleonPos.y) - abs(player1Pos.y * player1Pos.y)));
			float pythagorean2 = sqrt(abs(abs(galleonPos.x * galleonPos.x) - abs(player2Pos.x * player2Pos.x)) + abs(abs(galleonPos.y * galleonPos.y) - abs(player2Pos.y * player2Pos.y)));

			(pythagorean1 < pythagorean2) ? m_Target = m_Player1 : m_Target = m_Player2;

			// Calculate distance
			XMFLOAT3 currPos = this->GetController()->GetTransform()->GetPosition();
			XMFLOAT3 targetPos = m_Target->GetController()->GetTransform()->GetPosition();
			float distance = sqrt(abs(abs(currPos.x * currPos.x) - abs(targetPos.x * targetPos.x)) + abs(abs(currPos.y * currPos.y) - abs(targetPos.y * targetPos.y)));

				// Level vertically - top
			if (targetPos.y - 50 > currPos.y)	
			{
				PxVec3 rayStart = PxVec3(currPos.x + 40, currPos.y + 40, currPos.z);
				PxVec3 rayDirection = PxVec3(currPos.x, currPos.y + 150, currPos.z);

				PxRaycastBuffer hit;
				physxProxy->Raycast(rayStart, rayDirection.getNormalized(), 50.f, hit);

				if (!hit.hasBlock) // Can go up
				{
					currPos.y += 0.08f * fi;
				}
			}

				// Level vertically - bottom
			if (targetPos.y + 50 < currPos.y)
			{
				PxVec3 rayStart = PxVec3(currPos.x + 40, currPos.y - 40, currPos.z);
				PxVec3 rayDirection = PxVec3(currPos.x, currPos.y - 150, currPos.z);

				PxRaycastBuffer hit;
				physxProxy->Raycast(rayStart, rayDirection.getNormalized(), 50.f, hit);

				if (!hit.hasBlock) // Can go down
				{
					currPos.y -= 0.08f * fi;
				}
			}

				// Move towards target
			if (targetPos.x - 150 > currPos.x)
			{
				PxVec3 rayStart = PxVec3(currPos.x + 150, currPos.y, currPos.z);
				PxVec3 rayDirection = PxVec3(currPos.x + 150, currPos.y, currPos.z);

				PxRaycastBuffer hit;
				physxProxy->Raycast(rayStart, rayDirection.getNormalized(), 100.f, hit);

				if (!hit.hasBlock) // Can go forward
				{
					currPos.x += 0.08f * fi;
				}
			}

			//Y
			currPos.y += floatingValue - gravitySum;

			this->GetController()->GetTransform()->Translate(currPos);

			// Aiming
				// Calculate cannon position in NDC
			XMFLOAT4 CannonNDC = XMFLOAT4(m_CannonTop->GetTransform()->GetPosition().x, m_CannonTop->GetTransform()->GetPosition().y, 1, 0);
			XMFLOAT4X4 viewProj = gameContext.pCamera->GetViewProjection();

			XMVECTOR vecCannonPos = XMVector3TransformCoord(XMLoadFloat4(&CannonNDC), XMLoadFloat4x4(&viewProj));
			XMStoreFloat4(&CannonNDC, vecCannonPos);

				// Calculate target position in NDC
			XMFLOAT4 TargetNDC = XMFLOAT4(m_Target->GetController()->GetTransform()->GetPosition().x, m_Target->GetController()->GetTransform()->GetPosition().y, 1, 0);

			XMVECTOR vecTargetPos = XMVector3TransformCoord(XMLoadFloat4(&TargetNDC), XMLoadFloat4x4(&viewProj));
			XMStoreFloat4(&TargetNDC, vecTargetPos);

				// Math to calculate angle between target NDC and cannon NDC
			float deltaY = CannonNDC.y - TargetNDC.y;
			float deltaX = CannonNDC.x - TargetNDC.x;
			float test = atan2(deltaY, deltaX);
			m_ShootVector = test;
			float angle = atan2(deltaY, deltaX) * 180 / XM_PI;

			m_CannonTop->GetTransform()->Rotate(0, 0, angle, true);

			// Shooting
			//srand(gameContext.pGameTime->GetTotal());
			int randomization = rand() % 2 + 1;
			if (distance < 150)
			{
				if (m_AICannonTimer > 3.5f) { Shoot(); m_AICannonTimer = 0; }
				m_AICannonTimer += (float)randomization / 100.f;
			}
		}
	}

	// Translate the cannon with the ship
	if (m_Direction)
	{
		m_CannonBottom->GetTransform()->Translate(XMFLOAT3(this->GetController()->GetTransform()->GetPosition().x - 36, this->GetController()->GetTransform()->GetPosition().y + 11, 0));
		m_CannonTop->GetTransform()->Translate(XMFLOAT3(this->GetController()->GetTransform()->GetPosition().x - 36, this->GetController()->GetTransform()->GetPosition().y + 15, 0));
	}
	else
	{
		m_CannonBottom->GetTransform()->Translate(XMFLOAT3(this->GetController()->GetTransform()->GetPosition().x + 36, this->GetController()->GetTransform()->GetPosition().y + 11, 0));
		m_CannonTop->GetTransform()->Translate(XMFLOAT3(this->GetController()->GetTransform()->GetPosition().x + 36, this->GetController()->GetTransform()->GetPosition().y + 15, 0));
	}

	// Translate cloth with the ship
	if (m_Direction)
	{
		GetChild<Cloth>()->GetTransform()->Translate(XMFLOAT3(this->GetController()->GetTransform()->GetPosition().x + 40, this->GetController()->GetTransform()->GetPosition().y - 10, 0));
	}
	else
	{
		GetChild<Cloth>()->GetTransform()->Translate(XMFLOAT3(this->GetController()->GetTransform()->GetPosition().x - 40, this->GetController()->GetTransform()->GetPosition().y - 10, 0));
	}

	// Release joints on no hull health left
	if ((m_HullHealth <= 0))// && m_JointBig && m_JointMid && m_JointSmall)
	{
		if (m_JointBig) m_pJointBalloonBig->release(); //m_pJointBalloonBig->isReleasable() && 
		m_JointBig = false;
		if (m_JointMid) m_pJointBalloonMid->release();
		m_JointMid = false;
		if (m_JointSmall) m_pJointBalloonSmall->release();
		m_JointSmall = false;
	}
	else
	{
		// Release big balloon on no health left
		if ((m_BalloonBHealth <= 0) && (m_JointBig))
		{
			m_pJointBalloonBig->release();
			m_JointBig = false;
		}

		// Release medium balloon on no health left
		if ((m_BalloonMHealth <= 0) && (m_JointMid))
		{
			m_pJointBalloonMid->release();
			m_JointMid = false;
		}

		// Release small balloon on no health left
		if ((m_BalloonSHealth <= 0) && (m_JointSmall))
		{
			m_pJointBalloonSmall->release();
			m_JointSmall = false;
		}
	}

	for (size_t i = 0; i < m_Cannonballs.size(); i++)
	{
		if (m_Cannonballs[i] != nullptr) m_Cannonballs[i]->GetComponent<RigidBodyComponent>()->AddForce(PxVec3(0, -100, 0));
	}
}

void ShipPrefab::DoDamage(int part, float damage)
{
	// 1: hull, 2: big balloon, 3: medium balloon, 4: small balloon
	switch (part)
	{
	case 1:
		m_HullHealth -= damage;
		break;
	case 2:
		m_BalloonBHealth -= damage;
		break;
	case 3:
		m_BalloonMHealth -= damage;
		break;
	case 4:
		m_BalloonSHealth -= damage;
		break;
	}
}

int ShipPrefab::GetTotalHealth()
{
	float returnValue = m_HullHealth + m_BalloonBHealth + m_BalloonMHealth + m_BalloonSHealth;
	return ((int) returnValue / 2);
}

float ShipPrefab::GetBalloonHealth(byte balloon) 
{
	if (balloon == 0) return (m_BalloonBHealth / 5) * 10;
	if (balloon == 1) return (m_BalloonMHealth / 3.5f) * 10;
	return (m_BalloonSHealth / 1.5f) * 10;
}

void ShipPrefab::SetAIActive(bool active, ShipPrefab* player1, ShipPrefab* player2)
{
	XMFLOAT3 currPos = m_pGalleon->GetTransform()->GetPosition();
	XMFLOAT3 player1Pos = player1->GetController()->GetTransform()->GetPosition();
	XMFLOAT3 player2Pos = player2->GetController()->GetTransform()->GetPosition();

	float pythagorean1 = sqrt(abs(abs(currPos.x * currPos.x) - abs(player1Pos.x * player1Pos.x)) + abs(abs(currPos.y * currPos.y) - abs(player1Pos.y * player1Pos.y)));
	float pythagorean2 = sqrt(abs(abs(currPos.x * currPos.x) - abs(player2Pos.x * player2Pos.x)) + abs(abs(currPos.y * currPos.y) - abs(player2Pos.y * player2Pos.y)));

	(pythagorean1 < pythagorean2) ? m_Target = player1 : m_Target = player2;

	m_Player1 = player1;
	m_Player2 = player2;

	m_AI = active;
}

void ShipPrefab::Shoot()
{
	auto defaultMaterial = PhysxManager::GetInstance()->GetPhysics()->createMaterial(.5f, .5f, .1f);

	m_Cannonballs.push_back(new SpherePrefab(2, 10, XMFLOAT4(0.05f, 0.05f, 0.05f, 1.f)));
	m_CannonballRigidbody = new RigidBodyComponent;
	m_CannonballRigidbody->SetConstraint(RigidBodyConstraintFlag::TransZ, true);
	m_CannonballRigidbody->SetKinematic(false);

	m_Cannonballs[m_Cannonballs.size() - 1]->AddComponent(m_CannonballRigidbody);

	std::shared_ptr<PxGeometry> cannonballgeomtrigger(new PxSphereGeometry(2.1f));
	m_Cannonballs[m_Cannonballs.size() - 1]->AddComponent(new ColliderComponent(cannonballgeomtrigger, *defaultMaterial, PxTransform(PxQuat(XM_PIDIV2, PxVec3(0, 0, 1)))));
	m_Cannonballs[m_Cannonballs.size() - 1]->GetComponent<ColliderComponent>()->EnableTrigger(true);

	std::shared_ptr<PxGeometry> cannonballgeom(new PxSphereGeometry(2));
	m_Cannonballs[m_Cannonballs.size() - 1]->AddComponent(new ColliderComponent(cannonballgeom, *defaultMaterial, PxTransform(PxQuat(XM_PIDIV2, PxVec3(0, 0, 1)))));

	m_Cannonballs[m_Cannonballs.size() - 1]->SetOnTriggerCallBack([this](GameObject *trigger, GameObject *receiver, GameObject::TriggerAction action)
	{
		if (action == TriggerAction::ENTER)
		{
			if (receiver->GetTag() == L"HULL" && trigger->GetTag() != L"INACTIVE")
			{
				static_cast<ShipPrefab*>(receiver->GetParent())->DoDamage(1, m_CannonballDamage);
				trigger->SetTag(L"INACTIVE");
			}
			if (receiver->GetTag() == L"BALLOONB")
			{
				static_cast<ShipPrefab*>(receiver->GetParent())->DoDamage(2, m_CannonballDamage);
				trigger->SetTag(L"INACTIVE");
			}
			if (receiver->GetTag() == L"BALLOONM")
			{
				static_cast<ShipPrefab*>(receiver->GetParent())->DoDamage(3, m_CannonballDamage);
				trigger->SetTag(L"INACTIVE");
			}
			if (receiver->GetTag() == L"BALLOONS")
			{
				static_cast<ShipPrefab*>(receiver->GetParent())->DoDamage(4, m_CannonballDamage);
				trigger->SetTag(L"INACTIVE");
			}
		}
	});

	AddChild(m_Cannonballs[m_Cannonballs.size() - 1]);

	double cannonCosine = cos(m_CannonTop->GetTransform()->GetRotation().z);
	double cannonSine = sin(m_CannonTop->GetTransform()->GetRotation().z);
	
	if (m_Direction)
	{
		m_Cannonballs[m_Cannonballs.size() - 1]->GetTransform()->Translate((float)(m_CannonTop->GetTransform()->GetPosition().x - cannonCosine * 5), (float)(m_CannonTop->GetTransform()->GetPosition().y - cannonSine * 5), 0.f);
		m_CannonballRigidbody->AddForce(PxVec3((physx::PxReal) -(cannonCosine * m_CannonPower), (physx::PxReal) -(cannonSine * m_CannonPower * 2), 0), PxForceMode::eIMPULSE);
	}
	else
	{

		cannonCosine = cos(m_CannonTop->GetTransform()->GetRotation().z + XM_PI);
		cannonSine = sin(m_CannonTop->GetTransform()->GetRotation().z + XM_PI);
		
		m_Cannonballs[m_Cannonballs.size() - 1]->GetTransform()->Translate((float)(m_CannonTop->GetTransform()->GetPosition().x + 10), (float)(m_CannonTop->GetTransform()->GetPosition().y + cannonSine * 5), 0.f);

		float value;
		if (m_ShootVector < 0)
		{
			value = XM_PI + m_ShootVector;
		}
		else
		{
			value = XM_PI - m_ShootVector;
			value *= -1;
		}
		
		m_CannonballRigidbody->AddForce(PxVec3((physx::PxReal) (1 * m_CannonPower), (physx::PxReal) (value * 100), 0), PxForceMode::eIMPULSE);
	}

	m_CannonTimer = 0;
	m_CannonShot = true;
}