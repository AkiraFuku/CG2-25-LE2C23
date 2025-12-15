#include "ParticleManager.h"
#include "Logger.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "MassFunction.h"
#include <numbers>
#pragma once
ParticleManager* ParticleManager::instance = nullptr;
uint32_t ParticleManager::kMaxNumInstance = 1024;
void ParticleManager::Initialize(DXCommon* dxCommon, SrvManager* srvManager) {
    //DXCommonとSRVマネージャーの受け取り
    dxCommon_ = dxCommon;
    srvManager_ = srvManager;
    //ランダムエンジンの初期化
    randomEngine_.seed(seedGen_());
    //パイプラインステート生成
    CreatePSO();
    //頂点データの初期化（座標等）
    //頂点リソース生成
    //頂点バッファビュー（VBV）を作成
    //頂点リソースにデータを書き込む
    CreateVertexBuffer();
    CreateMaterialBuffer();
}
ParticleManager* ParticleManager::GetInstance() {
    if (instance == nullptr)
    {
        instance = new ParticleManager;
    }
    return instance;

};
void ParticleManager::Finalize() {

    delete instance;
    instance = nullptr;
}

void ParticleManager::Update() {
    Matrix4x4 backFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
    //ビルボード行列計算
    Matrix4x4 billboardMatrix = Multiply(backFrontMatrix, camera_->GetWorldMatrix());
    billboardMatrix.m[3][0] = 0.0f;
    billboardMatrix.m[3][1] = 0.0f;
    billboardMatrix.m[3][2] = 0.0f;
    //カメラからビューとプロジェクション行列
    Matrix4x4 viewMatrix = camera_->GetViewMatrix();
    Matrix4x4 projectionMatrix = camera_->GetProjectionMatirx();
    Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
    //
    for (auto& [key, particleGroup] : particleGroups)
    {
        uint32_t  numInstance = 0;
        for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin();
            particleIterator != particleGroup.particles.end();
            )
        {




            if ((*particleIterator).lifeTime <= (*particleIterator).currentTime)
            {
                particleIterator = particleGroup.particles.erase(particleIterator);
                continue;
            }

            float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
            (*particleIterator).transfom.translate += (*particleIterator).velocity * DXCommon::kDeltaTime;
            (*particleIterator).currentTime += DXCommon::kDeltaTime;

            if (numInstance < kMaxNumInstance)
            {

                particleGroup.instancingData[numInstance].color.w = alpha;
                Matrix4x4 worldMatrix = {};
                /*  if (isBillboard)
                  {*/
                (*particleIterator).transfom.rotate.z += 1.0f / 60.0f;


                worldMatrix = MakeBillboardMatrix((*particleIterator).transfom.scale, (*particleIterator).transfom.rotate, billboardMatrix, (*particleIterator).transfom.translate);

                /*  } else
                  {
                      worldMatrixInstance = MakeAfineMatrix((*particleIterator).transfom.scale, (*particleIterator).transfom.rotate, (*particleIterator).transfom.traslate);

                  }*/
                particleGroup.instancingData[numInstance].WVP = Multiply(worldMatrix, viewProjectionMatrix);
                particleGroup.instancingData[numInstance].color.x = (*particleIterator).color.x;
                particleGroup.instancingData[numInstance].color.y = (*particleIterator).color.y;
                particleGroup.instancingData[numInstance].color.z = (*particleIterator).color.z;
                ++numInstance;
            }
            ++particleIterator;

        }
        particleGroup.numInstance = numInstance;
    }
}
void ParticleManager::Draw() {
    // RootSignatureの設定
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
    //PSOの設定
    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    //VBVの設定
    dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);

    for (auto& [key, particleGroup] : particleGroups) {
        if (particleGroup.numInstance > 0) {

            // とりあえずコードの意図を汲んで修正すると：
            dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_.Get()->GetGPUVirtualAddress());

            // [1] Descriptor Table (Instancing Data): インスタンシング用SRV
            dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(particleGroup.instancingSRVIndex));

            // [2] Descriptor Table (Texture): テクスチャ用SRV
            dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(particleGroup.materialData.textureIndex));
            // DrawCall
            // 後述するトポロジーの修正に合わせて頂点数を変更 (6 -> 4)
            dxCommon_->GetCommandList()->DrawInstanced(4, particleGroup.numInstance, 0, 0);
        }
    }
}

void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilepath)
{
    assert(!particleGroups.contains(name));
    //
    ParticleGroup& newParticle = particleGroups[name];
    newParticle.materialData.textureFilePath = textureFilepath;
    newParticle.numInstance = kMaxNumInstance;
    newParticle.materialData.textureIndex = newParticle.materialData.textureIndex =
        TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilepath);
    newParticle.instancingResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * newParticle.numInstance);

    newParticle.instancingSRVIndex = srvManager_->AllocateSRV();

    srvManager_->CreateSRVforStructuredBuffer(
        newParticle.instancingSRVIndex,
        newParticle.instancingResource.Get(),
        newParticle.numInstance,
        sizeof(ParticleForGPU)
    );
    newParticle.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&newParticle.instancingData));
    for (uint32_t i = 0; i < newParticle.numInstance; ++i) {
        newParticle.instancingData[i].WVP = Makeidetity4x4(); // 単位行列などで埋める
        newParticle.instancingData[i].color = { 1.0f, 1.0f, 1.0f, 0.0f };
    }
}

void ParticleManager::Emit(const std::string name, const Vector3& postion, uint32_t count)
{
    assert(particleGroups.contains(name));

    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distTime(1.0f, 10.0f);

    for (uint32_t i = 0; i < count; ++i)
    {
        Particle particle;
        particle.transfom.scale = { 1.0f,1.0f,1.0f };
        particle.transfom.rotate = { 0.0f,0.0f,0.0f };
        Vector3 randamTranslate = { distribution(randomEngine_),distribution(randomEngine_) ,distribution(randomEngine_) };
        particle.transfom.translate = postion + randamTranslate;
        particle.velocity = { distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_) };

        particle.color = { distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_),1.0f };

        particle.lifeTime = distTime(randomEngine_);
        particle.currentTime = 0.0f;
        particleGroups[name].particles.push_back(particle);
    }

}

void ParticleManager::CreateRootSignature()
{
    // 1. ディスクリプタレンジの設定
// --------------------------------------------------------

// [Range 0] インスタンシング用 (Vertex Shader: t0)
    D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
    descriptorRangeForInstancing[0].BaseShaderRegister = 0; // t0
    descriptorRangeForInstancing[0].NumDescriptors = 1;     // 1つ
    descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV
    descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // [Range 1] テクスチャ用 (Pixel Shader: t0)
    // ※VSとPSでステージが違うため、同じ t0 でも問題ありません
    D3D12_DESCRIPTOR_RANGE descriptorRangeForTexture[1] = {};
    descriptorRangeForTexture[0].BaseShaderRegister = 0; // t0
    descriptorRangeForTexture[0].NumDescriptors = 1;     // 1つ
    descriptorRangeForTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV
    descriptorRangeForTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    // 2. ルートパラメータの設定 (計4つ)
    // --------------------------------------------------------
    D3D12_ROOT_PARAMETER rootParameters[4] = {};

    // [Param 0] 定数バッファ (Pixel Shader: b0) - マテリアル設定など
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0; // b0

    // [Param 1] ディスクリプタテーブル (Vertex Shader: t0) - インスタンシング行列データ
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // ★重要: VSで見えるようにする
    rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);

    // [Param 2] ディスクリプタテーブル (Pixel Shader: t0) - テクスチャ
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeForTexture;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForTexture);

    // [Param 3] 定数バッファ (Pixel Shader: b1) - ライトやカメラ情報など
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 1; // b1


    // 3. 静的サンプラーの設定 (s0)
    // --------------------------------------------------------
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0; // s0
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


    // 4. ルートシグネチャ記述の作成
    // --------------------------------------------------------
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature = {};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


    // 5. シリアライズと生成
    // --------------------------------------------------------
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    hr_ = D3D12SerializeRootSignature(
        &descriptionRootSignature,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob
    );

    if (FAILED(hr_)) {
        if (errorBlob) {
            // ログ出力関数に合わせて適宜変更してください
            Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        }
        assert(false && "Root Signature Serialization Failed");
    }



    hr_ = dxCommon_->GetDevice()->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_)
    );

    assert(SUCCEEDED(hr_) && "Root Signature Creation Failed");

}
void ParticleManager::CreatePSO() {
    CreateRootSignature();
    //InputLayoutの設定
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    //
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    //
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    // BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
   /* blendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;*/
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
blendDesc.RenderTarget[0].BlendEnable = TRUE; // ブレンドを有効にする
blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE; // 加算合成
blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    //RasteriwrStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;//カリングなし
    //BACK;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    //shaderのコンパイル
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(
        L"resources/shaders/Particle/Particle.vs.hlsl",
        L"vs_6_0"
    );
    assert(vertexShaderBlob.Get() != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(
        L"resources/shaders/Particle/Particle.ps.hlsl",
        L"ps_6_0"

    );
    assert(pixelShaderBlob.Get() != nullptr);


    //DSSの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;//深度テストを有効にする
    //書き込み
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    //比較関数
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    //PSOの生成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
    graphicPipelineStateDesc.pRootSignature = rootSignature_.Get();
    graphicPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
    vertexShaderBlob->GetBufferSize() };//ヴァーテックスシェーダー
    graphicPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
    pixelShaderBlob->GetBufferSize() };//ピクセルシェーダー
    graphicPipelineStateDesc.BlendState = blendDesc;//ブレンドステート
    graphicPipelineStateDesc.RasterizerState = rasterizerDesc;//ラスタライザーステート
    ///巻き込むRTV
    graphicPipelineStateDesc.NumRenderTargets = 1;
    graphicPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    ///トポロジー
    graphicPipelineStateDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    //カラー
    graphicPipelineStateDesc.SampleDesc.Count = 1;
    graphicPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    //深度ステンシルビューの設定
    graphicPipelineStateDesc.DepthStencilState = depthStencilDesc;//PSOにDSSを設定
    graphicPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;//深度ステンシルビューのフォーマット
    ////
    //PSOの生成
    hr_ = dxCommon_->GetDevice()->CreateGraphicsPipelineState(
        &graphicPipelineStateDesc,
        IID_PPV_ARGS(&graphicsPipelineState_)
    );
    assert(SUCCEEDED(hr_));
}
void ParticleManager::CreateVertexBuffer() {
    VertexData vertices[] = {
        // Position(x,y,z,w)             TexCoord(u,v)   Normal(x,y,z)
        {{-1.0f,  1.0f, 0.0f, 1.0f},     {0.0f, 0.0f},   {0.0f, 0.0f, 1.0f}}, // 左上
        {{ 1.0f,  1.0f, 0.0f, 1.0f},     {1.0f, 0.0f},   {0.0f, 0.0f, 1.0f}}, // 右上
        {{-1.0f, -1.0f, 0.0f, 1.0f},     {0.0f, 1.0f},   {0.0f, 0.0f, 1.0f}}, // 左下
        {{ 1.0f, -1.0f, 0.0f, 1.0f},     {1.0f, 1.0f},   {0.0f, 0.0f, 1.0f}}, // 右下
    };
    size_t sizeIB = sizeof(vertices);
    //頂点リソースの作成
    vertexResourse_ =
        dxCommon_->
        CreateBufferResource(sizeIB);
    //頂点バッファビューの設定
    vertexBufferView_.BufferLocation =
        vertexResourse_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = UINT(sizeIB);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    vertexResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    //頂点データの転送
    memcpy(vertexData_, vertices, sizeIB);

}

void ParticleManager::CreateMaterialBuffer()
{
    //データの設定

    materialResource_ =
        dxCommon_->
        CreateBufferResource(sizeof(Material));
    materialResource_->
        Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData_->enableLighting = false;
    materialData_->uvTransform = Makeidetity4x4();
}
