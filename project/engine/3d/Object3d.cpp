#include "Object3d.h"
#include "Object3dCommon.h"
#include <cassert>
#include <fstream> // 追加: ifstreamの完全な型を利用するため
#include <sstream> // 追加: istringstreamのため
#include "MassFunction.h"
#include "TextureManager.h"

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
    object3dCom_ = object3dCommon;

    modelData_ = LoadObjFile("resources", "plane.obj");
    if (modelData_.material.textureFilePath.empty()) {
        modelData_.material.textureFilePath = "resources/uvChecker.png"; // 確実に存在する画像を指定
        TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
    }

    //頂点リソースの作成
    CreateVertexBuffer();
    //マテリアルリソースの作成
    CreateMaterialResource();
    //WVP行列リソースの作成
    CreateWVPResource();
    //平行光源リソースの作成
    CreateDirectionalLightResource();
    //テクスチャの読み込み
    TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
    //テクスチャインデックスの取得
    modelData_.material.textureIndex =
        TextureManager::GetInstance()->GetTextureIndexByFilePath(
            modelData_.material.textureFilePath
        );

    transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    cameraTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
}
void Object3d::Update()
{
    //  WVP行列の作成
    Matrix4x4 worldMatrix = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.traslate);
    Matrix4x4 cameraMatrix = MakeAfineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.traslate);
    Matrix4x4 viewMatrix = Inverse(cameraMatrix);
    //透視投影行列の作成
    Matrix4x4 projectionMatirx = MakePerspectiveFovMatrix(
        0.45f, static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight), 0.1f, 100.0f
    );
    //ワールド行列とビュー行列とプロジェクション行列を掛け算
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatirx));
    //行列をGPUに転送
    wvpResource_->WVP = worldViewProjectionMatrix;
    wvpResource_->World = worldMatrix;
}

void Object3d::Draw()
{
    //VBVの設定
    object3dCom_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
    //マテリアルリソースの設定
    object3dCom_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_.Get()->GetGPUVirtualAddress());
    //WVP行列リソースの設定
    object3dCom_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourse_.Get()->GetGPUVirtualAddress());
    //SRVのディスクリプタテーブルの設定
    object3dCom_->GetDxCommon()->
        GetCommandList()->
        SetGraphicsRootDescriptorTable(2,
            TextureManager::GetInstance()->GetSrvHundleGPU(modelData_.material.textureIndex));
    object3dCom_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResourse_.Get()->GetGPUVirtualAddress());
    //描画コマンド

    object3dCom_->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);


}

Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directryPath, const std::string& filename)
{
    //1. 変数の宣言
    MaterialData materialData{}; // 修正: 初期化
    std::string line;
    std::ifstream file(directryPath + "/" + filename);//ファイルパスを結合して開く
    //2. ファイルを開く
    assert(file.is_open());//ファイルが開けたか確認
    //3. ファイルからデータを読み込みマテリアルデータを作成
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;//行の先頭を識別子として取得

        if (identifier == "map_Kd") {
            std::string textureFileName;
            s >> textureFileName;//テクスチャファイル名を読み込み
            //テクスチャのパスを設定
            materialData.textureFilePath = directryPath + "/" + textureFileName;
        }
    }

    //4. マテリアルデータを返す
    return materialData;
}

Object3d::ModelData Object3d::LoadObjFile(const std::string& directryPath, const std::string& filename)
{
    //1. 変数の宣言
    ModelData modelData;
    std::vector<Vector4> positions;//頂点座標
    std::vector<Vector3> normals;//法線ベクトル
    std::vector<Vector2> texcoords;//テクスチャ座標
    std::string line;
    //2. ファイルを開く
    std::ifstream file(directryPath + "/" + filename);//ファイルパスを結合して開く
    assert(file.is_open());//ファイルが開けたか確認

    //3. ファイルからデータを読み込みモデルデータを作成
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;//行の先頭を識別子として取得
        ///
        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;//頂点座標を読み込み
            position.w = 1.0f; // w成分を1.0に設定
            position.x *= -1.0f; // X軸を反転
            positions.push_back(position);//頂点座標を追加
        } else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;//テクスチャ座標を読み込み
            // OpenGLとDirectXでY軸の方向が異なるため、Y座標を反転
            texcoord.y = 1.0f - texcoord.y;
            texcoords.push_back(texcoord);//テクスチャ座標を追加
        } else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;//法線ベクトルを読み込み
            normal.x *= -1.0f; // X軸を反転
            normals.push_back(normal);

        } else if (identifier == "f") {
            VertexData Triangle[3];
            //面は三角形限定
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;//頂点定義を読み込み
                //頂点の要素へのIndexは「位置/UV/法線」の順番で格納されているので、分解して取得
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];//位置、UV、法線のインデックス
                for (uint32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/');//スラッシュで区切って取得
                    elementIndices[element] = std::stoi(index);//文字列を整数に変換
                }
                // 要素のインデックスから頂点データを構築
                Vector4 position = positions[elementIndices[0] - 1];//1から始まるので-1
                Vector2 texcoord = texcoords[elementIndices[1] - 1];//1から始まるので-1
                Vector3 normal = normals[elementIndices[2] - 1];//1から始まるので-1

                //    VertexData vertex = { position, texcoord, normal };//頂点データを構築
                //    modelData.vertices.push_back(vertex);//モデルデータに頂点を追加
                Triangle[faceVertex] = { position, texcoord, normal };//頂点データを構築
            }
            modelData.vertices.push_back(Triangle[2]);
            modelData.vertices.push_back(Triangle[1]);
            modelData.vertices.push_back(Triangle[0]);
        } else if (identifier == "mtllib")//マテリアルライブラリの読み込み
        {
            std::string materialFileName;
            s >> materialFileName;//マテリアルファイル名を読み込み
            //マテリアルデータを読み込む
            modelData.material = LoadMaterialTemplateFile(directryPath, materialFileName);
        }

    }
    //4. モデルデータを返す
    return modelData;
}

void Object3d::CreateVertexBuffer()
{
    //頂点リソースの作成
    vertexResourse_ =
        object3dCom_->GetDxCommon()->
        CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());
    //頂点バッファビューの設定
    vertexBufferView_.BufferLocation =
        vertexResourse_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    vertexResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    //頂点データの転送
    memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

}

void Object3d::CreateMaterialResource()
{
    //マテリアルリソースの作成
    materialResource_ =
        object3dCom_->GetDxCommon()->
        CreateBufferResource(sizeof(Material));
    materialResource_->
        Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

    materialData_->color = Vector4{ 1.0f,1.0f,1.0f,1.0f };
    materialData_->enableLighting = false;
    materialData_->uvTransform = Makeidetity4x4();



}

void Object3d::CreateWVPResource()
{
    //座標変換
    transformationMatrixResourse_ =
        object3dCom_->GetDxCommon()->
        CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResourse_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&wvpResource_));
    wvpResource_->WVP = Makeidetity4x4();
    wvpResource_->World = Makeidetity4x4();

}

void Object3d::CreateDirectionalLightResource()
{
    directionalLightResourse_ =
        object3dCom_->GetDxCommon()->
        CreateBufferResource(sizeof(DirectionalLight));
    directionalLightResourse_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    directionalLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
    directionalLightData_->direction = { 0.0f,-1.0f,0.0f };
    directionalLightData_->intensity = 1.0f;

}
