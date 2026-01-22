#include "BubbleParticle.h"

#include "MathFunction.h"
#include "PSOMnager.h"
#include "SrvManager.h"
#include "TextureManager.h"

#include <cassert>
#include <cmath>
#include <numbers>

std::unique_ptr<BubbleParticle> BubbleParticle::instance_;
uint32_t BubbleParticle::kMaxNumInstance_ = 1024;

BubbleParticle *BubbleParticle::GetInstance() {
  if (!instance_) {
    instance_.reset(new BubbleParticle());
  }
  return instance_.get();
}

void BubbleParticle::Finalize() { instance_.reset(); }

void BubbleParticle::Initialize() {
  randomEngine_.seed(seedGen_());

  // ParticleManagerと同じPSOを使う（Addで泡っぽく）
  PsoProperty pso = {PipelineType::Particle, BlendMode::Add};
  PsoSet psoset = PSOMnager::GetInstance()->GetPsoSet(pso);
  graphicsPipelineState_ = psoset.pipelineState;
  rootSignature_ = psoset.rootSignature;

  CreateVertexBuffer();
  CreateMaterialBuffer();
}

void BubbleParticle::CreateGroup(const std::string &name,
                                 const std::string &textureFilepath) {
  assert(!groups_.contains(name));

  Group &g = groups_[name];
  g.materialData.textureFilePath = textureFilepath;
  g.materialData.textureIndex =
      TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilepath);

  g.instancingResource = DXCommon::GetInstance()->CreateBufferResource(
      sizeof(ParticleForGPU) * kMaxNumInstance_);
  g.instancingSrvIndex = SrvManager::GetInstance()->AllocateSRV();

  SrvManager::GetInstance()->CreateSRVforStructuredBuffer(
      g.instancingSrvIndex, g.instancingResource.Get(), kMaxNumInstance_,
      sizeof(ParticleForGPU));

  g.instancingResource->Map(0, nullptr,
                            reinterpret_cast<void **>(&g.instancingData));

  for (uint32_t i = 0; i < kMaxNumInstance_; ++i) {
    g.instancingData[i].WVP = Makeidetity4x4();
    g.instancingData[i].World = Makeidetity4x4();
    g.instancingData[i].color = {1, 1, 1, 0};
  }
}

void BubbleParticle::ReleaseGroup(const std::string &name) {
  groups_.erase(name);
}

void BubbleParticle::Emit(const std::string &name, const Vector3 &position,
                          uint32_t count) {
  assert(groups_.contains(name));
  Group &g = groups_[name];

  // 「下から上へぶわぁ」
  std::uniform_real_distribution<float> offX(-2.6f, 2.6f);
  std::uniform_real_distribution<float> offZ(-0.6f, 0.6f);
  std::uniform_real_distribution<float> offY(-0.2f, 0.2f);

  // 上昇は強め、横は弱め
  std::uniform_real_distribution<float> vx(-0.10f, 0.10f);
  std::uniform_real_distribution<float> vy(1.2f, 3.0f); // ここが「ぶわぁ」感
  std::uniform_real_distribution<float> vz(-0.10f, 0.10f);

  // 寿命短め
  std::uniform_real_distribution<float> life(1.6f, 3.2f);

  // 大小さまざま
  std::uniform_real_distribution<float> s0(0.10f, 0.45f);
  std::uniform_real_distribution<float> sMul(1.1f, 1.8f);

  // ゆらゆら
  std::uniform_real_distribution<float> phase(0.0f, 6.283185f);
  std::uniform_real_distribution<float> amp(0.15f, 0.45f);
  std::uniform_real_distribution<float> freq(6.0f, 12.0f);

  // 色（泡は白寄り・薄め）
  std::uniform_real_distribution<float> a(0.20f, 0.55f);
  std::uniform_real_distribution<float> c(0.92f, 1.0f);

  for (uint32_t i = 0; i < count; ++i) {
    Bubble b{};
    b.transfom.rotate = {0, 0, 0};

    b.transfom.translate =
        position +
        Vector3{offX(randomEngine_), offY(randomEngine_), offZ(randomEngine_)};
    b.velocity = {vx(randomEngine_), vy(randomEngine_), vz(randomEngine_)};

    float start = s0(randomEngine_);
    b.startScale = start;
    b.endScale = start * sMul(randomEngine_);
    b.transfom.scale = {start, start, start};

    b.color = {c(randomEngine_), c(randomEngine_), c(randomEngine_),
               a(randomEngine_)};

    b.lifeTime = life(randomEngine_);
    b.currentTime = 0.0f;

    b.phase = phase(randomEngine_);
    b.amp = amp(randomEngine_);
    b.freq = freq(randomEngine_);

    g.bubbles.push_back(b);
  }
}

void BubbleParticle::Update() {
  assert(camera_);

  Matrix4x4 backFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
  Matrix4x4 billboardMatrix =
      Multiply(backFrontMatrix, camera_->GetWorldMatrix());
  billboardMatrix.m[3][0] = 0.0f;
  billboardMatrix.m[3][1] = 0.0f;
  billboardMatrix.m[3][2] = 0.0f;

  Matrix4x4 viewMatrix = camera_->GetViewMatrix();
  Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();
  Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

  const float dt = DXCommon::kDeltaTime;

  for (auto &[key, g] : groups_) {
    uint32_t numInstance = 0;

    for (auto it = g.bubbles.begin(); it != g.bubbles.end();) {
      if (it->lifeTime <= it->currentTime) {
        it = g.bubbles.erase(it);
        continue;
      }

      it->currentTime += dt;
      float t = it->currentTime / it->lifeTime; // 0..1

      // 移動（上へ）
      it->transfom.translate += it->velocity * dt;

      // ゆらゆら（大きめに効かせると泡っぽい）
      float w = std::sin(it->phase + it->currentTime * it->freq);
      float w2 = std::cos(it->phase + it->currentTime * it->freq);
      it->transfom.translate.x += w * it->amp * dt;
      it->transfom.translate.z += w2 * it->amp * dt;

      // 末期に向けて「縮小」して消える（透明化はしない）
      float shrink = 1.0f;
      if (t > 0.65f) {
        float k = (t - 0.65f) / 0.35f; // 0..1
        if (k > 1.0f)
          k = 1.0f;
        shrink = 1.0f - k; // 1 -> 0
      }

      // スケール：膨らむ → 最後に縮む
      float sGrow = it->startScale + (it->endScale - it->startScale) * t;
      float scale = sGrow * shrink;
      it->transfom.scale = {scale, scale, scale};

      // αは固定（消し方は縮小に任せる）
      float alpha = it->color.w;


      if (numInstance < kMaxNumInstance_) {
        // ほんの少し回転
        it->transfom.rotate.z += 0.6f * dt;

        Matrix4x4 worldMatrix =
            MakeBillboardMatrix(it->transfom.scale, it->transfom.rotate,
                                billboardMatrix, it->transfom.translate);

        g.instancingData[numInstance].World = worldMatrix;
        g.instancingData[numInstance].WVP =
            Multiply(worldMatrix, viewProjectionMatrix);

        g.instancingData[numInstance].color = it->color;
        g.instancingData[numInstance].color.w = alpha;

        ++numInstance;
      }

      ++it;
    }

    g.kNumInstance = numInstance;
  }
}

void BubbleParticle::Draw() {
  DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootSignature(
      rootSignature_.Get());
  DXCommon::GetInstance()->GetCommandList()->SetPipelineState(
      graphicsPipelineState_.Get());
  DXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(
      D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  DXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(
      0, 1, &vertexBufferView_);

  for (auto &[key, g] : groups_) {
    if (g.kNumInstance == 0)
      continue;

    DXCommon::GetInstance()
        ->GetCommandList()
        ->SetGraphicsRootConstantBufferView(
            0, materialResource_.Get()->GetGPUVirtualAddress());

    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(
        1, SrvManager::GetInstance()->GetGPUDescriptorHandle(
               g.instancingSrvIndex));

    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(
        2, SrvManager::GetInstance()->GetGPUDescriptorHandle(
               g.materialData.textureIndex));

    DXCommon::GetInstance()->GetCommandList()->DrawInstanced(4, g.kNumInstance,
                                                             0, 0);
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

  vertexBufferView_.BufferLocation =
      vertexResourse_.Get()->GetGPUVirtualAddress();
  vertexBufferView_.SizeInBytes = UINT(sizeVB);
  vertexBufferView_.StrideInBytes = sizeof(VertexData);

  vertexResourse_->Map(0, nullptr, reinterpret_cast<void **>(&vertexData_));
  memcpy(vertexData_, vertices, sizeVB);
}

void BubbleParticle::CreateMaterialBuffer() {
  materialResource_ =
      DXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
  materialResource_->Map(0, nullptr, reinterpret_cast<void **>(&materialData_));
  materialData_->color = Vector4(1, 1, 1, 1);
  materialData_->enableLighting = false;
  materialData_->uvTransform = Makeidetity4x4();
}
