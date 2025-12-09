#include "particle.h"
#include "MathFunction.h"
Particle MakeNewParticle(std::mt19937& ramdamEngine,const Vector3& Transform)
{
        std::uniform_real_distribution<float> distribution(-1.0f,1.0f);
        std::uniform_real_distribution<float> distTime(1.0f,10.0f);
   

        Particle particle;
        
        particle.transfom.scale = {1.0f,1.0f,1.0f  };
        particle.transfom.rotate = { 0.0f,0.0f,0.0f };
        Vector3 randamTranslate={ distribution(ramdamEngine),distribution(ramdamEngine) ,distribution(ramdamEngine) };
        particle.transfom.traslate = Transform+randamTranslate;
        particle.velocity = { distribution(ramdamEngine),distribution(ramdamEngine),distribution(ramdamEngine) };

        particle.color = { distribution(ramdamEngine),distribution(ramdamEngine),distribution(ramdamEngine),1.0f };
        
        particle.lifeTime = distTime(ramdamEngine);
        particle.currentTime = 0.0f;
        return particle;

}

std::list<Particle> Emit(const Emitter& emitter, std::mt19937& ramdamEngine)
{
   std::list<Particle>particles;
   for (uint32_t i = 0; i < emitter.count; ++i)
   {
       particles.push_back(MakeNewParticle(ramdamEngine,emitter.transfom.traslate));
   }
   return particles;
}

bool IsCollision(const AABB& aabb, const Vector3& point)
{
    bool inX=aabb.max.x>point.x&&aabb.min.x<point.x;
    bool inY=aabb.max.y>point.y&&aabb.min.y<point.y;
    bool inZ=aabb.max.z>point.z&&aabb.min.z<point.z;
    if (inX&&inY&&inZ)
    {
        return true;
    }

    return false;
}
