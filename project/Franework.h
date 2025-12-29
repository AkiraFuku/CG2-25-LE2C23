#pragma once
class Franework
{
public:
    virtual ~Franework() = default;
    virtual void Initialize();
    virtual void Finalize();
    virtual void Update();
    virtual void Draw()=0;
    bool IsEnd() {
        return endReqest_;
    }
private:
    bool endReqest_ = false;
};

