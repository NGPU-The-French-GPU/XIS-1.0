#pragma once

#include <memory>
#include "../Core/XISParameters.h"

namespace XIS {

// Déclarations anticipées
class IRenderer;

/**
 * @brief Étape d'antialiasing dans le pipeline
 * 
 * Cette classe implémente l'étape d'antialiasing qui réduit les artefacts
 * d'aliasing avant l'upscaling ou la génération de frames.
 */
class AntiAliasingStage {
public:
    /**
     * @brief Constructeur
     * 
     * @param renderer Renderer à utiliser
     */
    explicit AntiAliasingStage(std::shared_ptr<IRenderer> renderer);
    ~AntiAliasingStage();

    /**
     * @brief Initialise l'étape d'antialiasing
     * 
     * @param quality Qualité de l'antialiasing
     * @return true si l'initialisation réussit, false sinon
     */
    bool Initialize(AAQuality quality = AAQuality::Medium);

    /**
     * @brief Traite une frame avec antialiasing
     * 
     * @param inputTexture Texture d'entrée
     * @param outputTexture Texture de sortie
     * @return true si le traitement réussit, false sinon
     */
    bool Process(void* inputTexture, void* outputTexture);

    /**
     * @brief Change la qualité de l'antialiasing
     * 
     * @param quality Nouvelle qualité d'antialiasing
     */
    void SetQuality(AAQuality quality);

private:
    std::shared_ptr<IRenderer> m_renderer;
    AAQuality m_quality;
    
    // Ressources des shaders
    void* m_aaShader;
    void* m_aaConstantBuffer;
    
    // Paramètres internes
    struct AAParams {
        float threshold;     // Seuil de détection des contours
        float blendFactor;   // Facteur de mélange pour lissage adaptatif
        int kernelSize;      // Taille du noyau de convolution
        float reserved;      // Pour alignement
    };
    
    AAParams m_aaParams;
    
    // Méthodes d'initialisation des ressources
    bool CreateShaderResources();
    void UpdateConstantBuffer();
    
    // Méthodes d'application des différentes qualités d'AA
    bool ApplyLowQualityAA(void* input, void* output);
    bool ApplyMediumQualityAA(void* input, void* output);
    bool ApplyHighQualityAA(void* input, void* output);
};

} // namespace XIS