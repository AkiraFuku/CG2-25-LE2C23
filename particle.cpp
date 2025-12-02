#include "particle.h"

Particle MakeNewParticle(std::mt19937& ramdamEngine)
{
        std::uniform_real_distribution<float> distribution(-1.0f,1.0f);
        std::uniform_real_distribution<float> distTime(1.0f,3.0f);
   

        Particle particle;
        
        particle.transfom.scale = { distribution(ramdamEngine),distribution(ramdamEngine),distribution(ramdamEngine) };
        particle.transfom.rotate = { 0.0f,0.0f,0.0f };
        particle.transfom.traslate = { distribution(ramdamEngine),distribution(ramdamEngine) ,distribution(ramdamEngine) };
        particle.velocity = { distribution(ramdamEngine),distribution(ramdamEngine),distribution(ramdamEngine) };

        particle.color = { distribution(ramdamEngine),distribution(ramdamEngine),distribution(ramdamEngine),1.0f };
        
        particle.lifeTime = distTime(ramdamEngine);
        particle.currentTime = 0.0f;
        return particle;

}
