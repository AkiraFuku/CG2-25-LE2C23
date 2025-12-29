
#include "Model.h"
#include"Object3D.h"
#include"MathFunction.h"
#include "Sprite.h"
#include "Camera.h"
#include "ParicleEmitter.h"
#include "Framwork.h"
class GameEngine : public Framwork
{
public:
    void Initialize()override;

    void Finalize()override;

    void Update()override;

    void Draw()override;

    
private:
    Camera* camera;
    Sprite* sprite;
    Object3d* object3d2;
    Object3d* object3d;
    ParicleEmitter* emitter;
    Audio::SoundData soundData1;
private:
    //ログファイルパス
    const std::filesystem::path logFilePath = "D3DResourceLeakLog.txt";
    //終了フラグ
   

};

