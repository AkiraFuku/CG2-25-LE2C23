#pragma once
#include "Vector4.h"
#include "Vector2.h"
#include<random>
#include<list>
#include "DrawFunction.h"
#include "DXCommon.h"
#include <wrl/client.h>
#include "d3d12.h"
#include <cstdint>
#include "Camera.h"

class Camera;
class DXCommon;
class SrvManager;
class TextureManager;
class ParticleManager
{
public:
    struct MaterialData {
        std::string textureFilePath;
        uint_fast16_t textureIndex = 0;
    };
    struct VertexData {
        Vector4 position; // 4D position vector
        Vector2 texcoord; // 2D texture coordinate vector
        Vector3 normal;
    };
    struct Particle
    {
        Transform transfom;
        Vector3 velocity;
        Vector4 color;
        float lifeTime;
        float currentTime;

    };

    struct ParticleForGPU
    {
        Matrix4x4 WVP;
        Matrix4x4 World;
        Vector4 color;

    };

    struct Emitter
    {
        Transform transfom;//位置
        uint32_t count;//パーティクル数
        float frequency;//範囲
        float frequencyTime;//発生時間

    };

    struct ParticleGroup
    {
        MaterialData materialData;
        std::list<Particle> particles;
        D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc;
        Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
        uint32_t numInstance;
        ParticleForGPU* instancingData;

    };


    void Initialize(DXCommon* dxCommon, SrvManager* srvManager);
    void 

    void CreateParticleGroup(const std::string name, const std::string textureFilepath);

    void Setcamera(Camera* camera) {
        camera_ = camera;
    }
private:

    ParticleManager() = default;
    ~ParticleManager() = default;
    ParticleManager(ParticleManager&) = delete;
    ParticleManager& operator=(ParticleManager&) = delete;
    static ParticleManager* instance;
    static uint32_t kMaxNumInstance;
    DXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;

    std::random_device seedGen_;
    std::mt19937 randomEngine_;
    HRESULT hr_;
    //ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_;
    void CreateRootSignature();
    //グラフィックパイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

    //頂点リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
    VertexData* vertexData_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    void CreateVertexBuffer();
    void CreatePSO();

    std::unordered_map<std::string, ParticleGroup> particleGroups;

    Camera* camera_;
};

