#pragma once
#include "../Types/SkinnedModel.h"
#include "Ragdoll.h"

struct MeshRenderingEntry {
    std::string meshName;
    int materialIndex = 0;
    int emissiveColorTexutreIndex = -1;
    bool blendingEnabled = false;
    bool drawingEnabled = true;
    bool renderAsGlass = false;
    int meshIndex = -1;
};

struct AnimatedGameObject {

    enum AnimationMode { BINDPOSE, ANIMATION, RAGDOLL };

	void Update(float deltaTime);
	void SetName(std::string name);
	void SetSkinnedModel(std::string skinnedModelName);
	void SetScale(float scale);
	void SetPosition(glm::vec3 position);
	void SetRotationX(float rotation);
	void SetRotationY(float rotation);
	void SetRotationZ(float rotation);
	void ToggleAnimationPause();
	void PlayAnimation(std::string animationName, float speed);
	void PlayAndLoopAnimation(std::string animationName, float speed);
    void PauseAnimation();
    void SetAnimationModeToBindPose();
    void SetMeshMaterialByMeshName(std::string meshName, std::string materialName);
    void SetMeshMaterialByMeshIndex(int meshIndex, std::string materialName);
    void SetMeshToRenderAsGlassByMeshIndex(std::string materialName);
    void SetMeshEmissiveColorTextureByMeshName(std::string meshName, std::string textureName);
	void SetAllMeshMaterials(std::string materialName);
    void UpdateBoneTransformsFromBindPose();
    void UpdateBoneTransformsFromRagdoll();
	glm::mat4 GetBoneWorldMatrixFromBoneName(std::string name);

	std::string GetName();
	glm::mat4 GetModelMatrix();
	bool IsAnimationComplete(); 
	bool AnimationIsPastPercentage(float percent);
    glm::vec3 GetScale();

	SkinnedModel* _skinnedModel = nullptr;
	Transform _transform;
	AnimatedTransforms _animatedTransforms;
	float _currentAnimationTime = 0;
	glm::mat4 _cameraMatrix = glm::mat4(1);
	std::vector<MeshRenderingEntry> _meshRenderingEntries;
    AnimationMode _animationMode = BINDPOSE;
    Ragdoll _ragdoll;

	// Hacky shit
	//glm::vec3 GetGlockBarrelPostion();
	glm::vec3 GetGlockCasingSpawnPostion();
	glm::vec3 GetAKS74UBarrelPostion();
	glm::vec3 GetShotgunBarrelPosition();
	glm::vec3 GetAK74USCasingSpawnPostion();

    void LoadRagdoll(std::string filename, PxU32 ragdollCollisionFlags);
    void SetAnimatedModeToRagdoll();
    void DestroyRagdoll();
    void EnableDrawingForAllMesh();
    void EnableDrawingForMeshByMeshName(std::string meshName);
    void DisableDrawingForMeshByMeshName(std::string meshName); 
    void PrintMeshNames();
    void EnableBlendingByMeshIndex(int index);

    std::vector<glm::mat4> _debugTransformsA;
    std::vector<glm::mat4> _debugTransformsB;
    bool _hasRagdoll = false;

    struct BoneDebugInfo {
        const char* name;
        const char* parentName;
        glm::vec3 worldPos;
        glm::vec3 parentWorldPos;
    };
    std::vector<BoneDebugInfo> _debugBoneInfo;
    bool _renderDebugBones = false;

    struct JointWorldMatrix {
        const char* name;
        glm::mat4 worldMatrix;
    };

    glm::vec3 FindClosestParentAnimatedNode(std::vector<JointWorldMatrix>& worldMatrices, int parentIndex);

    void UpdateRenderItems();
    std::vector<RenderItem3D>& GetRenderItems();

private:

	void UpdateAnimation(float deltaTime);
	void CalculateBoneTransforms();	

	Animation* _currentAnimation = nullptr;
	bool _loopAnimation = false;
	bool _animationPaused = false;
	float _animationSpeed = 1.0f;
	std::string _name;
	bool _animationIsComplete = true;
    std::vector<RenderItem3D> renderItems;


};