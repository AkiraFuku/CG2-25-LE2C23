#include "BubbleParticle.h"

#include "Logger.h"
#include "MathFunction.h"

#include "SrvManager.h"
#include "TextureManager.h"

#include <algorithm>
#include <cassert>
#include <numbers>

std::unique_ptr<BubbleParticle> BubbleParticle::instance_;
uint32_t BubbleParticle::kMaxNumInstance_ = 1024;

BubbleParticle *BubbleParticle::GetInstance() {
  if (!instance_) {
    instance_.reset(new BubbleParticle());
  }
  return instance_.get();
}

void BubbleParticle::Shutdown() {
  if (instance_) {
    instance_->Finalize();
    instance_.reset();
  }
}

void BubbleParticle::Initialize(const std::string &textureFilePath,
                                uint32_t maxInstance) {
  randomEngine_.seed(seedGen_());

  kMaxNumInstance_ = std::max<uint32_t>(1, maxInstance);

  // ParticleManager と同じ PSO を使う（Add にして泡を映える感じに）
  // ParticleManager と同じ PSO を使う（Add にして泡を映える感じに）
  PsoProperty psoProp{};
  psoProp.type = PipelineType::Particle;
  psoProp.blendMode = BlendMode::Add;
  psoProp.depthMode = DepthMode::NoDepth; // ★これ
  // psoProp.fillMode = FillMode::Solid;    // 必要なら明示

  const PsoSet &psoset = PSOMnager::GetInstance()->GetPsoSet(psoProp);
  graphicsPipelineState_ = psoset.pipelineState;
  rootSignature_ = psoset.rootSignature;

  CreateVertexBuffer();
  CreateMaterialBuffer();

  // texture
  materialTex_.textureFilePath = textureFilePath;
  materialTex_.textureIndex =
      TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

  // instancing buffer
  instancingResource_ = DXCommon::GetInstance()->CreateBufferResource(
      sizeof(ParticleForGPU) * kMaxNumInstance_);

  instancingSrvIndex_ = SrvManager::GetInstance()->AllocateSRV();
  SrvManager::GetInstance()->CreateSRVforStructuredBuffer(
      instancingSrvIndex_, instancingResource_.Get(), kMaxNumInstance_,
      sizeof(ParticleForGPU));

  instancingResource_->Map(0, nullptr,
                           reinterpret_cast<void **>(&instancingData_));
  for (uint32_t i = 0; i < kMaxNumInstance_; ++i) {
    instancingData_[i].WVP = Makeidetity4x4();
    instancingData_[i].World = Makeidetity4x4();
    instancingData_[i].color = {1.0f, 1.0f, 1.0f, 0.0f};
  }

  particles_.clear();
  numInstance_ = 0;
}

void BubbleParticle::Finalize() {
  particles_.clear();
  numInstance_ = 0;
  // ComPtr は勝手に解放される
}

void BubbleParticle::Clear() {
  particles_.clear();
  numInstance_ = 0;
}

void BubbleParticle::Update() {
  if (!camera_) {
    return;
  }

  // ビルボード行列計算などはそのまま
  Matrix4x4 backFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
  Matrix4x4 billboardMatrix =
      Multiply(backFrontMatrix, camera_->GetWorldMatrix());
  // ... (省略) ...

  Matrix4x4 viewMatrix = camera_->GetViewMatrix();
  Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();
  Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

  numInstance_ = 0;

  for (auto it = particles_.begin(); it != particles_.end();) {

    // 1. 寿命チェック（既存）
    if (it->lifeTime <= it->currentTime) {
      it = particles_.erase(it);
      continue;
    }

    // 移動更新
    it->velocity += it->accel * DXCommon::kDeltaTime;
    it->transfom.translate += it->velocity * DXCommon::kDeltaTime;
    it->transfom.rotate.z += it->angularVelocityZ * DXCommon::kDeltaTime;
    it->currentTime += DXCommon::kDeltaTime;

    // ---------------------------------------------------------
    // ★追加: 画面外判定 (NDC座標変換)
    // ---------------------------------------------------------
    // ワールド座標
    Vector3 pos = it->transfom.translate;
    // ビュープロジェクション変換 (w成分も計算)
    float clipX = pos.x * viewProjectionMatrix.m[0][0] +
                  pos.y * viewProjectionMatrix.m[1][0] +
                  pos.z * viewProjectionMatrix.m[2][0] +
                  viewProjectionMatrix.m[3][0];
    float clipY = pos.x * viewProjectionMatrix.m[0][1] +
                  pos.y * viewProjectionMatrix.m[1][1] +
                  pos.z * viewProjectionMatrix.m[2][1] +
                  viewProjectionMatrix.m[3][1];
    float clipW = pos.x * viewProjectionMatrix.m[0][3] +
                  pos.y * viewProjectionMatrix.m[1][3] +
                  pos.z * viewProjectionMatrix.m[2][3] +
                  viewProjectionMatrix.m[3][3];

    // ゼロ除算対策と画面外判定
    if (clipW > 0.0001f) {
      float ndcY = clipY / clipW;

      // 画面上端(1.0)より少し上(+0.2fなどのマージン)に行ったら消す
      // これにより、Fade側で IsEmpty() が早くtrueになる
      if (ndcY > 1.2f) {
        it = particles_.erase(it);
        continue;
      }
    }
    // ---------------------------------------------------------

    float t = (it->currentTime / it->lifeTime);
    float alpha = 1.0f - t;
    alpha = std::clamp(alpha, 0.0f, 1.0f);

    if (numInstance_ < kMaxNumInstance_) {
      // ... インスタンシングデータの登録 (既存のまま) ...
      Matrix4x4 worldMatrix =
          MakeBillboardMatrix(it->transfom.scale, it->transfom.rotate,
                              billboardMatrix, it->transfom.translate);

      instancingData_[numInstance_].WVP =
          Multiply(worldMatrix, viewProjectionMatrix);
      instancingData_[numInstance_].World = worldMatrix;

      instancingData_[numInstance_].color = it->color;
      instancingData_[numInstance_].color.w = alpha; // Alpha更新

      ++numInstance_;
    }

    ++it;
  }
}
void BubbleParticle::Draw() {
  if (numInstance_ == 0) {
    return;
  }

  DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
  DXCommon::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());
  DXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  DXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);

  DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(
      0, materialResource_.Get()->GetGPUVirtualAddress());

  // [1] instancing SRV
  DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(
      1,
      SrvManager::GetInstance()->GetGPUDescriptorHandle(instancingSrvIndex_));

  // [2] texture SRV
  DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(
      2, SrvManager::GetInstance()->GetGPUDescriptorHandle(
             materialTex_.textureIndex));

  DXCommon::GetInstance()->GetCommandList()->DrawInstanced(4, numInstance_, 0,
                                                           0);
}

void BubbleParticle::Emit(const Vector3 &position, uint32_t count) {
  std::uniform_real_distribution<float> dx(-0.4f, 0.4f);
  std::uniform_real_distribution<float> dz(-0.4f, 0.4f);
  std::uniform_real_distribution<float> dy(-0.4f, 0.4f);

  // 泡：上向き（Y+）メイン
  std::uniform_real_distribution<float> vy(0.8f, 2.2f);
  std::uniform_real_distribution<float> vx(-0.25f, 0.25f);
  std::uniform_real_distribution<float> vz(-0.25f, 0.25f);

  // 弱い上向き加速 + 横揺れ
  std::uniform_real_distribution<float> ax(-0.08f, 0.08f);
  std::uniform_real_distribution<float> ay(0.05f, 0.20f);
  std::uniform_real_distribution<float> az(-0.08f, 0.08f);

  std::uniform_real_distribution<float> life(0.8f, 2.5f);
  std::uniform_real_distribution<float> scale(0.25f, 0.45f);
  std::uniform_real_distribution<float> ang(-2.5f, 2.5f);

  for (uint32_t i = 0; i < count; ++i) {
    Particle p{};
    p.transfom.scale = {scale(randomEngine_), scale(randomEngine_), 1.0f};
    p.transfom.rotate = {0.0f, 0.0f, 0.0f};

    p.transfom.translate =
        position + Vector3{dx(randomEngine_), dy(randomEngine_), dz(randomEngine_)};

    p.velocity = {vx(randomEngine_), vy(randomEngine_), vz(randomEngine_)};
    p.accel = {ax(randomEngine_), ay(randomEngine_), az(randomEngine_)};

    // 泡っぽい色（白寄りでOK。ここは好みで変えて）
    p.color = {1.0f, 1.0f, 1.0f, 1.0f};

    p.lifeTime = life(randomEngine_);
    p.currentTime = 0.0f;

    p.angularVelocityZ = ang(randomEngine_);

    particles_.push_back(p);
  }
}

void BubbleParticle::EmitFromBottom(float y, float xMin, float xMax, float zMin,
                                    float zMax, uint32_t count) {
  std::uniform_real_distribution<float> rx(xMin, xMax);
  std::uniform_real_distribution<float> rz(zMin, zMax);

  for (uint32_t i = 0; i < count; ++i) {
    Emit(Vector3{rx(randomEngine_), y, rz(randomEngine_)}, 1);
  }
}

void BubbleParticle::CreateVertexBuffer() {
  VertexData vertices[] = {
      {{-1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
      {{1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
      {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
      {{1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
  };

  size_t sizeVB = sizeof(vertices);

  vertexResourse_ = DXCommon::GetInstance()->CreateBufferResource(sizeVB);

  vertexBufferView_.BufferLocation = vertexResourse_->GetGPUVirtualAddress();
  vertexBufferView_.SizeInBytes = UINT(sizeVB);
  vertexBufferView_.StrideInBytes = sizeof(VertexData);

  vertexResourse_->Map(0, nullptr, reinterpret_cast<void **>(&vertexData_));
  memcpy(vertexData_, vertices, sizeVB);
}

void BubbleParticle::CreateMaterialBuffer() {
  materialResource_ =
      DXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
  materialResource_->Map(0, nullptr, reinterpret_cast<void **>(&materialData_));

  materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  materialData_->enableLighting = false;
  materialData_->uvTransform = Makeidetity4x4();
}
