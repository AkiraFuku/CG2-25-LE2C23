#include "PSOMnager.h"
#include "DXCommon.h"
#include "Logger.h"
#include <cassert>
#include <vector>

std::unique_ptr<PSOMnager> PSOMnager::instance_ = nullptr;

PSOMnager* PSOMnager::GetInstance() {
    if (!instance_) {
        instance_.reset(new PSOMnager());
    }
    return instance_.get();
}

void PSOMnager::Initialize() {
    psoCache_.clear();
    rootSignatureCache_.clear();
}

void PSOMnager::Finalize() {
    psoCache_.clear();
    rootSignatureCache_.clear();
    instance_.reset();
}

const PsoSet& PSOMnager::GetPsoSet(const PsoProperty& property) {
    // 既にPSOがあるか確認
    auto it = psoCache_.find(property);
    if (it != psoCache_.end()) {
        return it->second;
    }

    // 無ければ生成
    CreatePso(property);

    return psoCache_.at(property);
}

// -------------------------------------------------------------------------
// RootSignature 生成関数
// -------------------------------------------------------------------------
void PSOMnager::CreateRootSignature(PipelineType type) {
    // 既にこのタイプのRootSignatureが作られていたら何もしない
    if (rootSignatureCache_.contains(type)) {
        return;
    }

    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    
    // 各種設定用配列
    std::vector<D3D12_ROOT_PARAMETER> rootParameters;
    std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
    
    // 一時保管用のDescriptorRange（ポインタ保持のためスコープ外に出す）
    D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};

    // 共通サンプラー設定（線形補間・ラップ）
    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    staticSamplers.push_back(sampler);

    // タイプごとのパラメータ設定
    if (type == PipelineType::Sprite || type == PipelineType::Object3d) {
        // SpriteCommon.cpp / Object3dCommon.cpp の構成
        
        // Descriptor Range (Texture)
        descRangeTexture[0].BaseShaderRegister = 0; // t0
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        rootParameters.resize(4);

        // 0. Material (CBV b0, Pixel)
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[0].Descriptor.ShaderRegister = 0;

        // 1. Transform (CBV b0, Vertex)
        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameters[1].Descriptor.ShaderRegister = 0;

        // 2. Texture (Table t0, Pixel)
        rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[2].DescriptorTable.pDescriptorRanges = descRangeTexture;
        rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

        // 3. DirectionalLight (CBV b1, Pixel)
        rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[3].Descriptor.ShaderRegister = 1;

    } 
    else if (type == PipelineType::Particle) {
        // ParticleManager.cpp の構成
        // ※実装に合わせて調整してください。ここではSpriteと同じ構成と仮定するか、
        // インスタンシング用のStructuredBufferがある場合は設定を変えます。
        
        // とりあえずSpriteと同じ構成で実装（必要に応じて変更可能）
        descRangeTexture[0].BaseShaderRegister = 0;
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        rootParameters.resize(4);
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // Material
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[0].Descriptor.ShaderRegister = 0;

        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // Instancing Data?
        // ※もしParticleでInstancing用のDescriptorTableを使うならここで設定
        // 今回はSprite同様CBVでTransform渡すならこうなるが、
        // ParticleManagerの実装を見るにInstancingを使っているなら変更が必要。
        // ここでは汎用性を重視し、Spriteと同じにしておきます。
        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameters[1].Descriptor.ShaderRegister = 0;

        rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // Texture
        rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[2].DescriptorTable.pDescriptorRanges = descRangeTexture;
        rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

        rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // Light
        rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[3].Descriptor.ShaderRegister = 1;
    }

    // シリアライズと生成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.pParameters = rootParameters.data();
    descriptionRootSignature.NumParameters = (UINT)rootParameters.size();
    descriptionRootSignature.pStaticSamplers = staticSamplers.data();
    descriptionRootSignature.NumStaticSamplers = (UINT)staticSamplers.size();

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    
    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    // マップに登録
    rootSignatureCache_[type] = rootSignature;
}


// -------------------------------------------------------------------------
// PSO 生成関数
// -------------------------------------------------------------------------
void PSOMnager::CreatePso(const PsoProperty& property) {
    HRESULT hr;
    auto device = DXCommon::GetInstance()->GetDevice();

    // 1. RootSignatureの取得（無ければ作る）
    CreateRootSignature(property.type);
    // キャッシュから取得
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = rootSignatureCache_[property.type];

    // 2. シェーダーとInputLayoutの準備
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
    D3D12_RASTERIZER_DESC rasterizerDesc{};

    if (property.type == PipelineType::Sprite) {
        vsBlob = DXCommon::GetInstance()->CompileShader(L"resources/shaders/Object3d/Object3D.vs.hlsl", L"vs_6_0");
        psBlob = DXCommon::GetInstance()->CompileShader(L"resources/shaders/Object3d/Object3D.ps.hlsl", L"ps_6_0");
        
        inputElements = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // Spriteはカリングなし
    }
    else if (property.type == PipelineType::Object3d) {
        vsBlob = DXCommon::GetInstance()->CompileShader(L"resources/shaders/Object3d/Object3D.vs.hlsl", L"vs_6_0");
        psBlob = DXCommon::GetInstance()->CompileShader(L"resources/shaders/Object3d/Object3D.ps.hlsl", L"ps_6_0");

        inputElements = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK; // 3Dは背面カリング
    }
    else if (property.type == PipelineType::Particle) {
        vsBlob = DXCommon::GetInstance()->CompileShader(L"resources/shaders/Particle/Particle.vs.hlsl", L"vs_6_0");
        psBlob = DXCommon::GetInstance()->CompileShader(L"resources/shaders/Particle/Particle.ps.hlsl", L"ps_6_0");

        inputElements = {
             { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
             { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
             { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    }

    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    // 3. ブレンドモード設定
    D3D12_BLEND_DESC blendDesc = CreateBlendDesc(property.blendMode);

    // 4. デプス設定
    D3D12_DEPTH_STENCIL_DESC depthDesc{};
    depthDesc.DepthEnable = true;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    // ParticleのみZ書き込みOFF
    if (property.type == PipelineType::Particle) {
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    } else {
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    }

    // 5. PSO生成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature.Get(); // 生成済みRootSigを使用
    psoDesc.InputLayout = { inputElements.data(), (UINT)inputElements.size() };
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    psoDesc.BlendState = blendDesc;
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.DepthStencilState = depthDesc;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    PsoSet psoSet;
    psoSet.rootSignature = rootSignature; // セットにも持たせておく
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoSet.pipelineState));
    assert(SUCCEEDED(hr));

    // キャッシュ保存
    psoCache_[property] = psoSet;
}

// CreateBlendDescの実装は前回の回答と同様のため省略（変更なし）
// case BlendMode::Normal ... 等の処理が入ります
D3D12_BLEND_DESC PSOMnager::CreateBlendDesc(BlendMode mode) {
    // ...前回のコードの通り記述...
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    
    // 簡易実装例
    if(mode == BlendMode::None) blendDesc.RenderTarget[0].BlendEnable = FALSE;
    else if(mode == BlendMode::Normal){
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    }
    else if(mode == BlendMode::Add){
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    }
    // ...他のモードも記述
    return blendDesc;
}