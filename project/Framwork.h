#pragma once
class Framwork
{
public:
    virtual ~Framwork() = default;
    virtual void Initialize();
    virtual void Finalize();
    virtual void Update();
    virtual  void Draw();
    virtual  bool IsEnd() {
        return endReqest_;
    }
private:
    bool endReqest_ = false;
};

