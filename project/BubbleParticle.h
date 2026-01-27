#pragma once
#include "Camera.h"
#include "DXCommon.h"
#include "DrawFunction.h"
#include "Vector2.h"
#include "Vector4.h"
#include "PSOMnager.h"

#include <cstdint>
#include <d3d12.h>
#include <list>
#include <random>
#include <string>
#include <wrl/client.h>

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

  struct Particle {
    Transform transfom;
    Vector3 velocity;
    Vector3 accel;
    Vector4 color;
    float lifeTime;
    float currentTime;
    float angularVelocityZ;
  };

  struct ParticleForGPU {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;
  };

public:

  static BubbleParticle *GetInstance();
  static void Shutdown();

  void Initialize(const std::string &textureFilePath,
                  uint32_t maxInstance = 1024);
  void Finalize();

  void SetCamera(Camera *camera) { camera_ = camera; }

  void Update();
  void Draw();

  // 位置指定で発生（シーン側から自由に呼べる）
  void Emit(const Vector3 &position, uint32_t count);

  // 「下から湧く泡」用：範囲指定でX/Zを散らして発生
  void EmitFromBottom(float y, float xMin, float xMax, float zMin, float zMax,
                      uint32_t count);

  // 全消し（シーン切り替え等で便利）
  void Clear();

  BubbleParticle() = default;
  ~BubbleParticle() = default;

private:
  BubbleParticle(const BubbleParticle &) = delete;
  BubbleParticle &operator=(const BubbleParticle &) = delete;

private:
  void CreateVertexBuffer();
  void CreateMaterialBuffer();

private:
  static std::unique_ptr<BubbleParticle> instance_;

  static uint32_t kMaxNumInstance_;
  uint32_t numInstance_ = 0;

  std::random_device seedGen_;
  std::mt19937 randomEngine_;

  // PSO / RootSignature は ParticleManager と同じ PSOMnager を使う想定
  Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
  Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

  // vertex
  Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
  VertexData *vertexData_ = nullptr;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

  // material
  Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
  Material *materialData_ = nullptr;

  // instancing
  uint32_t instancingSrvIndex_ = 0;
  Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
  ParticleForGPU *instancingData_ = nullptr;

  // texture
  MaterialData materialTex_{};

  // particles
  std::list<Particle> particles_;

  // camera
  Camera *camera_ = nullptr;
};
