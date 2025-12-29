#pragma once
class Franework
{
public:
    virtual ~Franework() = default;
    virtual void Initialize()=0;
    virtual void Finalize()=0;
    virtual void Update()=0;
    virtual void Draw()=0;
    bool IsEnd() {
        return endReqest_;
    }
    void Run();

    void RequestEnd() {
        endReqest_ = true;
    }
private:
    bool endReqest_ = false;
};

