#include "Player.h"
#include "../Core/Audio.hpp"
#include "../Core/Game.h"
#include "../Core/Input.h"
#include "../Core/InputMulti.h"
#include "../BackEnd/BackEnd.h"
#include "../API/OpenGL/GL_assetManager.h"
#include "../Core/Scene.h"
#include "../Renderer/TextBlitter.h"
#include "../Renderer/RendererUtil.hpp"
#include "../Common.h"
#include "../Util.hpp"
#include "AnimatedGameObject.h"
#include "Config.hpp"
#include "../EngineState.hpp"
#include "../Timer.hpp"

int Player::GetCurrentWeaponClipAmmo() {
	if (_currentWeaponIndex == Weapon::GLOCK) {
		return _inventory.glockAmmo.clip;
    }
    if (_currentWeaponIndex == Weapon::AKS74U) {
        return _inventory.aks74uAmmo.clip;
    }
    if (_currentWeaponIndex == Weapon::SHOTGUN) {
        return _inventory.shotgunAmmo.clip;
    }
	return 0;
}

int Player::GetCurrentWeaponTotalAmmo() {
	if (_currentWeaponIndex == Weapon::GLOCK) {
		return _inventory.glockAmmo.total;
    }
    if (_currentWeaponIndex == Weapon::AKS74U) {
        return _inventory.aks74uAmmo.total;
    }
    if (_currentWeaponIndex == Weapon::SHOTGUN) {
        return _inventory.shotgunAmmo.total;
    }
	return 0;
}

Player::Player() {

    Respawn();

    //_characterModel.PrintMeshNames();

	_characterModel.SetSkinnedModel("UniSexGuyScaled");
    _characterModel.SetMeshMaterialByMeshName("CC_Base_Body", "UniSexGuyBody");
    _characterModel.SetMeshMaterialByMeshName("CC_Base_Eye", "UniSexGuyBody");
    _characterModel.SetMeshMaterialByMeshName("Biker_Jeans", "UniSexGuyJeans");
    _characterModel.SetMeshMaterialByMeshName("CC_Base_Eye", "UniSexGuyEyes");
    _characterModel.SetMeshMaterialByMeshName("Glock", "Glock");
    _characterModel.SetMeshMaterialByMeshName("SM_Knife_01", "Knife");
    _characterModel.SetMeshMaterialByMeshName("Shotgun_Mesh", "Shotgun");
    _characterModel.SetMeshMaterialByMeshIndex(13, "UniSexGuyHead");
    _characterModel.SetMeshMaterialByMeshIndex(14, "UniSexGuyLashes");
    _characterModel.EnableBlendingByMeshIndex(14);
    _characterModel.SetMeshMaterialByMeshName("FrontSight_low", "AKS74U_0");
    _characterModel.SetMeshMaterialByMeshName("Receiver_low", "AKS74U_1");
    _characterModel.SetMeshMaterialByMeshName("BoltCarrier_low", "AKS74U_1");
    _characterModel.SetMeshMaterialByMeshName("SafetySwitch_low", "AKS74U_0");
    _characterModel.SetMeshMaterialByMeshName("MagRelease_low", "AKS74U_0");
    _characterModel.SetMeshMaterialByMeshName("Pistol_low", "AKS74U_2");
    _characterModel.SetMeshMaterialByMeshName("Trigger_low", "AKS74U_1");
    _characterModel.SetMeshMaterialByMeshName("Magazine_Housing_low", "AKS74U_3");
    _characterModel.SetMeshMaterialByMeshName("BarrelTip_low", "AKS74U_4");

	//_shadowMap.Init();

	CreateCharacterController(_position);
	CreateItemPickupOverlapShape();
}

void Player::SetWeapon(Weapon weapon) {
	if (_currentWeaponIndex != weapon) {
		_currentWeaponIndex = (int)weapon;
		_needsAmmoReloaded = false;
		_weaponAction = WeaponAction::DRAW_BEGIN;
	}
}

PxSweepCallback* CreateSweepBuffer() {
	return new PxSweepBuffer;
}

bool Player::MuzzleFlashIsRequired() {	
	return (_muzzleFlashCounter > 0);
}

glm::mat4 Player::GetWeaponSwayMatrix() {
	return _weaponSwayMatrix;
}

//void Player::WipeYVelocityToZeroIfHeadHitCeiling() {
	/*
	glm::vec3 rayOrigin = _position + glm::vec3(0, 1.5, 0);
	glm::vec3 rayDirection = glm::vec3(0, 1, 0);
	PxReal rayLength = 0.5f;
	PxScene* scene = Physics::GetScene();
	PxVec3 origin = PxVec3(rayOrigin.x, rayOrigin.y, rayOrigin.z);
	PxVec3 unitDir = PxVec3(rayDirection.x, rayDirection.y, rayDirection.z);
	PxRaycastBuffer hit;
	const PxHitFlags outputFlags = PxHitFlag::ePOSITION;
	PxQueryFilterData filterData = PxQueryFilterData();
	filterData.data.word0 = RaycastGroup::RAYCAST_ENABLED;
	filterData.data.word2 = CollisionGroup::ENVIROMENT_OBSTACLE;

	if (scene->raycast(origin, unitDir, rayLength, hit, outputFlags, filterData)) {
		//_yVelocity = 0;
		//std::cout << "HIT HEAD " << Util::Vec3ToString(rayOrigin) << "\n";
	}*/
//}

void Player::ShowPickUpText(std::string text) {
	_pickUpText = text;
	_pickUpTextTimer = Config::pickup_text_time;
}



void Player::PickUpGlock() {
    if (_weaponInventory[Weapon::GLOCK] == false) {
        ShowPickUpText("PICKED UP GLOCK");
        Audio::PlayAudio("ItemPickUp.wav", 1.0f);
        _weaponInventory[Weapon::GLOCK] = true;
        _inventory.glockAmmo.clip = GLOCK_CLIP_SIZE;
        _inventory.glockAmmo.total = GLOCK_CLIP_SIZE * 2;
    }
    else {
        PickUpGlockAmmo();
    }
}

void Player::PickUpAKS74U() {
    if (_weaponInventory[Weapon::AKS74U] == false) {
        ShowPickUpText("PICKED UP AKS74U");
        Audio::PlayAudio("ItemPickUp.wav", 1.0f);
        _weaponInventory[Weapon::AKS74U] = true;
        _inventory.aks74uAmmo.clip = AKS74U_MAG_SIZE;
        _inventory.aks74uAmmo.total = AKS74U_MAG_SIZE * 2;
        if (_currentWeaponIndex == GLOCK || _currentWeaponIndex == KNIFE) {
            SetWeapon(Weapon::AKS74U);
        }
    }
    else {
        PickUpAKS74UAmmo();
    }
}

void Player::PickUpShotgun() {
    if (_weaponInventory[Weapon::SHOTGUN] == false) {
        ShowPickUpText("PICKED UP SHOTGUN");
        Audio::PlayAudio("ItemPickUp.wav", 1.0f);
        _weaponInventory[Weapon::SHOTGUN] = true;
        _inventory.shotgunAmmo.clip = SHOTGUN_AMMO_SIZE;
        _inventory.shotgunAmmo.total += SHOTGUN_AMMO_SIZE * 2;
        if (_currentWeaponIndex == KNIFE || _currentWeaponIndex == GLOCK) {
            SetWeapon(Weapon::SHOTGUN);
        }
    }
    else {
        PickUpShotgunAmmo();
    }
}


void Player::PickUpAKS74UAmmo() {
    ShowPickUpText("PICKED UP SOME AMMO");
    Audio::PlayAudio("ItemPickUp.wav", 1.0f);
    _inventory.aks74uAmmo.total += AKS74U_MAG_SIZE * 3;
}

void Player::PickUpShotgunAmmo() {
    ShowPickUpText("PICKED UP SOME AMMO");
    Audio::PlayAudio("ItemPickUp.wav", 1.0f);
    _inventory.shotgunAmmo.total += 25; // make this a define when you see this next or else.
}

void Player::PickUpGlockAmmo() {
    ShowPickUpText("PICKED UP GLOCK AMMO");
    Audio::PlayAudio("ItemPickUp.wav", 1.0f);
    _inventory.glockAmmo.total += 50; // make this a define when you see this next or else.
}


void Player::CheckForItemPickOverlaps() {

	if (_ignoreControl) {
		return;
	}

	const PxGeometry& overlapShape = _itemPickupOverlapShape->getGeometry();
	const PxTransform shapePose(_characterController->getActor()->getGlobalPose());

	OverlapReport overlapReport = Physics::OverlapTest(overlapShape, shapePose, CollisionGroup::GENERIC_BOUNCEABLE);

	if (overlapReport.hits.size()) {
		for (auto* hit : overlapReport.hits) {
			if (hit->userData) {

				PhysicsObjectData* physicsObjectData = (PhysicsObjectData*)hit->userData;
				PhysicsObjectType physicsObjectType = physicsObjectData->type;
				GameObject* parent = (GameObject*)physicsObjectData->parent;

				if (physicsObjectType == GAME_OBJECT) {
                    // Weapon pickups
                    if (!parent->IsCollected() && parent->GetPickUpType() == PickUpType::AKS74U) {
                        PickUpAKS74U();
                        parent->PickUp();
                    }
                    if (!parent->IsCollected() && parent->GetPickUpType() == PickUpType::GLOCK) {
                        PickUpGlock();
                        parent->PickUp();
                    }
                    if (!parent->IsCollected() && parent->GetPickUpType() == PickUpType::SHOTGUN) {
                        PickUpShotgun();
                        parent->PickUp();
                        // Think about this brother. Next time you see it of course. Not now.
                        // Think about this brother. Next time you see it of course. Not now.
                        // Think about this brother. Next time you see it of course. Not now.
                        if (parent->_respawns) {
                            parent->PutRigidBodyToSleep();
                        }
                    }
                    if (!parent->IsCollected() && parent->GetPickUpType() == PickUpType::AKS74U_SCOPE) {
                        GiveAKS74UScope();
                        parent->PickUp();
                        // Think about this brother. Next time you see it of course. Not now.
                        // Think about this brother. Next time you see it of course. Not now.
                        // Think about this brother. Next time you see it of course. Not now.
                        if (parent->_respawns) {
                            parent->PutRigidBodyToSleep();
                        }
                    }
                    if (!parent->IsCollected() && parent->GetPickUpType() == PickUpType::GLOCK_AMMO) {

                        GameObject* topDrawer = Scene::GetGameObjectByName("TopDraw");

                        auto ammoPose = parent->collisionRigidBody.pxRigidBody->getGlobalPose();
                        auto ammoY = ammoPose.p.y;

                        if (ammoY > 0.4f && topDrawer->_openState == OpenState::OPEN) {
                            PickUpGlockAmmo();
                            parent->PickUp();
                            parent->PutRigidBodyToSleep();
                        }
                        else if (ammoY <= 0.4f) {
                            PickUpGlockAmmo();
                            parent->PickUp();
                            parent->PutRigidBodyToSleep();
                        }

                       /* GameObject* topDrawer = Scene::GetGameObjectByName("TopDraw");
                        //GameObject* wholeDrawers = Scene::GetGameObjectByName("SmallDrawerTop");

                        const PxGeometry& overlapShape = parent->_raycastShape->getGeometry();

                        PxVec3 ammoPos = parent->_raycastBody->getGlobalPose().p;
                        PxVec3 offset = PxVec3(0, -0.05, 0);
                        PxTransform transform(ammoPos + offset);

                        auto report = Physics::OverlapTest(overlapShape, transform, CollisionGroup::ENVIROMENT_OBSTACLE);

                        if (report.HitsFound()) {
                            // ammo collied with geometry;
                            std::cout << "ammo colliding with geo\n";
                        }
                        */
                        //if (Scene::GetGameObjectByName("SmallDrawerTop")->GetOpenState() == OpenState::OPEN) {

                            
                              //  parent->GetWorldPosition();

                 
                       // }
                    }
				}
			}
			else {
				// std::cout << "no user data found on ray hit\n";
			}
		}
	}
	else {
		// std::cout << "no overlap bro\n";
	}
}

void Player::UpdateRagdoll() {

    _characterModel.UpdateRenderItems();

    // Collision only if dead
    if (_isDead) {
      //  _characterModel._ragdoll.EnableCollision();
    }
    else {
        // BROKEN
        // BROKEN
        // BROKEN
        // BROKEN
       // _characterModel._ragdoll.DisableCollision();
    }
    // Updated user data pointer
    for (RigidComponent& rigid : _characterModel._ragdoll._rigidComponents) {
        PhysicsObjectData* physicsObjectData = (PhysicsObjectData*)rigid.pxRigidBody->userData;
        physicsObjectData->parent = this;
    }
}

void Player::Update(float deltaTime) {

    if (Input::KeyPressed(HELL_KEY_G)) {
        _hasGlockSilencer = !_hasGlockSilencer;
        Audio::PlayAudio("Glock_Equip.wav", 0.5f);

        if (_hasGlockSilencer) {
            _firstPersonWeapon.EnableDrawingForMeshByMeshName("Glock_silencer");
        }
        else {
            _firstPersonWeapon.DisableDrawingForMeshByMeshName("Glock_silencer");
        }
    }

    /*
    if (maleHands) {
        if (meshName == "SK_FPSArms_Female.001" ||
            meshName == "SK_FPSArms_Female" ||
            meshName == "SK_FPSArms_Female_LOD0.001") {
            continue;
        }
    }
    else {
        if (meshName == "manniquen1_2.001" ||
            meshName == "manniquen1_2") {
            continue;
        }
        */

    if (Input::KeyPressed(HELL_KEY_J)) {
        RespawnAtCurrentPosition();
    }

    // Damage color
    _damageColorTimer += deltaTime * 0.5f;
    _damageColorTimer = std::min(1.0f, _damageColorTimer);

    // Death timer
    if (_isDead) {
        _timeSinceDeath += deltaTime;
        _ignoreControl = true;
    }
    else {
        _timeSinceDeath = 0;
    }

    // Pressed Respawn
    bool autoRespawn = false;
    //autoRespawn = true;
    //autoRespawn = true;
    if (_isDead && _timeSinceDeath > 3.25) {
        if (PressedFire() ||
            PressedReload() ||
            PressedCrouch() ||
            PressedInteract() ||
            PresingJump() ||
            PressedNextWeapon() ||
            autoRespawn)
        {
            Respawn();
            Audio::PlayAudio("RE_Beep.wav", 0.75);
        }
    }

    UpdateRagdoll();


    // Take damage outside
    _isOutside = true;
    for (Floor& floor : Scene::_floors) {
        if (floor.PointIsAboveThisFloor(_position)) {
            _isOutside = false;
            break;
        }
    }
    if (_isOutside) {
        _outsideDamageTimer += deltaTime;
        _outsideDamageAudioTimer += deltaTime;
    }
    else {
        _outsideDamageAudioTimer = 0.84f;
    }
    if (_outsideDamageAudioTimer > 0.85f && !_isDead) {
        _outsideDamageAudioTimer = 0.0f;
        Audio::PlayAudio("Pain.wav", 1.0f);
    }
    if (_outsideDamageTimer > 0.15f) {
        _outsideDamageTimer = 0.0f;
        _health -= 1;
    }
    if (!_isDead && _isOutside && _health <= 0) {
        Kill();
        _health = 0;

        /*
        glm::vec3 deathPosition = { GetFeetPosition().x, 0.0, GetFeetPosition().z };
        AnimatedGameObject* dyingGuy = Scene::GetAnimatedGameObjectByName("DyingGuy");
        dyingGuy->SetRotationY(_rotation.y + HELL_PI);
        dyingGuy->SetPosition(deathPosition);
        dyingGuy->PlayAnimation("DyingGuy_Death", 1.0f);*/
    }
    // If you're dead, reset the 
    if (_isDead) {
        _outsideDamageTimer = 0;
    }

	CheckForItemPickOverlaps();
    			
	if (_pickUpTextTimer > 0) {
		_pickUpTextTimer -= deltaTime;
	}
	else {
		_pickUpTextTimer = 0;
		_pickUpText = "";
	}

	// Muzzle flash timer
	_muzzleFlashCounter -= deltaTime;
	_muzzleFlashCounter = std::max(_muzzleFlashCounter, 0.0f);

	// Mouselook
	if (EngineState::GetEngineMode() == GAME) {
		if (!_ignoreControl && BackEnd::WindowHasFocus()) {
			float mouseSensitivity = 0.002f;
            if (InADS()) {
                mouseSensitivity = 0.001f;
            }
            float xOffset = (float)InputMulti::GetMouseXOffset(_mouseIndex);
            float yOffset = (float)InputMulti::GetMouseYOffset(_mouseIndex);
			_rotation.x += -yOffset * mouseSensitivity;
			_rotation.y += -xOffset * mouseSensitivity;
			_rotation.x = std::min(_rotation.x, 1.5f);
			_rotation.x = std::max(_rotation.x, -1.5f);
		}
	}

    float amt = 0.02f;
    if (Input::KeyDown(HELL_KEY_MINUS)) {
        _viewHeightStanding -= amt;
    }
    if (Input::KeyDown(HELL_KEY_EQUAL)) {
        _viewHeightStanding += amt;
    }
    /*if (Input::KeyDown(HELL_KEY_8)) {
        _viewHeightStanding -= amt * 100;
    }
    if (Input::KeyDown(HELL_KEY_9)) {
        _viewHeightStanding += amt * 100;
    }*/

	// Crouching
	bool crouching = false;
	if (!_ignoreControl && PressingCrouch()) {
		crouching = true;
	}

	// View height
	float viewHeightTarget = crouching ? _viewHeightCrouching : _viewHeightStanding;
	_currentViewHeight = Util::FInterpTo(_currentViewHeight, viewHeightTarget, deltaTime, _crouchDownSpeed);
    
    float _breatheAmplitude = 0.0004f;
    float _breatheFrequency = 5;
    float _headBobAmplitude = 0.008;
    float _headBobFrequency = 17.0f;

    if (crouching) {
        _breatheFrequency *= 0.5f;
        _headBobFrequency *= 0.5f;
    }
    
    // Breathe bob
	static float totalTime;
    totalTime += deltaTime / 2.25f;// 0.0075f;
	Transform breatheTransform;
	breatheTransform.position.x = cos(totalTime * _breatheFrequency) * _breatheAmplitude * 1;
	breatheTransform.position.y = sin(totalTime * _breatheFrequency) * _breatheAmplitude * 2;

	// Head bob
	Transform headBobTransform;
	if (_isMoving) {
		headBobTransform.position.x = cos(totalTime * _headBobFrequency) * _headBobAmplitude * 1;
		headBobTransform.position.y = sin(totalTime * _headBobFrequency) * _headBobAmplitude * 2;
	}

	// View matrix
	Transform camTransform;
	camTransform.position = _position + glm::vec3(0, _currentViewHeight, 0);
	camTransform.rotation = _rotation;

    if (!_isDead) {
        _viewMatrix = glm::inverse(headBobTransform.to_mat4() * breatheTransform.to_mat4() * camTransform.to_mat4());
    }
    // Kill cam
    else {

        for (RigidComponent& rigidComponent : _characterModel._ragdoll._rigidComponents) {
            if (rigidComponent.name == "rMarker_CC_Base_Head") {
                PxMat44 globalPose = rigidComponent.pxRigidBody->getGlobalPose();
                _viewMatrix = glm::inverse(Util::PxMat44ToGlmMat4(globalPose));
                break;
            }
        }
    }

	_inverseViewMatrix = glm::inverse(_viewMatrix);
	_right = glm::vec3(_inverseViewMatrix[0]);
	_up = glm::vec3(_inverseViewMatrix[1]);
	_forward = glm::vec3(_inverseViewMatrix[2]);
	_movementVector = glm::normalize(glm::vec3(_forward.x, 0, _forward.z));
	_viewPos = _inverseViewMatrix[3];

    // WSAD movement
    _isMoving = false;
    if (!_ignoreControl) {
	    if (PressingWalkForward()) {
            _displacement -= _movementVector;
		    _isMoving = true;
	    }
	    if (PressingWalkBackward()) {
            _displacement += _movementVector;
		    _isMoving = true;
	    }
	    if (PressingWalkLeft()) {
            _displacement -= _right;
		    _isMoving = true;
	    }
	    if (PressingWalkRight()) {
            _displacement += _right;
		    _isMoving = true;
	    }
    }
	


    float targetSpeed = crouching ? _crouchingSpeed : _walkingSpeed;
    float interSpeed = 18.0f;
    if (!_isMoving) {
        targetSpeed = 0.0f;
        interSpeed = 22.0f;
    }
    _currentSpeed = Util::FInterpTo(_currentSpeed, targetSpeed, deltaTime, interSpeed);

    // Normalize displacement vector and include player speed
	float len = length(_displacement);
	if (len != 0.0) {
        _displacement = (_displacement / len) * _currentSpeed * deltaTime;
	}

	// Jump
    if (PresingJump() && !_ignoreControl && _isGrounded) {
        _yVelocity = 4.75f; // magic value for jump strength
        _yVelocity = 4.9f; // magic value for jump strength (had to change cause you could no longer jump thru window after fixing character controller height bug)
		_isGrounded = false;
	}

	// Gravity		
	if (_isGrounded) {
		_yVelocity = -0.1f; // can't be 0, or the _isGrounded check next frame will fail
	}
	else {
		float gravity = 15.75f; // 9.8 feels like the moon
		_yVelocity -= gravity * deltaTime;
	}
	float yDisplacement = _yVelocity * deltaTime;

	// Move PhysX character controller
	PxFilterData filterData;
	filterData.word0 = 0;
	filterData.word1 = CollisionGroup::ENVIROMENT_OBSTACLE;	// Things to collide with	
	PxControllerFilters data;
	data.mFilterData = &filterData;
	PxF32 minDist = 0.001f; 
    float fixedDeltaTime = (1.0f / 60.0f);
	_characterController->move(PxVec3(_displacement.x, yDisplacement, _displacement.z), minDist, fixedDeltaTime, data);
	_position = Util::PxVec3toGlmVec3(_characterController->getFootPosition());

	
	// Footstep audio
	if (!_ignoreControl) {
		if (!_isMoving)
			_footstepAudioTimer = 0;
		else {
			if (_isMoving && _footstepAudioTimer == 0) {

                // Audio
                const std::vector<const char*> footstepFilenames = {
                    "player_step_1.wav",
                    "player_step_2.wav",
                    "player_step_3.wav",
                    "player_step_4.wav",
                };
                int random = rand() % 4;
                Audio::PlayAudio(footstepFilenames[random], 0.5f);
			}
			float timerIncrement = crouching ? deltaTime * 0.75f : deltaTime;
			_footstepAudioTimer += timerIncrement;
			if (_footstepAudioTimer > _footstepAudioLoopLength) {
				_footstepAudioTimer = 0;
			}
		}
	}
	// Next weapon
	if (!_ignoreControl && PressedNextWeapon()) {

		_needsToDropAKMag = false;

		Audio::PlayAudio("Glock_Equip.wav", 0.5f);
		_needsAmmoReloaded = false;

		bool foundNextWeapon = false;
		while (!foundNextWeapon) {
			_currentWeaponIndex++;
			if (_currentWeaponIndex == Weapon::WEAPON_COUNT) {
				_currentWeaponIndex = 0;
			}
			if (_weaponInventory[_currentWeaponIndex]) {
				foundNextWeapon = true;
			}
			_weaponAction = WeaponAction::DRAW_BEGIN;
		}
	}

	if (EngineState::GetEngineMode() == GAME) {
		UpdateFirstPersonWeaponLogicAndAnimations(deltaTime);
	}

	if (_muzzleFlashTimer >= 0) {
		_muzzleFlashTimer += deltaTime * 20;
	}

	// Interact
	if (!_ignoreControl) {
        //PxU32 raycastFlags = RaycastGroup::RAYCAST_ENABLED | ~RaycastGroup::PLAYER_1_RAGDOLL | ~RaycastGroup::PLAYER_2_RAGDOLL;
        //_cameraRayResult = Util::CastPhysXRay(GetViewPos(), GetCameraForward() * glm::vec3(-1), 100, raycastFlags);
        _cameraRayResult = Util::CastPhysXRay(GetViewPos(), GetCameraForward() * glm::vec3(-1), 100, _interactFlags);
		Interact();
	}

    // Character model animation
    if (!_isDead) {
        if (_currentWeaponIndex == KNIFE) {
            if (_isMoving) {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_Knife_Walk", 1.0f);
            }
            else {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_Knife_Idle", 1.0f);
            }
            if (crouching) {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_Knife_Crouch", 1.0f);
            }
        }
        if (_currentWeaponIndex == GLOCK) {
            if (_isMoving) {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_Glock_Walk", 1.0f);
            }
            else {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_Glock_Idle", 1.0f);
            }
            if (crouching) {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_Glock_Crouch", 1.0f);
            }
        }
        if (_currentWeaponIndex == AKS74U) {
            if (_isMoving) {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_AKS74U_Walk", 1.0f);
            }
            else {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_AKS74U_Idle", 1.0f);
            }
            if (crouching) {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_AKS74U_Crouch", 1.0f);
            }
        }
        if (_currentWeaponIndex == SHOTGUN) {
            if (_isMoving) {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_Shotgun_Walk", 1.0f);
            }
            else {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_Shotgun_Idle", 1.0f);
            }
            if (crouching) {
                _characterModel.PlayAndLoopAnimation("UnisexGuy_Shotgun_Crouch", 1.0f);
            }
        }
        _characterModel.SetPosition(GetFeetPosition());// +glm::vec3(0.0f, 0.1f, 0.0f));
        _characterModel.Update(deltaTime);
        _characterModel.SetRotationY(_rotation.y + HELL_PI);
    }
    else {
        // THIS IS WHERE YOU SKIN THE MESH TO THE RAGDOLL BUT IT IS CURRENTLY BROKEN
        // THIS IS WHERE YOU SKIN THE MESH TO THE RAGDOLL BUT IT IS CURRENTLY BROKEN
        // THIS IS WHERE YOU SKIN THE MESH TO THE RAGDOLL BUT IT IS CURRENTLY BROKEN
        // THIS IS WHERE YOU SKIN THE MESH TO THE RAGDOLL BUT IT IS CURRENTLY BROKEN
        // THIS IS WHERE YOU SKIN THE MESH TO THE RAGDOLL BUT IT IS CURRENTLY BROKEN
        // THIS IS WHERE YOU SKIN THE MESH TO THE RAGDOLL BUT IT IS CURRENTLY BROKEN
        // THIS IS WHERE YOU SKIN THE MESH TO THE RAGDOLL BUT IT IS CURRENTLY BROKEN
        // THIS IS WHERE YOU SKIN THE MESH TO THE RAGDOLL BUT IT IS CURRENTLY BROKEN
        // THIS IS WHERE YOU SKIN THE MESH TO THE RAGDOLL BUT IT IS CURRENTLY BROKEN
        // 
        _characterModel.UpdateBoneTransformsFromRagdoll();
    }

	// Debug casing spawn
	/*if (!_ignoreControl) {
		if (Input::KeyDown(HELL_KEY_T) && GetCurrentWeaponIndex() == GLOCK) {
			SpawnGlockCasing();
        }
        if (Input::KeyDown(HELL_KEY_T) && GetCurrentWeaponIndex() == AKS74U) {
            SpawnAKS74UCasing();
        }
        if (Input::KeyDown(HELL_KEY_T) && GetCurrentWeaponIndex() == SHOTGUN) {
            SpawnShotgunShell();
        }
	}*/

	/*
	// Check for game object pick up collision
	for (GameObject & gameObject: Scene::_gameObjects) {

		if (gameObject.IsCollectable() && !gameObject.IsCollected()) {

			glm::vec3 worldPositionOfPickUp = glm::vec4(gameObject._transform.position, 1.0f);
			float allowedPickupMinDistance = 0.6f;
			glm::vec3 a = glm::vec3(worldPositionOfPickUp.x, 0, worldPositionOfPickUp.z);
			glm::vec3 b = glm::vec3(GetFeetPosition().x, 0, GetFeetPosition().z);
			float distanceToPickUp = glm::distance(a, b);

			if (distanceToPickUp < allowedPickupMinDistance) {
				if (gameObject.GetPickUpType() == PickUpType::AKS74U) {
					PickUpAKS74U();
				}
				gameObject.PickUp();
			}
		}		
	}*/

	// Check for pick up "collision"
	for (PickUp& pickUp : Scene::_pickUps) {
		if (pickUp.pickedUp) {
			continue;
		}
		glm::mat4 parentMatrix = glm::mat4(1);
		if (pickUp.parentGameObjectName != "") {
			GameObject* parentgameObject = Scene::GetGameObjectByName(pickUp.parentGameObjectName);
			if (parentgameObject->GetOpenState() == OpenState::CLOSED ||
				parentgameObject->GetOpenState() == OpenState::OPENING) {
				continue;
			}
			parentMatrix = parentgameObject->GetModelMatrix();
		}
		glm::vec3 worldPositionOfPickUp = parentMatrix * glm::vec4(pickUp.position, 1.0f);
		float allowedPickupMinDistance = 0.4f;
		glm::vec3 a = glm::vec3(worldPositionOfPickUp.x, 0, worldPositionOfPickUp.z);
		glm::vec3 b = glm::vec3(GetFeetPosition().x, 0, GetFeetPosition().z);
		float distanceToPickUp = glm::distance(a, b);

		if (distanceToPickUp < allowedPickupMinDistance) {
			pickUp.pickedUp = true;
			_inventory.glockAmmo.total += 50.0f;
			_pickUpText = "PICKED UP GLOCK AMMO";
			_pickUpTextTimer = 2.0f;
			Audio::PlayAudio("ItemPickUp.wav", 1.0f);
		}
	}

    if (_isDead) {
        _health = 0;
    }




    finalImageColorTint = glm::vec3(1, 1, 1);
    finalImageContrast = 1;

    if (IsAlive() && _isOutside) {
        finalImageColorTint = RED;
        finalImageColorTint.g = _outsideDamageAudioTimer;
        finalImageColorTint.b = _outsideDamageAudioTimer;
    }

    if (IsAlive() && _damageColorTimer < 1.0f) {
        finalImageColorTint.g = _damageColorTimer + 0.75;
        finalImageColorTint.b = _damageColorTimer + 0.75;
        finalImageColorTint.g = std::min(finalImageColorTint.g, 1.0f);
        finalImageColorTint.b = std::min(finalImageColorTint.b, 1.0f);
    }

    if (IsDead()) {

        // Make it red
        if (_timeSinceDeath > 0) {
            finalImageColorTint.g *= 0.25f;
            finalImageColorTint.b *= 0.25f;
            finalImageContrast = 1.2f;
        }
        // Darken it after 3 seconds
        float waitTime = 3;
        if (_timeSinceDeath > waitTime) {
            float val = (_timeSinceDeath - waitTime) * 10;
            finalImageColorTint.r -= val;
        }
    }


}


void Player::ForceSetViewMatrix(glm::mat4 viewMatrix) {
    _viewMatrix = viewMatrix;
    _inverseViewMatrix = glm::inverse(_viewMatrix);
    _right = glm::vec3(_inverseViewMatrix[0]);
    _up = glm::vec3(_inverseViewMatrix[1]);
    _forward = glm::vec3(_inverseViewMatrix[2]);
    _movementVector = glm::normalize(glm::vec3(_forward.x, 0, _forward.z));
    _viewPos = _inverseViewMatrix[3];
}

glm::mat4 Player::GetViewMatrix() {
	return  glm::mat4(glm::mat3(_firstPersonWeapon._cameraMatrix)) * _viewMatrix;;
}

glm::mat4 Player::GetInverseViewMatrix() {
	return _inverseViewMatrix;
}

glm::vec3 Player::GetViewPos() {
	return _viewPos;
}

glm::vec3 Player::GetViewRotation() {
	return _rotation;
}


glm::vec3 Player::GetFeetPosition() {
	return _position;
}

glm::vec3 Player::GetCameraRight() {
	return _right;
}

glm::vec3 Player::GetCameraForward() {
	return _forward;
}

glm::vec3 Player::GetCameraUp() {
	return _up;
}

bool Player::IsMoving() {
	return _isMoving;
}

int Player::GetCurrentWeaponIndex() {
	return _currentWeaponIndex;
}

void Player::Interact() {

	if (PressedInteract()) {
		if (_cameraRayResult.physicsObjectType == DOOR) {
			//std::cout << "you pressed interact on a door \n";
			Door* door = (Door*)(_cameraRayResult.parent);
			if (!door->IsInteractable(GetFeetPosition())) {
				return;
			}
			door->Interact();
		}
		if (_cameraRayResult.physicsObjectType == GAME_OBJECT) {
			GameObject* gameObject = (GameObject*)(_cameraRayResult.parent);
			if (gameObject && !gameObject->IsInteractable()) {
				return;
			}
            if (gameObject) {
                gameObject->Interact();
            }
		}
	}
}

void Player::SetPosition(glm::vec3 position) {
    _characterController->setFootPosition(PxExtendedVec3(position.x, position.y, position.z));
}

void Player::Respawn() {

    _isDead = false;
    _ignoreControl = false;
    _characterModel._ragdoll.DisableCollision();

    int index = Util::RandomInt(0, Scene::_spawnPoints.size() - 1);
    SpawnPoint& spawnPoint = Scene::_spawnPoints[index];

    // Check you didn't just spawn on another player
    for (int i = 0; i < Game::GetPlayerCount(); i++) {
        Player* otherPlayer = Game::GetPlayerByIndex(i);
        if (this != otherPlayer) {
            float distanceToOtherPlayer = glm::distance(spawnPoint.position, otherPlayer->_position);
            if (distanceToOtherPlayer < 1.0f) {
                Respawn();
                return;
            }
        }
    }

	if (_weaponInventory.empty()) {
		_weaponInventory.resize(Weapon::WEAPON_COUNT);
	}

    _hasAKS74UScope = false;
    _health = 100;

	// Loadout on spawn
	_weaponInventory[Weapon::KNIFE] = true;
	_weaponInventory[Weapon::GLOCK] = true;
	_weaponInventory[Weapon::SHOTGUN] = false;
	_weaponInventory[Weapon::AKS74U] = false;
	_weaponInventory[Weapon::MP7] = false;

	SetWeapon(Weapon::GLOCK);
	_weaponAction = SPAWNING;

    if (_characterController) {
        PxExtendedVec3 globalPose = PxExtendedVec3(spawnPoint.position.x, spawnPoint.position.y, spawnPoint.position.z);
        _characterController->setFootPosition(globalPose);
    }
    _position = spawnPoint.position;

	_rotation = spawnPoint.rotation;
	_inventory.glockAmmo.clip = GLOCK_CLIP_SIZE;
    _inventory.glockAmmo.total = 20;
    _inventory.aks74uAmmo.clip = 0;
    _inventory.aks74uAmmo.total = 0;
    _inventory.shotgunAmmo.clip = 0;
    _inventory.shotgunAmmo.total = 0;
    SetGlockAnimatedModelSettings();

	Audio::PlayAudio("Glock_Equip.wav", 0.5f);
}

void Player::SetGlockAnimatedModelSettings() {

    _firstPersonWeapon.SetName("Glock");
    _firstPersonWeapon.SetSkinnedModel("Glock");
    if (!_firstPersonWeapon._skinnedModel) {
        return; // remove this once you have Vulkan loading shit correctly
    }
    _firstPersonWeapon.PlayAnimation("Glock_Spawn", 1.0f);
    _firstPersonWeapon.SetAllMeshMaterials("Glock");
    _firstPersonWeapon.SetMeshMaterialByMeshName("manniquen1_2.001", "Hands");
    _firstPersonWeapon.SetMeshMaterialByMeshName("manniquen1_2", "Hands");
    _firstPersonWeapon.SetMeshMaterialByMeshName("SK_FPSArms_Female.001", "FemaleArms");
    _firstPersonWeapon.SetMeshMaterialByMeshName("SK_FPSArms_Female", "FemaleArms");
    _firstPersonWeapon.SetMeshMaterialByMeshName("Glock_silencer", "Silencer");
    _firstPersonWeapon.SetMeshMaterialByMeshName("RedDotSight", "RedDotSight");
    _firstPersonWeapon.SetMeshMaterialByMeshName("RedDotSightGlass", "RedDotSight");
    _firstPersonWeapon.SetMeshToRenderAsGlassByMeshIndex("RedDotSightGlass");
    _firstPersonWeapon.SetMeshEmissiveColorTextureByMeshName("RedDotSight", "RedDotSight_EmissiveColor");

    _firstPersonWeapon.EnableDrawingForAllMesh();
    if (!_hasGlockSilencer) {
        _firstPersonWeapon.DisableDrawingForMeshByMeshName("Glock_silencer");
    }
    _firstPersonWeapon.DisableDrawingForMeshByMeshName("SK_FPSArms_Female");
    _firstPersonWeapon.DisableDrawingForMeshByMeshName("SK_FPSArms_Female.001");
}

void Player::RespawnAtCurrentPosition() {
    _isDead = false;
    _ignoreControl = false;
    _characterModel._ragdoll.DisableCollision();
    if (_weaponInventory.empty()) {
        _weaponInventory.resize(Weapon::WEAPON_COUNT);
    }
    _hasAKS74UScope = false;
    _health = 100;
    _weaponInventory[Weapon::KNIFE] = true;
    _weaponInventory[Weapon::GLOCK] = true;
    _weaponInventory[Weapon::SHOTGUN] = false;
    _weaponInventory[Weapon::AKS74U] = false;
    _weaponInventory[Weapon::MP7] = false;
    SetWeapon(Weapon::GLOCK);
    _weaponAction = SPAWNING;
    _inventory.glockAmmo.clip = GLOCK_CLIP_SIZE;
    _inventory.glockAmmo.total = 20;
    _inventory.aks74uAmmo.clip = 0;
    _inventory.aks74uAmmo.total = 0;
    _inventory.shotgunAmmo.clip = 0;
    _inventory.shotgunAmmo.total = 0;
    SetGlockAnimatedModelSettings();
    Audio::PlayAudio("Glock_Equip.wav", 0.5f);
}

bool Player::CanFire() {

    if (_ignoreControl || _isDead) {
        return false;
    }
    if (_currentWeaponIndex == Weapon::KNIFE) {
		return true;
	}
	if (_currentWeaponIndex == Weapon::GLOCK) {
		return (
			_weaponAction == IDLE ||
			_weaponAction == DRAWING && _firstPersonWeapon.AnimationIsPastPercentage(50.0f) ||
			_weaponAction == FIRE && _firstPersonWeapon.AnimationIsPastPercentage(25.0f) ||
			_weaponAction == RELOAD && _firstPersonWeapon.AnimationIsPastPercentage(80.0f) ||
			_weaponAction == RELOAD_FROM_EMPTY && _firstPersonWeapon.AnimationIsPastPercentage(80.0f) ||
			_weaponAction == SPAWNING && _firstPersonWeapon.AnimationIsPastPercentage(5.0f)
		);
	}
	if (_currentWeaponIndex == Weapon::SHOTGUN) {
    return (
        _weaponAction == IDLE ||
        _weaponAction == DRAWING && _firstPersonWeapon.AnimationIsPastPercentage(50.0f) ||
        _weaponAction == FIRE && _firstPersonWeapon.AnimationIsPastPercentage(50.0f) ||
        _weaponAction == RELOAD_SHOTGUN_BEGIN ||
        _weaponAction == RELOAD_SHOTGUN_END ||
        _weaponAction == RELOAD_SHOTGUN_SINGLE_SHELL ||
        _weaponAction == RELOAD_SHOTGUN_DOUBLE_SHELL ||
        _weaponAction == SPAWNING && _firstPersonWeapon.AnimationIsPastPercentage(5.0f)
        );
	}
	if (_currentWeaponIndex == Weapon::AKS74U) {
		return (
			_weaponAction == IDLE ||
			_weaponAction == DRAWING && _firstPersonWeapon.AnimationIsPastPercentage(75.0f) ||
			_weaponAction == FIRE && _firstPersonWeapon.AnimationIsPastPercentage(22.5f) ||
			_weaponAction == RELOAD && _firstPersonWeapon.AnimationIsPastPercentage(80.0f) ||
			_weaponAction == RELOAD_FROM_EMPTY && _firstPersonWeapon.AnimationIsPastPercentage(95.0f) ||

            _weaponAction == ADS_IDLE ||
            _weaponAction == ADS_FIRE && _firstPersonWeapon.AnimationIsPastPercentage(22.0f)
		);
	}
	if (_currentWeaponIndex == Weapon::MP7) {
		// TO DO
		return true;
	}
}


bool Player::InADS() {
    if (_weaponAction == ADS_IN ||
        _weaponAction == ADS_OUT ||
        _weaponAction == ADS_IDLE ||
        _weaponAction == ADS_FIRE)
        return true;
    else {
        return false;
    }
}


bool Player::CanReload() {

	if (_ignoreControl) {
		return false;
	}
	if (_currentWeaponIndex == Weapon::GLOCK) {
		return (_inventory.glockAmmo.total > 0 && _inventory.glockAmmo.clip < GLOCK_CLIP_SIZE && _weaponAction != RELOAD && _weaponAction != RELOAD_FROM_EMPTY);
	}
	if (_currentWeaponIndex == Weapon::SHOTGUN) {
        if (_weaponAction == FIRE && !_firstPersonWeapon.AnimationIsPastPercentage(50.0f)) {
            return false;
        }
        return (_inventory.shotgunAmmo.total > 0 && _inventory.shotgunAmmo.clip < SHOTGUN_AMMO_SIZE && _weaponAction != RELOAD_SHOTGUN_BEGIN && _weaponAction != RELOAD_SHOTGUN_END && _weaponAction != RELOAD_SHOTGUN_SINGLE_SHELL && _weaponAction != RELOAD_SHOTGUN_DOUBLE_SHELL);
	}
	if (_currentWeaponIndex == Weapon::AKS74U) {
		return (_inventory.aks74uAmmo.total > 0 && _inventory.aks74uAmmo.clip < AKS74U_MAG_SIZE && _weaponAction != RELOAD && _weaponAction != RELOAD_FROM_EMPTY);
	}
	if (_currentWeaponIndex == Weapon::MP7) {
		// TO DO
		return true;
	}
}

void Player::UpdateFirstPersonWeaponLogicAndAnimations(float deltaTime) {

    if (_weaponAction == SPAWNING) {
        _characterModel.EnableDrawingForAllMesh();
        HideKnifeMesh();
        HideShotgunMesh();
        HideAKS74UMesh();
    }

	// Switching weapon? Well change all the shit you need to then
	if (_weaponAction == DRAW_BEGIN) {
		if (_currentWeaponIndex == Weapon::KNIFE) {
			_firstPersonWeapon.SetName("Knife");
			_firstPersonWeapon.SetSkinnedModel("Knife");
			_firstPersonWeapon.SetMeshMaterialByMeshName("SM_Knife_01", "Knife");
            _characterModel.EnableDrawingForAllMesh();
            HideGlockMesh();
            HideShotgunMesh();
            HideAKS74UMesh();
		}
		else if (_currentWeaponIndex == Weapon::GLOCK) {
            SetGlockAnimatedModelSettings();
            HideKnifeMesh();
            HideShotgunMesh();
            HideAKS74UMesh();
		}
		else if (_currentWeaponIndex == Weapon::AKS74U) {
			_firstPersonWeapon.SetName("AKS74U");
			_firstPersonWeapon.SetSkinnedModel("AKS74U");
			_firstPersonWeapon.SetMeshMaterialByMeshIndex(2, "AKS74U_3");
			_firstPersonWeapon.SetMeshMaterialByMeshIndex(3, "AKS74U_3"); // possibly incorrect. this is the follower
			_firstPersonWeapon.SetMeshMaterialByMeshIndex(4, "AKS74U_1");
			_firstPersonWeapon.SetMeshMaterialByMeshIndex(5, "AKS74U_4");
			_firstPersonWeapon.SetMeshMaterialByMeshIndex(6, "AKS74U_0");
			_firstPersonWeapon.SetMeshMaterialByMeshIndex(7, "AKS74U_2");
			_firstPersonWeapon.SetMeshMaterialByMeshIndex(8, "AKS74U_1");  // Bolt_low. Possibly wrong
            _firstPersonWeapon.SetMeshMaterialByMeshIndex(9, "AKS74U_3"); // possibly incorrect.
            _characterModel.EnableDrawingForAllMesh();
            HideKnifeMesh();
            HideGlockMesh();
            HideShotgunMesh();

        }
        else if (_currentWeaponIndex == Weapon::SHOTGUN) {
            _firstPersonWeapon.SetName("Shotgun");
            _firstPersonWeapon.SetSkinnedModel("Shotgun");
            _firstPersonWeapon.SetAllMeshMaterials("Shotgun");
            _firstPersonWeapon.SetMeshMaterialByMeshIndex(2, "Shell");
            _characterModel.EnableDrawingForAllMesh();
            HideKnifeMesh();
            HideGlockMesh();
            HideAKS74UMesh();
        }
        else if (_currentWeaponIndex == Weapon::MP7) {
           // _firstPersonWeapon.SetName("MP7");
           // _firstPersonWeapon.SetSkinnedModel("MP7_test");
           // _firstPersonWeapon.SetMaterial("Glock"); // fix meeeee. remove meeee
           // _firstPersonWeapon.PlayAndLoopAnimation("MP7_ReloadTest", 1.0f);
        }
        _firstPersonWeapon.SetMeshMaterialByMeshName("manniquen1_2.001", "Hands");
        _firstPersonWeapon.SetMeshMaterialByMeshName("manniquen1_2", "Hands");
        _firstPersonWeapon.SetMeshMaterialByMeshName("SK_FPSArms_Female.001", "FemaleArms");
        _firstPersonWeapon.SetMeshMaterialByMeshName("SK_FPSArms_Female", "FemaleArms");
        _firstPersonWeapon.SetMeshMaterialByMeshName("Arms", "Hands");
        _firstPersonWeapon.DisableDrawingForMeshByMeshName("SK_FPSArms_Female");
        _firstPersonWeapon.DisableDrawingForMeshByMeshName("SK_FPSArms_Female.001");
	}
	_firstPersonWeapon.SetScale(0.001f);
	_firstPersonWeapon.SetRotationX(Player::GetViewRotation().x);
	_firstPersonWeapon.SetRotationY(Player::GetViewRotation().y);
	_firstPersonWeapon.SetPosition(Player::GetViewPos());

    if (_isDead) {
        _characterModel.EnableDrawingForAllMesh();
        HideKnifeMesh();
        HideGlockMesh();
        HideShotgunMesh();
        HideAKS74UMesh();
    }
    
	///////////////
	//   Knife   //

	if (_currentWeaponIndex == Weapon::KNIFE) {
		// Idle
		if (_weaponAction == IDLE) {
			if (Player::IsMoving()) {
				_firstPersonWeapon.PlayAndLoopAnimation("Knife_Walk", 1.0f);
			}
			else {
				_firstPersonWeapon.PlayAndLoopAnimation("Knife_Idle", 1.0f);
			}
		}
		// Draw
		if (_weaponAction == DRAW_BEGIN) {
			_firstPersonWeapon.PlayAnimation("Knife_Draw", 1.0f);
			_weaponAction = DRAWING;
		}
		// Drawing
		if (_weaponAction == DRAWING && _firstPersonWeapon.IsAnimationComplete()) {
			_weaponAction = IDLE;
		}
		// Fire
		if (PressedFire() && CanFire()) {
			if (_weaponAction == DRAWING ||
				_weaponAction == IDLE ||
				_weaponAction == FIRE && _firstPersonWeapon.AnimationIsPastPercentage(25.0f) ||
				_weaponAction == RELOAD && _firstPersonWeapon.AnimationIsPastPercentage(80.0f)) {
				_weaponAction = FIRE;
				int random_number = std::rand() % 3 + 1;
				std::string aninName = "Knife_Swing" + std::to_string(random_number);
				_firstPersonWeapon.PlayAnimation(aninName, 1.5f);
				Audio::PlayAudio("Knife.wav", 1.0f); 
				//SpawnBullet(0, Weapon::KNIFE);
                CheckForKnifeHit();
			}
		}
		if (_weaponAction == FIRE && _firstPersonWeapon.IsAnimationComplete()) {
			_weaponAction = IDLE;
		}
	}


	///////////////
	//   Glock	 //

	if (_currentWeaponIndex == Weapon::GLOCK) {

		// Give reload ammo
		if (_weaponAction == RELOAD || _weaponAction == RELOAD_FROM_EMPTY) {
            if (_needsAmmoReloaded && _firstPersonWeapon.AnimationIsPastPercentage(50.0f)) {
                int ammoToGive = std::min(GLOCK_CLIP_SIZE - _inventory.glockAmmo.clip, _inventory.glockAmmo.total);
				_inventory.glockAmmo.clip += ammoToGive;
				_inventory.glockAmmo.total -= ammoToGive;
				_needsAmmoReloaded = false;
				_glockSlideNeedsToBeOut = false;
			}
		}
		// Idle
		if (_weaponAction == IDLE) {
			if (Player::IsMoving()) {
				_firstPersonWeapon.PlayAndLoopAnimation("Glock_Walk", 1.0f);
			}
			else {
				_firstPersonWeapon.PlayAndLoopAnimation("Glock_Idle", 1.0f);
			}
		}
		// Draw
		if (_weaponAction == DRAW_BEGIN) {
			_firstPersonWeapon.PlayAnimation("Glock_Draw", 1.0f);
			_weaponAction = DRAWING;
		}
		// Drawing
		if (_weaponAction == DRAWING && _firstPersonWeapon.IsAnimationComplete()) {
			_weaponAction = IDLE;
		}
		// Fire
		if (PressedFire() && CanFire()) {
			// Has ammo
			if (_inventory.glockAmmo.clip > 0) {				
				_weaponAction = FIRE; 
                int random_number = std::rand() % 3 + 1;
                if (_hasGlockSilencer) {
                    Audio::PlayAudio("Silenced.wav", 1.0f);
                }
                else {
                    const std::vector<const char*> footstepFilenames = {
                        "Glock_Fire1.wav",
                        "Glock_Fire2.wav",
                        "Glock_Fire3.wav",
                    };
                    Audio::PlayAudio(footstepFilenames[random_number - 1], 1.0f);
                }
                std::string aninName = "Glock_Fire" + std::to_string(random_number);
                _firstPersonWeapon.PlayAnimation(aninName, 1.5f);
				SpawnMuzzleFlash();
				SpawnBullet(0, Weapon::GLOCK);
                SpawnGlockCasing();
				_inventory.glockAmmo.clip--;

			}
			// Is empty 
			else {
				Audio::PlayAudio("Dry_Fire.wav", 0.8f);
			}
		}
		if (_weaponAction == FIRE && _firstPersonWeapon.AnimationIsPastPercentage(60.0f)) {
			_weaponAction = IDLE;
		}
		// Reload
		if (PressedReload() && CanReload()) {
			if (GetCurrentWeaponClipAmmo() == 0) {
				_weaponAction = RELOAD_FROM_EMPTY;
				_firstPersonWeapon.PlayAnimation("Glock_ReloadEmpty", 1.0f);
				Audio::PlayAudio("Glock_ReloadFromEmpty.wav", 1.0f);
			}
			else {
				_firstPersonWeapon.PlayAnimation("Glock_Reload", 1.0f);
				_weaponAction = RELOAD;
				Audio::PlayAudio("Glock_Reload.wav", 1.0f);
			}
			_needsAmmoReloaded = true;
		}
		if (_weaponAction == RELOAD && _firstPersonWeapon.IsAnimationComplete() ||
			_weaponAction == RELOAD_FROM_EMPTY && _firstPersonWeapon.IsAnimationComplete() ||
			_weaponAction == SPAWNING && _firstPersonWeapon.IsAnimationComplete()) {
			_weaponAction = IDLE;
		}
		// Set flag to move glock slide out
		if (GetCurrentWeaponClipAmmo() == 0) {
			if (_weaponAction != RELOAD_FROM_EMPTY) {
				_glockSlideNeedsToBeOut = true;
			}
			if (_weaponAction == RELOAD_FROM_EMPTY && !_firstPersonWeapon.AnimationIsPastPercentage(50.0f)) {
				_glockSlideNeedsToBeOut = false;
			}
		}
        else {
            _glockSlideNeedsToBeOut = false;
        }
	}


	////////////////
	//   AKs74u   //

    if (_currentWeaponIndex == Weapon::AKS74U) {


        if (!_ignoreControl) {

            static float current = 0;
            constexpr float max = 0.0018f;
            constexpr float speed = 20.0f;
            float zoomSpeed = 0.075f;

            if (_weaponAction == ADS_IN ||
                _weaponAction == ADS_IDLE ||
                _weaponAction == ADS_FIRE
                ) {
                current = Util::FInterpTo(current, max, deltaTime, speed);
                _zoom -= zoomSpeed;
            }
            else {
                current = Util::FInterpTo(current, 0, deltaTime, speed);
                _zoom += zoomSpeed;
            }
            //current = max

           // std::cout << Util::WeaponActionToString(_weaponAction) << " " << current << "\n";


            // move the weapon down if you are in ads
            if (InADS()) {


                glm::vec3 offset = GetCameraUp() * current;
                glm::vec3 offset2 = GetCameraForward() * current;

                glm::vec3 position = Player::GetViewPos() - offset + offset2;

                _firstPersonWeapon.SetPosition(position);

            }
        }

        // ZOOM
        _zoom = std::max(0.575f, _zoom);
        _zoom = std::min(1.0f, _zoom);


        float adsInOutSpeed = 3.0f;


        // ADS in
        if (PressingADS() && CanEnterADS() && _hasAKS74UScope) {
            _weaponAction = ADS_IN;
            _firstPersonWeapon.PlayAnimation("AKS74U_ADS_In", adsInOutSpeed);
        }
        // ADS in complete
        if (_weaponAction == ADS_IN && _firstPersonWeapon.IsAnimationComplete()) {
            _firstPersonWeapon.PlayAnimation("AKS74U_ADS_Idle", 1.0f);
            _weaponAction = ADS_IDLE;
        }
        // ADS out
        if (!PressingADS()) {

            if (_weaponAction == ADS_IN ||
                _weaponAction == ADS_IDLE) {
                _weaponAction = ADS_OUT;
                _firstPersonWeapon.PlayAnimation("AKS74U_ADS_Out", adsInOutSpeed);
            }
        }
        // ADS out complete
        if (_weaponAction == ADS_OUT && _firstPersonWeapon.IsAnimationComplete()) {
            _firstPersonWeapon.PlayAnimation("AKS74U_Idle", 1.0f);
            _weaponAction = IDLE;
        }
        // ADS walk
        if (_weaponAction == ADS_IDLE) {
            if (Player::IsMoving()) {
                _firstPersonWeapon.PlayAndLoopAnimation("AKS74U_ADS_Walk", 1.0f);
            }
            else {
                _firstPersonWeapon.PlayAndLoopAnimation("AKS74U_ADS_Idle", 1.0f);
            }
        }

        // ADS fire
        if (PressingFire() && CanFire() && InADS() && _inventory.aks74uAmmo.clip > 0) {
            _weaponAction = ADS_FIRE;
            std::string  aninName = "AKS74U_ADS_Fire1";

            const std::vector<const char*> footstepFilenames = {
                "AK47_Fire1.wav",
                "AK47_Fire2.wav",
                "AK47_Fire3.wav",
            };
            int random_number = std::rand() % 3;
            Audio::PlayAudio(footstepFilenames[random_number], 1.0f);       

            _firstPersonWeapon.PlayAnimation(aninName, 1.625f);
            SpawnMuzzleFlash();
            SpawnBullet(0.02, Weapon::AKS74U);
            SpawnAKS74UCasing();
            _inventory.aks74uAmmo.clip--;
        }
        // Finished ADS Fire
        if (_weaponAction == ADS_FIRE && _firstPersonWeapon.IsAnimationComplete()) {
            _firstPersonWeapon.PlayAnimation("AKS74U_ADS_Idle", 1.0f);
            _weaponAction = ADS_IDLE;
        }
        // Not finished ADS Fire but player HAS LET GO OF RIGHT MOUSE
        if (_weaponAction == ADS_FIRE && !PressingADS()) {
            _weaponAction = ADS_OUT;
            _firstPersonWeapon.PlayAnimation("AKS74U_ADS_Out", adsInOutSpeed);
        }




		// Drop the mag
		if (_needsToDropAKMag && _weaponAction == RELOAD_FROM_EMPTY && _firstPersonWeapon.AnimationIsPastPercentage(16.9f)) {
			_needsToDropAKMag = false;
			DropAKS7UMag();
		}

		// Give reload ammo
		if (_weaponAction == RELOAD || _weaponAction == RELOAD_FROM_EMPTY) {
		//	if (_needsAmmoReloaded && _firstPersonWeapon.AnimationIsPastPercentage(38.0f)) {
			if (_needsAmmoReloaded && _firstPersonWeapon.AnimationIsPastPercentage(10.0f)) {
				int ammoToGive = std::min(AKS74U_MAG_SIZE - _inventory.aks74uAmmo.clip, _inventory.aks74uAmmo.total);
				_inventory.aks74uAmmo.clip += ammoToGive;
				_inventory.aks74uAmmo.total -= ammoToGive;
				_needsAmmoReloaded = false;
			}
		}
		// Fire (has ammo)
		if (PressingFire() && CanFire() && _inventory.aks74uAmmo.clip > 0) {
			_weaponAction = FIRE;
			int random_number = std::rand() % 3 + 1;
			std::string aninName = "AKS74U_Fire" + std::to_string(random_number);
			std::string audioName = "AK47_Fire" + std::to_string(random_number) + ".wav";
			_firstPersonWeapon.PlayAnimation(aninName, 1.625f);
			Audio::PlayAudio(audioName, 1.0f);
			SpawnMuzzleFlash();
            SpawnBullet(0.05f, Weapon::AKS74U);
            SpawnAKS74UCasing();
			_inventory.aks74uAmmo.clip--;
		}
		// Fire (no ammo)
		if (PressedFire() && CanFire() && _inventory.aks74uAmmo.clip == 0) {
			Audio::PlayAudio("Dry_Fire.wav", 0.8f);
		}
		// Reload	
		if (PressedReload() && CanReload()) {						
			if (GetCurrentWeaponClipAmmo() == 0) {
				_firstPersonWeapon.PlayAnimation("AKS74U_ReloadEmpty", 1.0f);
				Audio::PlayAudio("AK47_ReloadEmpty.wav", 1.0f);
				_weaponAction = RELOAD_FROM_EMPTY;
				_needsToDropAKMag = true;
			}
			else {
				_firstPersonWeapon.PlayAnimation("AKS74U_Reload", 1.0f);
				Audio::PlayAudio("AK47_Reload.wav", 1.0f); 
				_weaponAction = RELOAD;
			}
			_needsAmmoReloaded = true;
		}
		// Return to idle
		if (_weaponAction == RELOAD && _firstPersonWeapon.IsAnimationComplete() ||
            _weaponAction == RELOAD_FROM_EMPTY && _firstPersonWeapon.IsAnimationComplete()) {
			_weaponAction = IDLE;
		}
		if (_weaponAction == FIRE && _firstPersonWeapon.AnimationIsPastPercentage(50.0f)) {
			_weaponAction = IDLE;
		}
		//Idle
		if (_weaponAction == IDLE) {
			if (Player::IsMoving()) {
				_firstPersonWeapon.PlayAndLoopAnimation("AKS74U_Walk", 1.0f);
			}
            else {
                _firstPersonWeapon.PlayAndLoopAnimation("AKS74U_Idle", 1.0f);
			}
		}
		// Draw
		if (_weaponAction == DRAW_BEGIN) {
			_firstPersonWeapon.PlayAnimation("AKS74U_Draw", 1.125f);
			_weaponAction = DRAWING;
		}
		// Drawing
		if (_weaponAction == DRAWING && _firstPersonWeapon.IsAnimationComplete()) {
			_weaponAction = IDLE;
		}
	}


	/////////////////
	//   Shotgun   //

    if (_currentWeaponIndex == Weapon::SHOTGUN) {

        // Idle
        if (_weaponAction == IDLE) {
            if (Player::IsMoving()) {
                _firstPersonWeapon.PlayAndLoopAnimation("Shotgun_Walk", 1.0f);
            }
            else {
                _firstPersonWeapon.PlayAndLoopAnimation("Shotgun_Idle", 1.0f);
            }
        }
        // Draw
        if (_weaponAction == DRAW_BEGIN) {
            _firstPersonWeapon.PlayAnimation("Shotgun_Equip", 1.0f);
            _weaponAction = DRAWING;
        }
        // Drawing
        if (_weaponAction == DRAWING && _firstPersonWeapon.IsAnimationComplete()) {
            _weaponAction = IDLE;
        }
        // Fire
        if (PressedFire() && CanFire()) {
            // Has ammo
            if (_inventory.shotgunAmmo.clip > 0) {
                _weaponAction = FIRE;
                std::string aninName = "Shotgun_Fire";
                std::string audioName = "Shotgun_Fire.wav";
                _firstPersonWeapon.PlayAnimation(aninName, 1.0f);
                Audio::PlayAudio(audioName, 1.0f);
                SpawnMuzzleFlash();
                for (int i = 0; i < 12; i++) {
                    SpawnBullet(0.1, Weapon::SHOTGUN);
                }
                SpawnShotgunShell();
                _inventory.shotgunAmmo.clip--;

            }
            // Is empty 
            else {
                Audio::PlayAudio("Dry_Fire.wav", 0.8f);
            }
        }
        if (_weaponAction == FIRE && _firstPersonWeapon.AnimationIsPastPercentage(100.0f)) {
            _weaponAction = IDLE;
        }
        // Reload
       if (PressedReload() && CanReload()) {
           _firstPersonWeapon.PlayAnimation("Shotgun_ReloadWetstart", 1.0f);
           _weaponAction = RELOAD_SHOTGUN_BEGIN;
        }

       // BEGIN RELOAD THING
       if (_weaponAction == RELOAD_SHOTGUN_BEGIN && _firstPersonWeapon.IsAnimationComplete()) {
           bool singleShell = false;
           if (_inventory.shotgunAmmo.clip == 7 || 
               _inventory.shotgunAmmo.total == 1 ) {
               singleShell = true;
           }

           // Single shell
           if (singleShell) {
               _firstPersonWeapon.PlayAnimation("Shotgun_Reload1Shell", 1.5f);
               _weaponAction = RELOAD_SHOTGUN_SINGLE_SHELL;
           }
           // Double shell
           else {
               _firstPersonWeapon.PlayAnimation("Shotgun_Reload2Shells", 1.5f);
               _weaponAction = RELOAD_SHOTGUN_DOUBLE_SHELL;
           }

           _needsShotgunFirstShellAdded = true;
           _needsShotgunSecondShellAdded = true;
       }
       // END RELOAD THING
       if (_weaponAction == RELOAD_SHOTGUN_SINGLE_SHELL && _firstPersonWeapon.IsAnimationComplete() && GetCurrentWeaponClipAmmo() == SHOTGUN_AMMO_SIZE) {
           _firstPersonWeapon.PlayAnimation("Shotgun_ReloadEnd", 1.25f);
           _weaponAction = RELOAD_SHOTGUN_END;
       }
       if (_weaponAction == RELOAD_SHOTGUN_DOUBLE_SHELL && _firstPersonWeapon.IsAnimationComplete() && GetCurrentWeaponClipAmmo() == SHOTGUN_AMMO_SIZE) {
           _firstPersonWeapon.PlayAnimation("Shotgun_ReloadEnd", 1.25f);
           _weaponAction = RELOAD_SHOTGUN_END;
       }
       // CONTINUE THE RELOAD THING
       if (_weaponAction == RELOAD_SHOTGUN_SINGLE_SHELL && _firstPersonWeapon.IsAnimationComplete()) {
           if (_inventory.shotgunAmmo.total > 0) {
               _firstPersonWeapon.PlayAnimation("Shotgun_Reload1Shell", 1.5f);
               _weaponAction = RELOAD_SHOTGUN_SINGLE_SHELL;
               _needsShotgunFirstShellAdded = true;
               _needsShotgunSecondShellAdded = true;
           }
           else {
               _firstPersonWeapon.PlayAnimation("Shotgun_ReloadEnd", 1.25f);
               _weaponAction = RELOAD_SHOTGUN_END;
           }
       }
       if (_weaponAction == RELOAD_SHOTGUN_DOUBLE_SHELL && _firstPersonWeapon.IsAnimationComplete()) {
           bool singleShell = false;
           if (_inventory.shotgunAmmo.clip == 7 ||
               _inventory.shotgunAmmo.total == 1) {
               singleShell = true;
           }
           // Single shell
           if (singleShell) {
               _firstPersonWeapon.PlayAnimation("Shotgun_Reload1Shell", 1.5f);
               _weaponAction = RELOAD_SHOTGUN_SINGLE_SHELL;
           }
           // Double shell
           else {
               _firstPersonWeapon.PlayAnimation("Shotgun_Reload2Shells", 1.5f);
               _weaponAction = RELOAD_SHOTGUN_DOUBLE_SHELL;
           }
           _needsShotgunFirstShellAdded = true;
           _needsShotgunSecondShellAdded = true;
       } 

       // Give ammo on reload
       if (_needsShotgunFirstShellAdded && _weaponAction == RELOAD_SHOTGUN_SINGLE_SHELL && _firstPersonWeapon.AnimationIsPastPercentage(35.0f)) {
           _inventory.shotgunAmmo.clip++;
           _inventory.shotgunAmmo.total--;
           _needsShotgunFirstShellAdded = false;
           Audio::PlayAudio("Shotgun_Reload.wav", 1.0f);
       }
       if (_needsShotgunFirstShellAdded && _weaponAction == RELOAD_SHOTGUN_DOUBLE_SHELL && _firstPersonWeapon.AnimationIsPastPercentage(28.0f)) {
           _inventory.shotgunAmmo.clip++;
           _inventory.shotgunAmmo.total--;
           _needsShotgunFirstShellAdded = false;
           Audio::PlayAudio("Shotgun_Reload.wav", 1.0f);
       }
       if (_needsShotgunSecondShellAdded && _weaponAction == RELOAD_SHOTGUN_DOUBLE_SHELL && _firstPersonWeapon.AnimationIsPastPercentage(62.0f)) {
           _inventory.shotgunAmmo.clip++;
           _inventory.shotgunAmmo.total--;
           _needsShotgunSecondShellAdded = false;
           Audio::PlayAudio("Shotgun_Reload.wav", 1.0f);
       }

       if (_weaponAction == FIRE && _firstPersonWeapon.IsAnimationComplete() ||
           _weaponAction == RELOAD_SHOTGUN_END && _firstPersonWeapon.IsAnimationComplete() ||
            _weaponAction == SPAWNING && _firstPersonWeapon.IsAnimationComplete()) {
            _weaponAction = IDLE;
        }
    }


	
	// Update animated bone transforms for the first person weapon model
	_firstPersonWeapon.Update(deltaTime);

	// Move glock slide bone if necessary
	if (GetCurrentWeaponIndex() == GLOCK && _glockSlideNeedsToBeOut) {
		Transform transform;
		transform.position.y = 0.055f;
		_firstPersonWeapon._animatedTransforms.local[3] *= transform.to_mat4();	// 3 is slide bone
	}

	// Weapon sway
	if (!_ignoreControl) {

        float xMax = 4.0;

        if (_zoom < 0.99f) {
            xMax = 2.0f;
        }

        float SWAY_AMOUNT = 0.125f;
        float SMOOTH_AMOUNT = 4.0f;
        float SWAY_MIN_X = -2.25f;
        float SWAY_MAX_X = xMax;
        float SWAY_MIN_Y = -2;
        float SWAY_MAX_Y = 0.95f;

        float xOffset = -(float)InputMulti::GetMouseXOffset(_mouseIndex);
        float yOffset = (float)InputMulti::GetMouseYOffset(_mouseIndex);

        if (GetCurrentWeaponIndex() == SHOTGUN) {
            xOffset *= -1;
        }

        float movementX = -xOffset * SWAY_AMOUNT;
        float movementY = -yOffset * SWAY_AMOUNT;

        movementX = std::min(movementX, SWAY_MAX_X);
        movementX = std::max(movementX, SWAY_MIN_X);
        movementY = std::min(movementY, SWAY_MAX_Y);
        movementY = std::max(movementY, SWAY_MIN_Y);

        _weaponSwayTransform.position.x = Util::FInterpTo(_weaponSwayTransform.position.x, movementX, deltaTime, SMOOTH_AMOUNT);
        _weaponSwayTransform.position.y = Util::FInterpTo(_weaponSwayTransform.position.y, movementY, deltaTime, SMOOTH_AMOUNT);
        _weaponSwayMatrix = _weaponSwayTransform.to_mat4();

        for (auto& transform : _firstPersonWeapon._animatedTransforms.local) {
            transform = _weaponSwayMatrix * transform;
        }
	}
}

AnimatedGameObject& Player::GetFirstPersonWeapon() {
	return _firstPersonWeapon;
}

void Player::SpawnMuzzleFlash() {
	_muzzleFlashTimer = 0;
	_muzzleFlashRotation = Util::RandomFloat(0, HELL_PI * 2);
}

glm::vec3 Player::GetGlockBarrelPosition() {

    auto glock = GetFirstPersonWeapon();
    if (glock.GetName() == "Glock") {
        int boneIndex = glock._skinnedModel->m_BoneMapping["Barrel"];
        glm::mat4 boneMatrix = glock._animatedTransforms.worldspace[boneIndex];
        Transform offset;
        offset.position = glm::vec3(0, 2 + 2, 11);
        if (_hasGlockSilencer) {
            offset.position = glm::vec3(0, 2 + 2, 28);
        }
        glm::mat4 m = glock.GetModelMatrix() * boneMatrix * offset.to_mat4() * _weaponSwayMatrix;
        float x = m[3][0];
        float y = m[3][1];
        float z = m[3][2];
        return glm::vec3(x, y, z);
    }
    else {
        return glm::vec3(0);
    }
}

void Player::SpawnGlockCasing() {

	Transform transform;
	transform.position = _firstPersonWeapon.GetGlockCasingSpawnPostion();
	transform.rotation.x = HELL_PI * 0.5f;
	transform.rotation.y = _rotation.y + (HELL_PI * 0.5f);

	PhysicsFilterData filterData;
	filterData.raycastGroup = RaycastGroup::RAYCAST_DISABLED;
	filterData.collisionGroup = CollisionGroup::BULLET_CASING;
    filterData.collidesWith = CollisionGroup::ENVIROMENT_OBSTACLE;
    //filterData.collidesWith = CollisionGroup::NO_COLLISION;

	PxShape* shape = Physics::CreateBoxShape(0.01f, 0.004f, 0.004f);
	PxRigidDynamic* body = Physics::CreateRigidDynamic(transform, filterData, shape);

	PxVec3 force = Util::GlmVec3toPxVec3(glm::normalize(GetCameraRight() + glm::vec3(0.0f, Util::RandomFloat(0.7f, 0.9f), 0.0f)) * glm::vec3(0.00215f));
	body->addForce(force);
	body->setAngularVelocity(PxVec3(Util::RandomFloat(0.0f, 100.0f), Util::RandomFloat(0.0f, 100.0f), Util::RandomFloat(0.0f, 100.0f)));
    //body->userData = (void*)&CasingType::BULLET_CASING;
    body->userData = (void*)&EngineState::weaponNamePointers[GLOCK];
    body->setName("BulletCasing");

 // std::cout << 

	BulletCasing bulletCasing;
	bulletCasing.type = GLOCK;
	bulletCasing.rigidBody = body;
	Scene::_bulletCasings.push_back(bulletCasing);
}



void Player::SpawnShotgunShell() {

    Transform transform;
    transform.position = _firstPersonWeapon.GetShotgunBarrelPosition();
    transform.rotation.x = HELL_PI * 0.5f;
    transform.rotation.y = _rotation.y + (HELL_PI * 0.5f);
    //transform.rotation.x = HELL_PI * 0.5f;
    //transform.rotation.y = _rotation.y + (HELL_PI * 0.5f);
    //transform.rotation.z = HELL_PI * 0.5f;

    transform.position -= _forward * 0.20f;
    transform.position -= _up * 0.05f;

    PhysicsFilterData filterData;
    filterData.raycastGroup = RaycastGroup::RAYCAST_DISABLED;
    filterData.collisionGroup = CollisionGroup::BULLET_CASING;
    filterData.collidesWith = CollisionGroup::ENVIROMENT_OBSTACLE;
    //filterData.collidesWith = CollisionGroup::NO_COLLISION;

    PxShape* shape = Physics::CreateBoxShape(0.03f, 0.007f, 0.007f);
    PxRigidDynamic* body = Physics::CreateRigidDynamic(transform, filterData, shape);

    PxVec3 force = Util::GlmVec3toPxVec3(glm::normalize(GetCameraRight() + glm::vec3(0.0f, Util::RandomFloat(0.7f, 1.4f), 0.0f)) * glm::vec3(0.02f));
    body->addForce(force);
    //body->userData = (void*)&CasingType::SHOTGUN_SHELL;
    body->userData = (void*)&EngineState::weaponNamePointers[SHOTGUN];
    body->setName("ShotgunShell");

    //body->setAngularVelocity(PxVec3(Util::RandomFloat(0.0f, 50.0f), Util::RandomFloat(0.0f, 50.0f), Util::RandomFloat(0.0f, 50.0f)));
    //shape->release();

    BulletCasing bulletCasing;
    bulletCasing.type = SHOTGUN;
    bulletCasing.rigidBody = body;
    Scene::_bulletCasings.push_back(bulletCasing);

    //body->userData = (void*)&Scene::_bulletCasings.back();
}

void Player::SpawnAKS74UCasing() {

	Transform transform;
	transform.position = _firstPersonWeapon.GetAK74USCasingSpawnPostion();
	transform.rotation.x = HELL_PI * 0.5f;
	transform.rotation.y = _rotation.y + (HELL_PI * 0.5f);

	PhysicsFilterData filterData;
	filterData.raycastGroup = RaycastGroup::RAYCAST_DISABLED;
	filterData.collisionGroup = CollisionGroup::BULLET_CASING;
    filterData.collidesWith = CollisionGroup::ENVIROMENT_OBSTACLE;
   // filterData.collidesWith = CollisionGroup::NO_COLLISION;

	PxShape* shape = Physics::CreateBoxShape(0.02f, 0.004f, 0.004f);
	PxRigidDynamic* body = Physics::CreateRigidDynamic(transform, filterData, shape);

	PxVec3 force = Util::GlmVec3toPxVec3(glm::normalize(GetCameraRight() + glm::vec3(0.0f, Util::RandomFloat(0.7f, 1.4f), 0.0f)) * glm::vec3(0.003f));
	body->addForce(force);
	body->setAngularVelocity(PxVec3(Util::RandomFloat(0.0f, 50.0f), Util::RandomFloat(0.0f, 50.0f), Util::RandomFloat(0.0f, 50.0f)));
    //body->userData = (void*)&CasingType::BULLET_CASING;
    body->userData = (void*)&EngineState::weaponNamePointers[AKS74U];
    body->setName("BulletCasing");


    //shape->release();

	BulletCasing bulletCasing;
	bulletCasing.type = AKS74U;
	bulletCasing.rigidBody = body;
	Scene::_bulletCasings.push_back(bulletCasing);

	//body->userData = (void*)&Scene::_bulletCasings.back();
}


void Player::SpawnBullet(float variance, Weapon type) {

	_muzzleFlashCounter = 0.0005f;

	Bullet bullet;
	bullet.spawnPosition = GetViewPos();
    bullet.type = type;
    bullet.raycastFlags = _bulletFlags;// RaycastGroup::RAYCAST_ENABLED;
    bullet.parentPlayersViewRotation = GetCameraRotation();


    glm::vec3 offset = glm::vec3(0);
	offset.x = Util::RandomFloat(-(variance * 0.5f), variance * 0.5f);
	offset.y = Util::RandomFloat(-(variance * 0.5f), variance * 0.5f);
	offset.z = Util::RandomFloat(-(variance * 0.5f), variance * 0.5f);
        
	bullet.direction = (glm::normalize(GetCameraForward() + offset)) * glm::vec3(-1);
	Scene::_bullets.push_back(bullet);
}

void Player::DropAKS7UMag() {

    if (true) {
        return;
    }

	PhysicsFilterData magFilterData;
	magFilterData.raycastGroup = RAYCAST_DISABLED;
	magFilterData.collisionGroup = CollisionGroup::GENERIC_BOUNCEABLE;
	magFilterData.collidesWith = CollisionGroup(ENVIROMENT_OBSTACLE | GENERIC_BOUNCEABLE);
	float magDensity = 750.0f;

	GameObject& mag = Scene::_gameObjects.emplace_back();
//	mag.SetPosition(GetViewPos() + glm::vec3(0, -0.2f, 0));
//	mag.SetRotationX(-1.7f);
	//mag.SetRotationY(0.0f);
	//mag.SetRotationZ(-1.6f);

	//mag.SetModel("AKS74UMag2");
	mag.SetModel("AKS74UMag");
	mag.SetName("AKS74UMag");
	mag.SetMeshMaterial("AKS74U_3");

	AnimatedGameObject& ak2 = GetFirstPersonWeapon();
	glm::mat4 matrix = ak2.GetBoneWorldMatrixFromBoneName("Magazine");
	glm::mat4 magWorldMatrix = ak2.GetModelMatrix() * GetWeaponSwayMatrix() * matrix;

	/*PxMat44 pxMat = Util::GlmMat4ToPxMat44(magWorldMatrix);
	PxTransform pxTrans = PxTransform(pxMat);
	mag._collisionBody->setGlobalPose(pxTrans);

	*/


    /*
	//mag.CreateRigidBody(mag.GetGameWorldMatrix(), false);
	mag.CreateRigidBody(magWorldMatrix, false);
	mag.SetRaycastShapeFromModel(AssetManager::GetModel("AKS74UMag"));
	mag.AddCollisionShapeFromConvexMesh(&AssetManager::GetModel("AKS74UMag_ConvexMesh")->_meshes[0], magFilterData);
	mag.SetModelMatrixMode(ModelMatrixMode::PHYSX_TRANSFORM);
	mag.UpdateRigidBodyMassAndInertia(magDensity);
	//mag.CreateEditorPhysicsObject();
    */

	//mag.SetScale(0.1f);

	// what, is this smart??????????? and necessary????????
	for (auto& gameObject : Scene::_gameObjects) {
		//gameObject.CreateEditorPhysicsObject();
	}

	std::cout << "dropped ak mag\n";

}

float Player::GetMuzzleFlashTime() {
    return _muzzleFlashTimer;
}

float Player::GetMuzzleFlashRotation() {
	return _muzzleFlashRotation;
}


void Player::SetRotation(glm::vec3 rotation) {
	_rotation = rotation;
}

float Player::GetRadius() {
	return _radius;
}


PxShape* Player::GetCharacterControllerShape() {
	PxShape* shape;
	_characterController->getActor()->getShapes(&shape, 1);
	return shape;
}

PxRigidDynamic* Player::GetCharacterControllerActor() {
	return _characterController->getActor();
}

void Player::CreateItemPickupOverlapShape() {
	if (_itemPickupOverlapShape) {
		_itemPickupOverlapShape->release();
	}
    float radius = PLAYER_CAPSULE_RADIUS + 0.075;
    float halfHeight = PLAYER_CAPSULE_HEIGHT * 0.75f;
    _itemPickupOverlapShape = Physics::GetPhysics()->createShape(PxCapsuleGeometry(radius, halfHeight), *Physics::GetDefaultMaterial(), true);
}

PxShape* Player::GetItemPickupOverlapShape() {
	return _itemPickupOverlapShape;
}

void Player::CreateCharacterController(glm::vec3 position) {

	PxMaterial* material = Physics::GetDefaultMaterial();
	PxCapsuleControllerDesc* desc = new PxCapsuleControllerDesc;
	desc->setToDefault();
	desc->height = PLAYER_CAPSULE_HEIGHT;
	desc->radius = PLAYER_CAPSULE_RADIUS;
	desc->position = PxExtendedVec3(position.x, position.y + (PLAYER_CAPSULE_HEIGHT / 2) + (PLAYER_CAPSULE_RADIUS * 2), position.z);
	desc->material = material;
	desc->stepOffset = 0.1f; 
	desc->contactOffset = 0.001;
	desc->scaleCoeff = .99f;
	desc->reportCallback = &Physics::_cctHitCallback;
	_characterController = Physics::_characterControllerManager->createController(*desc);

	PxShape* shape;
	_characterController->getActor()->getShapes(&shape, 1);

	PxFilterData filterData;
	filterData.word1 = CollisionGroup::PLAYER;
	filterData.word2 = CollisionGroup(ITEM_PICK_UP | ENVIROMENT_OBSTACLE);
	shape->setQueryFilterData(filterData);

}

glm::mat4 Player::GetProjectionMatrix() {
    float width = (float)BackEnd::GetWindowedWidth();
    float height = (float)BackEnd::GetWindowedHeight();

    if (Game::GetSplitscreenMode() == SplitscreenMode::TWO_PLAYER) {
        height *= 0.5f;
    }
    return glm::perspective(_zoom, width / height, NEAR_PLANE, FAR_PLANE);
}

bool Player::CanEnterADS() {

    if (!InADS() && _weaponAction != RELOAD && _weaponAction != RELOAD_FROM_EMPTY ||
        _weaponAction == RELOAD && _firstPersonWeapon.AnimationIsPastPercentage(65.0f) ||
        _weaponAction == RELOAD_FROM_EMPTY && _firstPersonWeapon.AnimationIsPastPercentage(65.0f)) {
        return true;
    }
    else {
        return false;
    }
}

WeaponAction& Player::GetWeaponAction() {
    return _weaponAction;
}


bool Player::PressingWalkForward() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyDown(_keyboardIndex, _mouseIndex, _controls.WALK_FORWARD);
    }
    else {
        // return InputMulti::ButtonDown(_controllerIndex, _controls.WALK_FORWARD);
        return false;
    }
}

bool Player::PressingWalkBackward() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyDown(_keyboardIndex, _mouseIndex, _controls.WALK_BACKWARD);
    }
    else {
        //return InputMulti::ButtonDown(_controllerIndex, _controls.WALK_BACKWARD);
        return false;
    }
}

bool Player::PressingWalkLeft() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyDown(_keyboardIndex, _mouseIndex, _controls.WALK_LEFT);
    }
    else {
        //return InputMulti::ButtonDown(_controllerIndex, _controls.WALK_LEFT);        
        return false;
    }
}

bool Player::PressingWalkRight() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyDown(_keyboardIndex, _mouseIndex, _controls.WALK_RIGHT);
    }
    else {
        //return InputMulti::ButtonDown(_controllerIndex, _controls.WALK_RIGHT);
        return false;
    }
}

bool Player::PressingCrouch() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyDown(_keyboardIndex, _mouseIndex, _controls.CROUCH);
    }
    else {
        //return InputMulti::ButtonDown(_controllerIndex, _controls.CROUCH);
        return false;
    }
}

bool Player::PressedWalkForward() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.WALK_FORWARD);
    }
    else {
        //return InputMulti::ButtonPressed(_controllerIndex, _controls.WALK_FORWARD);
        return false;
    }
}

bool Player::PressedWalkBackward() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.WALK_BACKWARD);
    }
    else {
        //return InputMulti::ButtonPressed(_controllerIndex, _controls.WALK_BACKWARD);
        return false;
    }
}

bool Player::PressedWalkLeft() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.WALK_LEFT);
    }
    else {
        // return InputMulti::ButtonPressed(_controllerIndex, _controls.WALK_LEFT);
        return false;
    }
}

bool Player::PressedWalkRight() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.WALK_RIGHT);
    }
    else {
        //return InputMulti::ButtonPressed(_controllerIndex, _controls.WALK_RIGHT);
        return false;
    }
}

bool Player::PressedInteract() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.INTERACT);
    }
    else {
        //return InputMulti::ButtonPressed(_controllerIndex, _controls.INTERACT);
        return false;
    }
}

bool Player::PressedReload() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.RELOAD);
    }
    else {
        //return InputMulti::ButtonPressed(_controllerIndex, _controls.RELOAD);
        return false;
    }
}

bool Player::PressedFire() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.FIRE);
    }
    else {
        //return InputMulti::ButtonPressed(_controllerIndex, _controls.FIRE);
        return false;
    }
}

bool Player::PressingFire() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyDown(_keyboardIndex, _mouseIndex, _controls.FIRE);
    }
    else {
        // return InputMulti::ButtonDown(_controllerIndex, _controls.FIRE);
        return false;
    }
}

bool Player::PresingJump() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyDown(_keyboardIndex, _mouseIndex, _controls.JUMP);
    }
    else {
        //return InputMulti::ButtonPressed(_controllerIndex, _controls.JUMP);
        return false;
    }
}

bool Player::PressedCrouch() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.CROUCH);
    }
    else {
        // return InputMulti::ButtonPressed(_controllerIndex, _controls.CROUCH);
        return false;
    }
}

bool Player::PressedNextWeapon() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.NEXT_WEAPON);
    }
    else {
        //return InputMulti::ButtonPressed(_controllerIndex, _controls.NEXT_WEAPON);
        return false;
    }
}

bool Player::PressingADS() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyDown(_keyboardIndex, _mouseIndex, _controls.ADS);
    }
    else {
        // return InputMulti::ButtonDown(_controllerIndex, _controls.ADS);
        return false;
    }
}

bool Player::PressedADS() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.ADS);
    }
    else {
        // return InputMulti::ButtonPressed(_controllerIndex, _controls.ADS);    
        return false;
    }
}

bool Player::PressedEscape() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.ESCAPE);
    }
    else {
        // return InputMulti::ButtonPressed(_controllerIndex, _controls.ESCAPE);
        return false;
    }
}
bool Player::PressedFullscreen() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.DEBUG_FULLSCREEN);
    }
    else {
        // return InputMulti::ButtonPressed(_controllerIndex, _controls.ESCAPE);
        return false;
    }
}

bool Player::PressedOne() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.DEBUG_ONE);
    }
    else {
        // return InputMulti::ButtonPressed(_controllerIndex, _controls.ESCAPE);
        return false;
    }
}

bool Player::PressedTwo() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.DEBUG_TWO);
    }
    else {
        // return InputMulti::ButtonPressed(_controllerIndex, _controls.ESCAPE);
        return false;
    }
}

bool Player::PressedThree() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.DEBUG_THREE);
    }
    else {
        // return InputMulti::ButtonPressed(_controllerIndex, _controls.ESCAPE);
        return false;
    }
}
bool Player::PressedFour() {
    if (_inputType == InputType::KEYBOARD_AND_MOUSE) {
        return InputMulti::KeyPressed(_keyboardIndex, _mouseIndex, _controls.DEBUG_FOUR);
    }
    else {
        // return InputMulti::ButtonPressed(_controllerIndex, _controls.ESCAPE);
        return false;
    }
}

glm::vec3 Player::GetCameraRotation() {
    return _rotation;
}

void Player::GiveAKS74UScope() {
    _hasAKS74UScope = true;
    ShowPickUpText("PICKED UP AKS74U SCOPE");
    Audio::PlayAudio("ItemPickUp.wav", 1.0f);
}


void Player::HideKnifeMesh() {
    _characterModel.DisableDrawingForMeshByMeshName("SM_Knife_01");
}
void Player::HideGlockMesh() {
    _characterModel.DisableDrawingForMeshByMeshName("Glock");
}
void Player::HideShotgunMesh() {
    _characterModel.DisableDrawingForMeshByMeshName("Shotgun_Mesh");
}
void Player::HideAKS74UMesh() {
    _characterModel.DisableDrawingForMeshByMeshName("FrontSight_low");
    _characterModel.DisableDrawingForMeshByMeshName("Receiver_low");
    _characterModel.DisableDrawingForMeshByMeshName("BoltCarrier_low");
    _characterModel.DisableDrawingForMeshByMeshName("SafetySwitch_low");
    _characterModel.DisableDrawingForMeshByMeshName("MagRelease_low");
    _characterModel.DisableDrawingForMeshByMeshName("Pistol_low");
    _characterModel.DisableDrawingForMeshByMeshName("Trigger_low");
    _characterModel.DisableDrawingForMeshByMeshName("Magazine_Housing_low");
    _characterModel.DisableDrawingForMeshByMeshName("BarrelTip_low");
}

void Player::DrawWeapons() {

    glm::vec3 spawnPos = GetFeetPosition() + glm::vec3(0, 1.5f, 0);

    if (_weaponInventory[Weapon::AKS74U]) {
        GameObject& weapon = Scene::_gameObjects.emplace_back();
        weapon.SetPosition(spawnPos);
        weapon.SetRotationX(-1.7f);
        weapon.SetRotationY(0.0f);
        weapon.SetRotationZ(-1.6f);
        weapon.SetModel("AKS74U_Carlos");
        weapon.SetName("AKS74U_Carlos");
        weapon.SetMeshMaterial("Ceiling");
        weapon.SetMeshMaterialByMeshName("FrontSight_low", "AKS74U_0");
        weapon.SetMeshMaterialByMeshName("Receiver_low", "AKS74U_1");
        weapon.SetMeshMaterialByMeshName("BoltCarrier_low", "AKS74U_1");
        weapon.SetMeshMaterialByMeshName("SafetySwitch_low", "AKS74U_1");
        weapon.SetMeshMaterialByMeshName("Pistol_low", "AKS74U_2");
        weapon.SetMeshMaterialByMeshName("Trigger_low", "AKS74U_2");
        weapon.SetMeshMaterialByMeshName("MagRelease_low", "AKS74U_2");
        weapon.SetMeshMaterialByMeshName("Magazine_Housing_low", "AKS74U_3");
        weapon.SetMeshMaterialByMeshName("BarrelTip_low", "AKS74U_4");
        weapon.SetPickUpType(PickUpType::AKS74U);
        weapon.SetWakeOnStart(true);
        PhysicsFilterData filterData666;
        filterData666.raycastGroup = RAYCAST_DISABLED;
        filterData666.collisionGroup = CollisionGroup::GENERIC_BOUNCEABLE;
        filterData666.collidesWith = (CollisionGroup)(ENVIROMENT_OBSTACLE | GENERIC_BOUNCEABLE);
        weapon.SetKinematic(false);
        weapon.AddCollisionShapeFromModelIndex(AssetManager::GetModelIndexByName("AKS74U_Carlos_ConvexMesh"), filterData666);
        weapon.SetRaycastShapeFromModelIndex(AssetManager::GetModelIndexByName("AKS74U_Carlos"));
        weapon.SetModelMatrixMode(ModelMatrixMode::PHYSX_TRANSFORM);
        weapon.UpdateRigidBodyMassAndInertia(50.0f);
        weapon.DisableRespawnOnPickup();
        weapon.collisionRigidBody.SetGlobalPose(weapon._transform.to_mat4());
    }

    if (_weaponInventory[Weapon::SHOTGUN]) {        
        GameObject& weapon = Scene::_gameObjects.emplace_back();
        weapon.SetPosition(spawnPos);
        weapon.SetRotationX(-1.7f);
        weapon.SetRotationY(0.0f);
        weapon.SetRotationZ(-1.6f);
        weapon.SetModel("Shotgun_Isolated");
        weapon.SetName("Shotgun_Pickup");
        weapon.SetMeshMaterial("Shotgun");
        weapon.SetPickUpType(PickUpType::SHOTGUN);
        weapon.SetWakeOnStart(true);
        PhysicsFilterData filterData666;
        filterData666.raycastGroup = RAYCAST_DISABLED;
        filterData666.collisionGroup = CollisionGroup::GENERIC_BOUNCEABLE;
        filterData666.collidesWith = (CollisionGroup)(ENVIROMENT_OBSTACLE | GENERIC_BOUNCEABLE);
        weapon.SetKinematic(false);
        weapon.AddCollisionShapeFromModelIndex(AssetManager::GetModelIndexByName("Shotgun_Isolated_ConvexMesh"), filterData666);
        weapon.SetRaycastShapeFromModelIndex(AssetManager::GetModelIndexByName("Shotgun_Isolated"));
        weapon.SetModelMatrixMode(ModelMatrixMode::PHYSX_TRANSFORM);
        weapon.UpdateRigidBodyMassAndInertia(50.0f);
        weapon.DisableRespawnOnPickup();
        weapon.collisionRigidBody.SetGlobalPose(weapon._transform.to_mat4());
    }

    if (_weaponInventory[Weapon::GLOCK]) {
        GameObject& weapon = Scene::_gameObjects.emplace_back();
        weapon.SetPosition(spawnPos);
        weapon.SetRotationX(-1.7f);
        weapon.SetRotationY(0.0f);
        weapon.SetRotationZ(-1.6f);
        weapon.SetModel("Glock_Isolated");
        weapon.SetName("GLOCKGLOCK");
        weapon.SetMeshMaterial("Glock");
        weapon.SetPickUpType(PickUpType::GLOCK);
        weapon.SetWakeOnStart(true);
        PhysicsFilterData filterData666;
        filterData666.raycastGroup = RAYCAST_DISABLED;
        filterData666.collisionGroup = CollisionGroup::GENERIC_BOUNCEABLE;
        filterData666.collidesWith = (CollisionGroup)(ENVIROMENT_OBSTACLE | GENERIC_BOUNCEABLE);
        weapon.SetKinematic(false);
        weapon.AddCollisionShapeFromModelIndex(AssetManager::GetModelIndexByName("Glock_Isolated_ConvexMesh"), filterData666);
        weapon.SetRaycastShapeFromModelIndex(AssetManager::GetModelIndexByName("Glock_Isolated"));
        weapon.SetModelMatrixMode(ModelMatrixMode::PHYSX_TRANSFORM);
        weapon.UpdateRigidBodyMassAndInertia(200.0f);
        weapon.DisableRespawnOnPickup();
        weapon.collisionRigidBody.SetGlobalPose(weapon._transform.to_mat4());
    }
}

void Player::Kill()  {

    if (_isDead) {
        return;
    }

    DrawWeapons();

    PxExtendedVec3 globalPose = PxExtendedVec3(-1, 0.1, -1);
    _characterController->setFootPosition(globalPose);

    std::cout << _playerName << " was killed\n";
    _characterModel._animationMode = AnimatedGameObject::AnimationMode::RAGDOLL;

    _isDead = true;

    _characterModel._ragdoll.EnableCollision();

    for (RigidComponent& rigid : _characterModel._ragdoll._rigidComponents) {
        rigid.pxRigidBody->wakeUp();
    }

    Audio::PlayAudio("Death0.wav", 1.0f);   
}

void Player::CheckForKnifeHit() {


    for (int i = 0; i < Game::GetPlayerCount(); i++) {
        Player* otherPlayer = Game::GetPlayerByIndex(i);

        // skip self
        if (otherPlayer == this)
            continue;

        if (!otherPlayer->_isDead) {

            bool knifeHit = false;

            glm::vec3 myPos = GetViewPos();
            glm::vec3 theirPos = otherPlayer->GetViewPos();
            glm::vec3 forward = GetCameraForward() * glm::vec3(-1);

            glm::vec3 v = glm::normalize(myPos - theirPos);
            float distToEnemy = glm::distance(myPos, theirPos);

            float dotProduct = glm::dot(forward, v);
            std::cout << dotProduct << "\n";
            if (dotProduct < -0.65 && distToEnemy < 1.0f) {
                knifeHit = true;
            }

            if (knifeHit) {
                // apply damage
                if (otherPlayer->_health > 0) {
                    otherPlayer->_health -= 20;// +rand() % 50;

                    otherPlayer->GiveDamageColor();

                    // Are they dead???
                    if (otherPlayer->_health <= 0 && !otherPlayer->_isDead)
                    {
                        otherPlayer->_health = 0;
                        std::string file = "Death0.wav";
                        Audio::PlayAudio(file.c_str(), 1.0f);

                        otherPlayer->Kill();
                        _killCount++;
                        /*
                        Player* otherPlayer = NULL;
                        if (this == &Scene::_players[0]) {
                            otherPlayer = &Scene::_players[1];
                        }
                        else {
                            otherPlayer = &Scene::_players[0];
                        }

                        glm::vec3 deathPosition = { otherPlayer->GetFeetPosition().x, 0.1, otherPlayer->GetFeetPosition().z };
                        deathPosition += (otherPlayer->_movementVector * 0.25f);

                        AnimatedGameObject* dyingGuy = Scene::GetAnimatedGameObjectByName("DyingGuy");
                        dyingGuy->SetRotationY(GetViewRotation().y);
                        dyingGuy->SetPosition(deathPosition);
                        dyingGuy->PlayAnimation("DyingGuy_Death", 1.0f); */

                    }
                }

                // Audio
                const std::vector<std::string> fleshImpactFilenames = {
                    "FLY_Bullet_Impact_Flesh_1.wav",
                    "FLY_Bullet_Impact_Flesh_2.wav",
                    "FLY_Bullet_Impact_Flesh_3.wav",
                    "FLY_Bullet_Impact_Flesh_4.wav",
                    "FLY_Bullet_Impact_Flesh_5.wav",
                    "FLY_Bullet_Impact_Flesh_6.wav",
                    "FLY_Bullet_Impact_Flesh_7.wav",
                    "FLY_Bullet_Impact_Flesh_8.wav",
                };
                int random = rand() % 8;
                Audio::PlayAudio(fleshImpactFilenames[random], 0.5f);
            }
        }
    }
}


bool Player::IsDead() {
    return _isDead;
}
bool Player::IsAlive() {
    return !_isDead;
}

bool Player::RespawnAllowed() {
    return _isDead && _timeSinceDeath > 3.25f;
}

CrosshairType Player::GetCrosshairType() {
    
    // None
    if (IsDead()) {
        return CrosshairType::NONE;
    }

    // Interact
    else if (_cameraRayResult.physicsObjectType == DOOR) {
        Door* door = (Door*)(_cameraRayResult.parent);
        if (door && door->IsInteractable(GetFeetPosition())) {
            return CrosshairType::INTERACT;
        }
    }
    else if (_cameraRayResult.physicsObjectType == GAME_OBJECT && _cameraRayResult.parent) {
        GameObject* gameObject = (GameObject*)(_cameraRayResult.parent);
        return CrosshairType::INTERACT;
    }

    // Regular
    return CrosshairType::REGULAR;
    
}

std::vector<RenderItem2D> Player::GetHudRenderItems(ivec2 viewportSize) {

    std::vector<RenderItem2D> renderItems;

    ivec2 viewportCenter = { viewportSize.x * 0.5f, viewportSize.y * 0.5f };
    ivec2 debugTextLocation = { 0, viewportSize.y };
    ivec2 pickupTextLocation = { viewportSize.x * 0.08f, viewportSize.y * 0.10f };

    if (Game::GetSplitscreenMode() == SplitscreenMode::TWO_PLAYER) {
        pickupTextLocation = { viewportSize.x * 0.08f, viewportSize.y * 0.10f };
    }
    if (Game::GetSplitscreenMode() == SplitscreenMode::FOUR_PLAYER) {
        pickupTextLocation = { viewportSize.x * 0.075f, viewportSize.y * 0.10f };
    }

    // Text
    if (!Game::DebugTextIsEnabled() && IsAlive()) {        
        std::string text;
        text += "Health: " + std::to_string(_health) + "\n";
        text += "Kills: " + std::to_string(_killCount) + "\n";
        RendererUtil::AddRenderItems(renderItems, TextBlitter::CreateText(text, debugTextLocation, viewportSize, Alignment::TOP_LEFT, BitmapFontType::STANDARD));
    }
    
    // Press Start
    if (RespawnAllowed()) {
      renderItems.push_back(RendererUtil::CreateRenderItem2D("PressStart", viewportCenter, viewportSize, Alignment::CENTERED));
    }

    if (IsAlive()) {

        // Crosshair
        switch (GetCrosshairType()) {
        case CrosshairType::REGULAR:
            renderItems.push_back(RendererUtil::CreateRenderItem2D("CrosshairDot", viewportCenter, viewportSize, Alignment::CENTERED));
            break;
        case CrosshairType::INTERACT:
            renderItems.push_back(RendererUtil::CreateRenderItem2D("CrosshairSquare", viewportCenter, viewportSize, Alignment::CENTERED));
            break;
        default:
            break;
        }

        // Pickup text
        RendererUtil::AddRenderItems(renderItems, TextBlitter::CreateText(_pickUpText, pickupTextLocation, viewportSize, Alignment::BOTTOM_LEFT, BitmapFontType::STANDARD));
    }

    return renderItems;
}

std::vector<RenderItem2D> Player::GetHudRenderItemsHiRes(ivec2 viewportSize) {

    std::vector<RenderItem2D> renderItems;

    float ammoTextScale = 1.2;
    ivec2 viewportCenter = { viewportSize.x * 0.5f, viewportSize.y * 0.5f };
    ivec2 ammoSlashTextLocation = { viewportSize.x * 0.875f, viewportSize.y * 0.15f };

    if (Game::GetSplitscreenMode() == SplitscreenMode::TWO_PLAYER) {
        ammoTextScale = 1.2f; 
        ammoSlashTextLocation = { viewportSize.x * 0.875f, viewportSize.y * 0.175f };
    }
    if (Game::GetSplitscreenMode() == SplitscreenMode::FOUR_PLAYER) {
        ammoTextScale = 1.2f;
        ammoSlashTextLocation = { viewportSize.x * 0.86f, viewportSize.y * 0.175f };
    }

    ivec2 ammoClipTextLocation = { ammoSlashTextLocation.x - int(TextBlitter::GetCharacterSize("/", BitmapFontType::AMMO_NUMBERS).x * 0.7f * ammoTextScale), ammoSlashTextLocation.y };
    ivec2 ammoTotalTextLocation = { ammoSlashTextLocation.x + int(TextBlitter::GetCharacterSize("/", BitmapFontType::AMMO_NUMBERS).x * 1.6f * ammoTextScale), ammoSlashTextLocation.y };


    if (IsAlive()) {

        // Ammo
        if (GetCurrentWeaponIndex() != Weapon::KNIFE) {
            std::string clipText = std::to_string(GetCurrentWeaponClipAmmo());
            std::string totalText = std::to_string(GetCurrentWeaponTotalAmmo());
            if (GetCurrentWeaponClipAmmo() == 0) {
                clipText = "[lr]" + clipText;
            }
            else {
                clipText = "[lg]" + clipText;
            }
            RendererUtil::AddRenderItems(renderItems, TextBlitter::CreateText(totalText, ammoTotalTextLocation, viewportSize, Alignment::TOP_LEFT, BitmapFontType::AMMO_NUMBERS, glm::vec3(ammoTextScale * 0.8f)));
            RendererUtil::AddRenderItems(renderItems, TextBlitter::CreateText("/", ammoSlashTextLocation, viewportSize, Alignment::TOP_LEFT, BitmapFontType::AMMO_NUMBERS, glm::vec3(ammoTextScale)));
            RendererUtil::AddRenderItems(renderItems, TextBlitter::CreateText(clipText, ammoClipTextLocation, viewportSize, Alignment::TOP_RIGHT, BitmapFontType::AMMO_NUMBERS, glm::vec3(ammoTextScale)));
        }
    }

    return renderItems;
}


void Player::GiveDamageColor() {
    _damageColorTimer = 0.0f;
}