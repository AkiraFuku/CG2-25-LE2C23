#include "PSOMnager.h" // ファイル名変更に合わせてインクルードも変更
#include "DXCommon.h"
#include "Logger.h"
#include <cassert>
#include <d3d12.h>
#include <dxcapi.h> // IDxcBlobのため

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;

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
    shaderCache_.clear();
}

void PSOMnager::Finalize() {
    psoCache_.clear();
    rootSignatureCache_.clear();
    shaderCache_.clear();
}

const PsoSet& PSOMnager::GetPsoSet(const PsoProperty& property) {
    auto it = psoCache_.find(property);
    if (it != psoCache_.end()) {
        return it->second;
    }
    CreatePso(property);
    return psoCache_.at(property);
}

// -------------------------------------------------------------------------
// シェーダー管理（重複コンパイル防止）
// -------------------------------------------------------------------------
void PSOMnager::EnsureShaders(PipelineType type, ComPtr<IDxcBlob>& outVS, ComPtr<IDxcBlob>& outPS) {
    // 既にキャッシュにあればそれを返す
    if (shaderCache_.contains(type)) {
        outVS = shaderCache_[type].vs;
        outPS = shaderCache_[type].ps;
        return;
    }

    // 新規コンパイル
    ComPtr<IDxcBlob> vs = nullptr;
    ComPtr<IDxcBlob> ps = nullptr;

    auto dxCommon = DXCommon::GetInstance();

    switch (type) {
    case PipelineType::Sprite:
        vs = dxCommon->CompileShader(L"resources/shaders/Object3d/Object3D.vs.hlsl", L"vs_6_0");
        ps = dxCommon->CompileShader(L"resources/shaders/Object3d/Object3D.ps.hlsl", L"ps_6_0");
        break;
    case PipelineType::Object3d:
        vs = dxCommon->CompileShader(L"resources/shaders/Object3d/Object3D.vs.hlsl", L"vs_6_0");
        ps = dxCommon->CompileShader(L"resources/shaders/Object3d/Object3D.ps.hlsl", L"ps_6_0");
        break;
    case PipelineType::Particle:
        vs = dxCommon->CompileShader(L"resources/shaders/Particle/Particle.vs.hlsl", L"vs_6_0");
        ps = dxCommon->CompileShader(L"resources/shaders/Particle/Particle.ps.hlsl", L"ps_6_0");
        break;
    }

    assert(vs && ps);

    // キャッシュに保存
    shaderCache_[type] = { vs, ps };
    
    outVS = vs;
    outPS = ps;
}

// -------------------------------------------------------------------------
// InputLayout 取得
// -------------------------------------------------------------------------
std::vector<D3D12_INPUT_ELEMENT_DESC> PSOMnager::GetInputLayout(PipelineType type) {
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;

    if (type == PipelineType::Sprite || type == PipelineType::Object3d || type == PipelineType::Particle) {
        // 現状すべて同じレイアウトを使用している
        inputElements = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
    }
    return inputElements;
}

// -------------------------------------------------------------------------
// RootSignature 生成
// -------------------------------------------------------------------------
void PSOMnager::CreateRootSignature(PipelineType type) {
    if (rootSignatureCache_.contains(type)) {
        return;
    }

    std::vector<D3D12_ROOT_PARAMETER> rootParameters;
    std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;

    // 共通サンプラー
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

    // DescriptorRangeはシリアライズまで生存している必要があるのでこのスコープで定義
    D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
    descRangeTexture[0].BaseShaderRegister = 0; // t0
    descRangeTexture[0].NumDescriptors = 1;
    descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // Sprite / Object3d / Particle 共通構成と仮定
    // 構成を変える場合は if (type == ...) で分岐
    rootParameters.resize(4);

    // ルートパラメータインデックスの可読性のため
    enum { kMaterial, kTransform, kTexture, kLight };

    // 0. Material (CBV b0, Pixel)
    rootParameters[kMaterial].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[kMaterial].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[kMaterial].Descriptor.ShaderRegister = 0;

    // 1. Transform (CBV b0, Vertex)
    rootParameters[kTransform].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[kTransform].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[kTransform].Descriptor.ShaderRegister = 0; // b0 (Vertex用)

    // 2. Texture (Table t0, Pixel)
    rootParameters[kTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[kTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[kTexture].DescriptorTable.pDescriptorRanges = descRangeTexture;
    rootParameters[kTexture].DescriptorTable.NumDescriptorRanges = 1;

    // 3. DirectionalLight (CBV b1, Pixel)
    rootParameters[kLight].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[kLight].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[kLight].Descriptor.ShaderRegister = 1;

    
    // シリアライズ
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.pParameters = rootParameters.data();
    descriptionRootSignature.NumParameters = (UINT)rootParameters.size();
    descriptionRootSignature.pStaticSamplers = staticSamplers.data();
    descriptionRootSignature.NumStaticSamplers = (UINT)staticSamplers.size();

    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    ComPtr<ID3D12RootSignature> rootSignature;
    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    rootSignatureCache_[type] = rootSignature;
}

// -------------------------------------------------------------------------
// PSO 生成
// -------------------------------------------------------------------------
void PSOMnager::CreatePso(const PsoProperty& property) {
    auto device = DXCommon::GetInstance()->GetDevice();

    // 1. RootSignature
    CreateRootSignature(property.type);
    auto rootSignature = rootSignatureCache_[property.type];

    // 2. Shader (キャッシュ対応版)
    ComPtr<IDxcBlob> vsBlob;
    ComPtr<IDxcBlob> psBlob;
    EnsureShaders(property.type, vsBlob, psBlob);

    // 3. InputLayout
    auto inputElements = GetInputLayout(property.type);

    // 4. Rasterizer & Depth
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    
    D3D12_DEPTH_STENCIL_DESC depthDesc{};
    depthDesc.DepthEnable = true;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // タイプごとの設定微調整
    switch (property.type) {
    case PipelineType::Sprite:
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // 2Dはカリングしないことが多い
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        break;
    case PipelineType::Object3d:
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        break;
    case PipelineType::Particle:
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 半透明パーティクルはZ書き込みOFF
        break;
    }

    // 5. Blend
    D3D12_BLEND_DESC blendDesc = CreateBlendDesc(property.blendMode);

    // 6. PSO構築
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.InputLayout = { inputElements.data(), (UINT)inputElements.size() };
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    psoDesc.BlendState = blendDesc;
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.NumRenderTargets = 1;
    // ★注意: ここはSwapChainの形式に合わせる必要があります
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; 
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.DepthStencilState = depthDesc;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    PsoSet psoSet;
    psoSet.rootSignature = rootSignature; 
    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoSet.pipelineState));
    assert(SUCCEEDED(hr));

    psoCache_[property] = psoSet;
}

D3D12_BLEND_DESC PSOMnager::CreateBlendDesc(BlendMode mode) {
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;

    // 共通初期値
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    switch (mode) {
    case BlendMode::None:
        blendDesc.RenderTarget[0].BlendEnable = FALSE;
        break;
    case BlendMode::Normal:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        break;
    case BlendMode::Add:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        break;
    case BlendMode::Subtract:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
        break;
    case BlendMode::Multiply:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        break;
    case BlendMode::Screen:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        break;
    }
    return blendDesc;
}