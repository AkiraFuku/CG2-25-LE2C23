#pragma once
#include "Vector2.h"
#include "Vector4.h"
#include <list>
#include <random>
#include <unordered_map>

#include "Camera.h"
#include "DXCommon.h"
#include "DrawFunction.h"
#include "d3d12.h"
#include <cstdint>
#include <wrl/client.h>

// 外部
class SrvManager;
class TextureManager;
class PSOMnager;

class BubbleParticle {
public:
  struct MaterialData {
    std::string textureFilePath;
    uint_fast16_t textureIndex = 0;
  };

  struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
  };

  struct Material {
    Vector4 color;
    int32_t enableLighting;
    float padding[3];
    Matrix4x4 uvTransform;
  };

  struct Bubble {
    Transform transfom;
    Vector3 velocity;
    Vector4 color;
    float lifeTime = 1.0f;
    float currentTime = 0.0f;

    // 泡っぽさ
    float phase = 0.0f;
    float amp = 0.0f;
    float freq = 0.0f;
    float startScale = 1.0f;
    float endScale = 1.0f;
  };

  struct ParticleForGPU {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;
  };

  struct Group {
    MaterialData materialData;
    std::list<Bubble> bubbles;

    uint32_t instancingSrvIndex = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
    ParticleForGPU *instancingData = nullptr;

    uint32_t kNumInstance = 0;
  };

public:
  void Initialize();
  void Update();
  void Draw();

  // グループ管理
  void CreateGroup(const std::string &name, const std::string &textureFilepath);
  void ReleaseGroup(const std::string &name);

  // 泡を「ぶわぁ」っと出す
  void Emit(const std::string &name, const Vector3 &position, uint32_t count);

  void SetCamera(Camera *camera) { camera_ = camera; }

  static BubbleParticle *GetInstance();
  void Finalize();

  BubbleParticle() = default;
  ~BubbleParticle() = default;
private:
  BubbleParticle(BubbleParticle &) = delete;
  BubbleParticle &operator=(BubbleParticle &) = delete;

  static std::unique_ptr<BubbleParticle> instance_;
  static uint32_t kMaxNumInstance_;

private:
  // 内部ユーティリティ
  void CreateVertexBuffer();
  void CreateMaterialBuffer();

private:
  std::unordered_map<std::string, Group> groups_;

  std::random_device seedGen_;
  std::mt19937 randomEngine_;

  Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
  Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

  Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
  VertexData *vertexData_ = nullptr;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

  Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
  Material *materialData_ = nullptr;

  Camera *camera_ = nullptr;
};
